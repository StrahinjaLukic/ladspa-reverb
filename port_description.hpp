//
// Created by strahinja on 8/28/21.
//

#ifndef LADSPA_REVERB_PORT_DESCRIPTION_HPP
#define LADSPA_REVERB_PORT_DESCRIPTION_HPP

#include <string>

#include "ladspa.h"

enum class PortUsage { kControl, kAudio };
enum class PortType { kInput, kOutput };

class PortDescription {
public:
  explicit PortDescription(std::string name);
  PortDescription(const PortDescription &other) = default;

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
  PortDescription &WithUsage(PortUsage usage);

  /** Sets port type
   *
   * @param type Port type. Available values: PortType::kInput,
   * PortType::kOutput
   * @return Mutable reference to this.
   * @throw std::invalid_argument if called more than once with different
   * arguments.
   */
  PortDescription &WithType(PortType type);

  // Range hint
  PortDescription &WithLowerBound(LADSPA_Data lower_bound);
  PortDescription &WithUpperBound(LADSPA_Data upper_bound);
  PortDescription &DeclareAsToggled();
  PortDescription &DeclareAsExpressedInSampleRate();
  PortDescription &DeclareAsLogarithmic();
  PortDescription &DeclareAsInteger();
  PortDescription &WithNoDefaultHint();
  PortDescription &WithDefaultHintMinimum();
  PortDescription &WithDefaultHintLow();
  PortDescription &WithDefaultHintMiddle();
  PortDescription &WithDefaultHintHigh();
  PortDescription &WithDefaultHintMaximum();
  PortDescription &WithDefaultHint0();
  PortDescription &WithDefaultHint1();
  PortDescription &WithDefaultHint100();
  PortDescription &WithDefaultHint440();

private:
  std::string name_;
  LADSPA_PortDescriptor descriptor_;
  LADSPA_PortRangeHint range_hint_;
};

class PortDescriptorSetter {
public:
  bool AddPortDescription(PortDescription port_description);

  void SetPortDescriptors(LADSPA_Descriptor &descriptor);
};

#endif // LADSPA_REVERB_PORT_DESCRIPTION_HPP
