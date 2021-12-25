#ifndef LADSPA_REVERB_PORT_ID_H
#define LADSPA_REVERB_PORT_ID_H

#include <stdexcept>

enum class PortId { kInput = 0, kOutput, kDecay, kWetLevel, kPortCount };

constexpr inline std::size_t ToIndex(PortId port_enum) {
  return static_cast<std::size_t>(port_enum);
}

static constexpr std::size_t kPortCount = ToIndex(PortId::kPortCount);

constexpr inline PortId ToEnum(unsigned long port_index) {
  if (port_index > kPortCount) { throw std::invalid_argument("PortId index out of range"); }
  return static_cast<PortId>(port_index);
}

#endif //LADSPA_REVERB_PORT_ID_H
