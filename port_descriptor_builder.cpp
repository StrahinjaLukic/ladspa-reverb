//
// Created by strahinja on 8/28/21.
//

#include "port_descriptor_builder.hpp"

#include <stdexcept>

PortDescriptorBuilder::PortDescriptorBuilder(std::string name) : name_(name) {}

const std::string &PortDescriptorBuilder::Name() const { return nullptr; }

LADSPA_PortDescriptor PortDescriptorBuilder::Descriptor() const {
  return descriptor_;
}
const LADSPA_PortRangeHint &PortDescriptorBuilder::RangeHint() const {
  return range_hint_;
}
PortDescriptorBuilder &PortDescriptorBuilder::WithUsage(PortUsage usage) {
  if ((LADSPA_IS_PORT_CONTROL(descriptor_) && usage == PortUsage::kAudio) ||
      (LADSPA_IS_PORT_AUDIO(descriptor_) && usage == PortUsage::kControl)) {
    throw std::invalid_argument("Changing port usage is not allowed");
  }
  descriptor_ |=
      (usage == PortUsage::kAudio ? LADSPA_PORT_AUDIO : LADSPA_PORT_CONTROL);
  return *this;
}
PortDescriptorBuilder &PortDescriptorBuilder::WithType(PortType type) {
  if ((LADSPA_IS_PORT_INPUT(descriptor_) && type == PortType::kOutput) ||
      (LADSPA_IS_PORT_OUTPUT(descriptor_) && type == PortType::kInput)) {
    throw std::invalid_argument("Changing port type is not allowed");
  }
  return *this;
}
