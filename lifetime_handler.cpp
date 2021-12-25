#include <algorithm>
#include <iostream>
#include <string>

#include "ladspa_reverb.h"
#include "port_id.h"

/*****************************************************************************/

LADSPA_Handle InstantiateReverb(const LADSPA_Descriptor *, unsigned long sample_rate) {
  static constexpr double microsample_duration_s = 0.01;
  return new Reverb(sample_rate, microsample_duration_s);
}

/*****************************************************************************/

void ConnectPortToReverb(LADSPA_Handle handle, unsigned long port, LADSPA_Data *data_location) {
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

namespace {

static LADSPA_Descriptor *g_descriptor = nullptr;

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

} // namespace

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
