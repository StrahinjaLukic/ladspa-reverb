#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ladspa.h"

namespace {

enum class PortId { kInput = 0, kOutput, kDecay, kWetLevel, kPortCount };

constexpr std::size_t ToIndex(PortId port_enum) {
  return static_cast<std::size_t>(port_enum);
}

static constexpr unsigned long kPortCount = ToIndex(PortId::kPortCount);

constexpr PortId ToEnum(unsigned long port_index) {
  if (port_index > kPortCount) { throw std::invalid_argument("PortId index out of range"); }
  return static_cast<PortId>(port_index);
}

static LADSPA_Handle InstantiateReverb(const LADSPA_Descriptor *, unsigned long sample_rate);

static void ConnectPortToReverb(LADSPA_Handle handle, unsigned long port, LADSPA_Data *data_location);

static void ActivateReverb(LADSPA_Handle handle);

static void RunReverb(LADSPA_Handle handle, unsigned long sample_count);

static void CleanupReverb(LADSPA_Handle handle);

class Reverb {
  public:
  static inline constexpr LADSPA_Data kMinimumDecayS{0.01};
  static inline constexpr LADSPA_Data kDefaultDecayS{0.1};
  static inline constexpr LADSPA_Data kMinimumWetLevel{0.1};
  static inline constexpr LADSPA_Data kMaximumWetLevel{10};
  static inline constexpr LADSPA_Data kDefaultWetLevel{1};

  private:
  /* Ports: */
  LADSPA_Data *input_{};
  LADSPA_Data *output_{};
  LADSPA_Data *decay_s_port_{};   // Exponential decay time in seconds
  LADSPA_Data *wet_level_port_{}; // Relative level of wet signal

  /* State */
  std::vector<LADSPA_Data> window_;
  std::size_t window_write_position_{0};
  std::size_t window_read_position_{0};
  double microsample_duration_s_;           // Duration of a microsample in seconds
  LADSPA_Data decay_s_{kDefaultDecayS};     // Stored exponential decay time in seconds
  LADSPA_Data wet_level_{kDefaultWetLevel}; // Stored relative level of wet signal
  unsigned long sample_rate_;

  double weight_{1};

  Reverb(unsigned long sample_rate, double microsample_duration_s)
      : microsample_duration_s_(microsample_duration_s),
        sample_rate_(sample_rate) {
    ResetWindow();
    UpdateWeight();
  }

  void ResetWindow() {
    window_.resize(std::lround(microsample_duration_s_ * sample_rate_), 0);
    window_write_position_ = 0;
    window_read_position_  = 0;
  }

  bool UpdateDecay() {
    const auto decay_from_port = decay_s_port_ != nullptr ? std::max(*decay_s_port_, kMinimumDecayS) : kDefaultDecayS;
    if (decay_s_ != decay_from_port) {
      decay_s_ = decay_from_port;
      return true;
    }
    return false;
  }

  bool UpdateWetLevel() {
    const auto wet_level_from_port = wet_level_port_ != nullptr
                                             ? std::clamp(*wet_level_port_, kMinimumWetLevel, kMaximumWetLevel)
                                             : kDefaultWetLevel;
    if (wet_level_ != wet_level_from_port) {
      wet_level_ = wet_level_from_port;
      return true;
    }
    return false;
  }

  void UpdateWeight(bool force = false) {
    if (UpdateDecay() || force) {
      const auto t_buffer = static_cast<double>(window_.size()) / sample_rate_;
      weight_             = std::exp(-t_buffer / decay_s_);
      std::cout << "Reverb: Updating weight to " << weight_ << std::endl;
    }
  }

  // Applies window contents to output, starting from offset and ending at
  // std::min(window_.size(), window_write_position_)
  void ApplyWindow(unsigned long offset) {
    const auto i_end      = std::min(window_.size(), window_write_position_);
    const auto wet_factor = wet_level_ / weight_;
    for (unsigned long i_window = window_read_position_; i_window < i_end; ++i_window) {
      output_[offset + i_window] += wet_factor * window_[i_window];
    }
    window_read_position_ = i_end;
    if (window_write_position_ == window_.size()) {
      window_write_position_ = 0;
      window_read_position_  = 0;
    }
  }

  void Run(unsigned long sample_count) {
    UpdateWetLevel();
    UpdateWeight();
    std::size_t window_offset = 0;

    memcpy(output_, input_, sample_count * sizeof(LADSPA_Data));

    for (unsigned long i_sample = 0; i_sample < sample_count; ++i_sample, ++window_write_position_) {
      if (window_offset + window_write_position_ >= sample_count) { break; }
      if (window_write_position_ >= window_.size()) {
        ApplyWindow(window_offset);
        window_offset += window_.size();
      }
      window_[window_write_position_] = weight_ * window_[window_write_position_] + (1. - weight_) * input_[i_sample];
    }
    ApplyWindow(window_offset);
  }

  void ConnectPort(unsigned long port, LADSPA_Data *data_location) {
    switch (ToEnum(port)) {
    case PortId::kInput:
      input_ = data_location;
      return;
    case PortId::kOutput:
      output_ = data_location;
      return;
    case PortId::kDecay:
      decay_s_port_ = data_location;
      return;
    case PortId::kWetLevel:
      wet_level_port_ = data_location;
      return;
    case PortId::kPortCount:
      throw std::invalid_argument("PortId number out of range");
    }
    throw std::invalid_argument("PortId number unknown");
  }

  friend LADSPA_Handle InstantiateReverb(const LADSPA_Descriptor *, unsigned long sample_rate);

  friend void ConnectPortToReverb(LADSPA_Handle handle, unsigned long port, LADSPA_Data *data_location);

