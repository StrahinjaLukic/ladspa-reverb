//
// Created by strahinja on 8/28/21.
//

#ifndef LADSPA_REVERB_PORT_DESCRIPTOR_BUILDER_HPP
#define LADSPA_REVERB_PORT_DESCRIPTOR_BUILDER_HPP

#include "ladspa.h"

#include <string>

enum class PortUsage { kControl, kAudio };
enum class PortType { kInput, kOutput };

class PortDescriptorBuilder {
public:
  explicit PortDescriptorBuilder(std::string name);
  PortDescriptorBuilder(const PortDescriptorBuilder &other) = default;

  [[nodiscard]] const std::string &Name() const;
  [[nodiscard]] LADSPA_PortDescriptor Descriptor() const;
  [[nodiscard]] const LADSPA_PortRangeHint &RangeHint() const;

  // Description
  /** Sets port usage
   *
   * @param usage Port usage. Available values: PortUsage::kAudio,
   * PortUsage::kControl.
   * @return Mutable reference to this.
   * @throw std::invalid_argument if called more than once with different
   * arguments.
   */
  PortDescriptorBuilder &WithUsage(PortUsage usage);

  /** Sets port type
   *
   * @param type Port type. Available values: PortType::kInput,
   * PortType::kOutput
   * @return Mutable reference to this.
   * @throw std::invalid_argument if called more than once with different
   * arguments.
   */
  PortDescriptorBuilder &WithType(PortType type);

  // Range hint
  PortDescriptorBuilder &WithLowerBound(LADSPA_Data lower_bound);
  PortDescriptorBuilder &WithUpperBound(LADSPA_Data upper_bound);
  PortDescriptorBuilder &DeclareAsToggled();
  PortDescriptorBuilder &DeclareAsExpressedInSampleRate();
  PortDescriptorBuilder &DeclareAsLogarithmic();
  PortDescriptorBuilder &DeclareAsInteger();
  PortDescriptorBuilder &WithNoDefaultHint();
  PortDescriptorBuilder &WithDefaultHintMinimum();
  PortDescriptorBuilder &WithDefaultHintLow();
  PortDescriptorBuilder &WithDefaultHintMiddle();
  PortDescriptorBuilder &WithDefaultHintHigh();
  PortDescriptorBuilder &WithDefaultHintMaximum();
  PortDescriptorBuilder &WithDefaultHint0();
  PortDescriptorBuilder &WithDefaultHint1();
  PortDescriptorBuilder &WithDefaultHint100();
  PortDescriptorBuilder &WithDefaultHint440();

private:
  std::string name_;
  LADSPA_PortDescriptor descriptor_;
  LADSPA_PortRangeHint range_hint_;
};

class PortDescriptorSetter {
public:
  bool AddPortDescriptor(PortDescriptorBuilder port_description);

  void SetPortDescriptors(LADSPA_Descriptor &descriptor);
};

#endif // LADSPA_REVERB_PORT_DESCRIPTOR_BUILDER_HPP
