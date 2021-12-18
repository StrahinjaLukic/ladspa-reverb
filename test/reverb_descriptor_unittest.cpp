#include <gtest/gtest.h>

#include <string>
#include <unordered_map>

#include "ladspa.h"
#include "ladspa_test_helpers.h"

TEST(ReverbDescriptorTest, DescriptorIsNotNull) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);
}

TEST(ReverbDescriptorTest, Label) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);

  ASSERT_NE(nullptr, descriptor->Label);
  std::string label(descriptor->Label);
  EXPECT_EQ("reverb", label);
}

TEST(ReverbDescriptorTest, Properties) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);

  EXPECT_FALSE(LADSPA_IS_REALTIME(descriptor->Properties));
  EXPECT_FALSE(LADSPA_IS_INPLACE_BROKEN(descriptor->Properties));
  EXPECT_FALSE(LADSPA_IS_HARD_RT_CAPABLE(descriptor->Properties));
}

TEST(ReverbDescriptorTest, PortDescriptors) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);

  auto port_descriptors = descriptor->PortDescriptors;
  ASSERT_NE(nullptr, port_descriptors);

  std::unordered_map<std::string, int> port_type_counter{{"audio input", 0},
                                                         {"audio output", 0},
                                                         {"control input", 0},
                                                         {"control output", 0}};

  const std::unordered_map<std::string, int> expected_port_type_count{{"audio input", 1},
                                                                      {"audio output", 1},
                                                                      {"control input", 2},
                                                                      {"control output", 0}};

  for (std::size_t i = 0; i < descriptor->PortCount; ++i) {
    auto port_descriptor = port_descriptors[i];
    EXPECT_TRUE(LADSPA_IS_PORT_INPUT(port_descriptor) ^ LADSPA_IS_PORT_OUTPUT(port_descriptor));
    EXPECT_TRUE(LADSPA_IS_PORT_CONTROL(port_descriptor) ^ LADSPA_IS_PORT_AUDIO(port_descriptor));

    if (IsAudioInput(port_descriptor)) { ++port_type_counter["audio input"]; }
    if (IsAudioOutput(port_descriptor)) { ++port_type_counter["audio output"]; }
    if (IsControlInput(port_descriptor)) { ++port_type_counter["control input"]; }
    if (IsControlOutput(port_descriptor)) { ++port_type_counter["control output"]; }
  }

  EXPECT_EQ(expected_port_type_count, port_type_counter);
}

TEST(ReverbDescriptorTest, PortNames) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);

  auto port_names = descriptor->PortNames;
  ASSERT_NE(nullptr, port_names);

  for (std::size_t i = 0; i < descriptor->PortCount; ++i) {
    std::string port_name(port_names[i]);
    EXPECT_FALSE(port_name.empty());
    EXPECT_LT(port_name.size(), 1000);
  }
}

TEST(ReverbDescriptorTest, PortRangeHints) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);

  auto port_range_hints = descriptor->PortRangeHints;
  ASSERT_NE(nullptr, port_range_hints);

  for (std::size_t i = 0; i < descriptor->PortCount; ++i) {
    port_range_hints[i];
  }
}

TEST(ReverbDescriptorTest, FunctionsAreNotNull) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);

  EXPECT_NE(nullptr, descriptor->instantiate);
  EXPECT_NE(nullptr, descriptor->connect_port);
  EXPECT_NE(nullptr, descriptor->activate);
  EXPECT_NE(nullptr, descriptor->run);
  EXPECT_NE(nullptr, descriptor->cleanup);
}