  friend void ActivateReverb(LADSPA_Handle handle);

  friend void RunReverb(LADSPA_Handle handle, unsigned long sample_count);

  friend void CleanupReverb(LADSPA_Handle handle);

  static constexpr Reverb &Instance(LADSPA_Handle handle) {
    return *static_cast<Reverb *>(handle);
  }
};

/*****************************************************************************/

static LADSPA_Handle InstantiateReverb(const LADSPA_Descriptor *, unsigned long sample_rate) {
  static constexpr double microsample_duration_s = 0.01;
  return new Reverb(sample_rate, microsample_duration_s);
}

/*****************************************************************************/

static void ConnectPortToReverb(LADSPA_Handle handle, unsigned long port, LADSPA_Data *data_location) {
  std::cout << "ConnectPortToReverb(" << handle << ", " << port << ")" << std::endl;
  Reverb::Instance(handle).ConnectPort(port, data_location);
}

/*****************************************************************************/

void ActivateReverb(LADSPA_Handle handle) {
  std::cout << "ActivateReverb(" << handle << ")" << std::endl;
  Reverb::Instance(handle).ResetWindow();
  Reverb::Instance(handle).UpdateWetLevel();
  Reverb::Instance(handle).UpdateWeight(true);
}

/*****************************************************************************/

void RunReverb(LADSPA_Handle handle, unsigned long sample_count) {
  Reverb::Instance(handle).Run(sample_count);
}

/*****************************************************************************/

void CleanupReverb(LADSPA_Handle handle) {
  delete static_cast<Reverb *>(handle);
}

/*****************************************************************************/

static LADSPA_Descriptor *g_descriptor = nullptr;

} // namespace

/* Global object to handle the lifetime of the plugin descriptor. */
class StartupShutdownHandler {

  const std::string maker{"Strahinja Lukic"};
  const std::string copyright{"None"};
  const std::string input_port_name{"Input"};
  const std::string output_port_name{"Output"};
  const std::string decay_port_name{"Decay (s)"};
  const std::string wet_level_port_name{"Relative wet signal level"};
  const std::string label{"reverb"};
  const std::string name{"Simple Reverb"};

  public:
  StartupShutdownHandler() {
    const char **pc_port_names;

    g_descriptor = new LADSPA_Descriptor;

    g_descriptor->Label      = label.data();
    g_descriptor->Name       = name.data();
    g_descriptor->UniqueID   = 1083;
    g_descriptor->Properties = 0;
    g_descriptor->Maker      = maker.data();
    g_descriptor->Copyright  = copyright.data();
    g_descriptor->PortCount  = kPortCount;

    LADSPA_PortDescriptor *port_descriptors      = new LADSPA_PortDescriptor[kPortCount];
    g_descriptor->PortDescriptors                = static_cast<const LADSPA_PortDescriptor *>(port_descriptors);
    port_descriptors[ToIndex(PortId::kInput)]    = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
    port_descriptors[ToIndex(PortId::kOutput)]   = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
    port_descriptors[ToIndex(PortId::kDecay)]    = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
    port_descriptors[ToIndex(PortId::kWetLevel)] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;

    LADSPA_PortRangeHint *port_range_hints                    = new LADSPA_PortRangeHint[kPortCount];
    port_range_hints[ToIndex(PortId::kInput)].HintDescriptor  = 0;
    port_range_hints[ToIndex(PortId::kOutput)].HintDescriptor = 0;

    port_range_hints[ToIndex(PortId::kDecay)].HintDescriptor = LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_DEFAULT_MINIMUM;
    port_range_hints[ToIndex(PortId::kDecay)].LowerBound     = Reverb::kMinimumDecayS;

    port_range_hints[ToIndex(PortId::kWetLevel)].HintDescriptor =
            LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_1;
    port_range_hints[ToIndex(PortId::kWetLevel)].LowerBound = Reverb::kMinimumWetLevel;
    port_range_hints[ToIndex(PortId::kWetLevel)].UpperBound = Reverb::kMaximumWetLevel;

    g_descriptor->PortRangeHints = port_range_hints;

    pc_port_names                             = new const char *[kPortCount];
    g_descriptor->PortNames                   = static_cast<const char **>(pc_port_names);
    pc_port_names[ToIndex(PortId::kInput)]    = input_port_name.data();
    pc_port_names[ToIndex(PortId::kOutput)]   = output_port_name.data();
    pc_port_names[ToIndex(PortId::kDecay)]    = decay_port_name.data();
    pc_port_names[ToIndex(PortId::kWetLevel)] = wet_level_port_name.data();

    g_descriptor->instantiate         = InstantiateReverb;
    g_descriptor->connect_port        = ConnectPortToReverb;
    g_descriptor->activate            = ActivateReverb;
    g_descriptor->run_adding          = nullptr;
    g_descriptor->set_run_adding_gain = nullptr;
    g_descriptor->deactivate          = nullptr;
    g_descriptor->cleanup             = CleanupReverb;
    g_descriptor->run                 = RunReverb;
  }

  ~StartupShutdownHandler() {
    if (g_descriptor) {
      delete[] g_descriptor->PortDescriptors;
      delete[] g_descriptor->PortRangeHints;
      delete[] g_descriptor->PortNames;
      delete g_descriptor;
    }
  }
};

static StartupShutdownHandler g_oShutdownStartupHandler;

/*****************************************************************************/

/* Return a descriptor of the requested plugin type. Only one plugin
   type is available in this library. */
const LADSPA_Descriptor *ladspa_descriptor(unsigned long index) {
  if (index == 0) {
    return g_descriptor;
  } else {
    return nullptr;
  }
}

/*****************************************************************************/
