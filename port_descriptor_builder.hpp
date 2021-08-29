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
  /** Specifies the lower bound of the port range.
   *
   * @param lower_bound The value of the lower bound to be set
   * @return Mutable reference to this.
   * @note Sets the lower bound and the hint that the port range is bounded
   * below.
   */
  PortDescriptorBuilder &WithLowerBound(LADSPA_Data lower_bound);
  /** Specifies the upper bound of the port range.
   *
   * @param upper_bound The value of the upper bound to be set
   * @return Mutable reference to this.
   * @note Sets the upper bound and the hint that the port range is bounded
   * above.
   */
  PortDescriptorBuilder &WithUpperBound(LADSPA_Data upper_bound);
  /** Declares that the port works as a toggle.
   *
   * @return Mutable reference to this.
   * @throw std::invalid_argument if the default hint has been set to 0 or 1.
   */
  PortDescriptorBuilder &DeclareAsToggled();
  /** Declares that the port value is expressed in the units of the sample rate.
   *
   * @return Mutable reference to this.
   */
  PortDescriptorBuilder &DeclareAsExpressedInSampleRate();
  /** Declares that the port value is best expressed on a logarithmic scale.
   *
   * @return Mutable reference to this.
   */
  PortDescriptorBuilder &DeclareAsLogarithmic();
  /** Declares that this control port is best expressed using integer values.
   *
   * @return Mutable reference to this.
   */
  PortDescriptorBuilder &DeclareAsInteger();
  /** Declares that no default value is provided.
   *
   * @return Mutable reference to this.
   * @throw std::invalid_argument if a default value hint has already been set.
   */
  PortDescriptorBuilder &WithNoDefaultHint();
  /** Declares that the lower bound is a meaningful default value for this port.
   *
   * @return Mutable reference to this.
   * @note Calling RangeHint() throws a runtime error if this has been called
   * and the lower bound has not been set.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHintMinimum();
  /** Declares that the value at 25% of the range should be used as the default.
   *
   * @return Mutable reference to this.
   * @note In case of the logarithmic range, the 25% are to be taken on the
   * logarithmic scale.
   * @note Calling RangeHint() throws a runtime error if this has been called
   * and the lower and the upper bounds have not been set.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHintLow();
  /** Declares that the value at 50% of the range should be used as the default.
   *
   * @return Mutable reference to this.
   * @note In case of the logarithmic range, the 50% are to be taken on the
   * logarithmic scale.
   * @note Calling RangeHint() throws a runtime error if this has been called
   * and the lower and the upper bounds have not been set.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHintMiddle();
  /** Declares that the value at 75% of the range should be used as the default.
   *
   * @return Mutable reference to this.
   * @note In case of the logarithmic range, the 75% are to be taken on the
   * logarithmic scale.
   * @note Calling RangeHint() throws a runtime error if this has been called
   * and the lower and the upper bounds have not been set.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHintHigh();
  /** Declares that the upper bound is a meaningful default value for this port.
   *
   * @return Mutable reference to this.
   * @note Calling RangeHint() throws a runtime error if this has been called
   * and the upper bound has not been set.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHintMaximum();
  /** Declares that zero is a meaningful default value for this port.
   *
   * @return Mutable reference to this.
   * @note Calling RangeHint() throws a runtime error if this has been called
   * and the upper bound has not been set.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHint0();
  /** Declares that 1 is a meaningful default value for this port.
   *
   * @return Mutable reference to this.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHint1();
  /** Declares that 100 is a meaningful default value for this port.
   *
   * @return Mutable reference to this.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHint100();
  /** Declares that 440 is a meaningful default value for this port.
   *
   * @return Mutable reference to this.
   * @throw std::invalid_argument if either another default value hint or the
   * "no default" hint has already been set.
   */
  PortDescriptorBuilder &WithDefaultHint440();

private:
  std::string name_;
  LADSPA_PortDescriptor descriptor_{};
  LADSPA_PortRangeHint range_hint_{0, 0, 0};
};

class PortDescriptorSetter {
public:
  bool AddPortDescriptor(PortDescriptorBuilder port_description);

  void SetPortDescriptors(LADSPA_Descriptor &descriptor);
};

#endif // LADSPA_REVERB_PORT_DESCRIPTOR_BUILDER_HPP
