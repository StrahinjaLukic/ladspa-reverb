#include <vector>

#include "ladspa.h"

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

  Reverb(unsigned long sample_rate, double microsample_duration_s);
  void ResetWindow();
  bool UpdateDecay();
  bool UpdateWetLevel();
  void UpdateWeight(bool force = false);
  // Superimposes window contents to output, starting from offset and ending at
  // std::min(window_.size(), window_write_position_)
  void ApplyWindow(unsigned long offset);
  void Run(unsigned long sample_count);

  void ConnectPort(unsigned long port, LADSPA_Data *data_location);

  friend LADSPA_Handle InstantiateReverb(const LADSPA_Descriptor *, unsigned long sample_rate);
  friend void ConnectPortToReverb(LADSPA_Handle handle, unsigned long port, LADSPA_Data *data_location);
  friend void ActivateReverb(LADSPA_Handle handle);
  friend void RunReverb(LADSPA_Handle handle, unsigned long sample_count);
  friend void CleanupReverb(LADSPA_Handle handle);

  static constexpr inline Reverb &Instance(LADSPA_Handle handle) {
    return *static_cast<Reverb *>(handle);
  }
};