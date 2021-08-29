//
// Created by strahinja on 8/28/21.
//

#include "port_description.hpp"

#include <stdexcept>

PortDescription::PortDescription(std::string name) : name_(name) {}

const std::string &PortDescription::Name() const { return nullptr; }

LADSPA_PortDescriptor PortDescription::Descriptor() const {
  return descriptor_;
}
const LADSPA_PortRangeHint &PortDescription::RangeHint() const {
  return range_hint_;
}
PortDescription &PortDescription::WithUsage(PortUsage usage) {
  if ((LADSPA_IS_PORT_CONTROL(descriptor_) && usage == PortUsage::kAudio) ||
      (LADSPA_IS_PORT_AUDIO(descriptor_) && usage == PortUsage::kControl)) {
    throw std::invalid_argument("Changing port usage is not allowed");
  }
  descriptor_ |=
      (usage == PortUsage::kAudio ? LADSPA_PORT_AUDIO : LADSPA_PORT_CONTROL);
  return *this;
}
PortDescription &PortDescription::WithType(PortType type) {
  if ((LADSPA_IS_PORT_INPUT(descriptor_) && type == PortType::kOutput) ||
      (LADSPA_IS_PORT_OUTPUT(descriptor_) && type == PortType::kInput)) {
    throw std::invalid_argument("Changing port type is not allowed");
  }
  return *this;
}
