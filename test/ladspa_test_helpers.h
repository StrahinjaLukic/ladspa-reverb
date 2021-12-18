#ifndef LADSPA_REVERB_LADSPA_TEST_HELPERS_H
#define LADSPA_REVERB_LADSPA_TEST_HELPERS_H

#include "ladspa.h"

inline bool IsAudioInput(LADSPA_PortDescriptor port_descriptor) {
  return LADSPA_IS_PORT_AUDIO(port_descriptor) && LADSPA_IS_PORT_INPUT(port_descriptor);
}

inline bool IsAudioOutput(LADSPA_PortDescriptor port_descriptor) {
  return LADSPA_IS_PORT_AUDIO(port_descriptor) && LADSPA_IS_PORT_OUTPUT(port_descriptor);
}

inline bool IsControlInput(LADSPA_PortDescriptor port_descriptor) {
  return LADSPA_IS_PORT_CONTROL(port_descriptor) && LADSPA_IS_PORT_INPUT(port_descriptor);
}

inline bool IsControlOutput(LADSPA_PortDescriptor port_descriptor) {
  return LADSPA_IS_PORT_CONTROL(port_descriptor) && LADSPA_IS_PORT_OUTPUT(port_descriptor);
}

#endif//LADSPA_REVERB_LADSPA_TEST_HELPERS_H
