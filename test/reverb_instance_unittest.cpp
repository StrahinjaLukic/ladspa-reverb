#include <gtest/gtest.h>

#include "ladspa.h"
#include "ladspa_test_helpers.h"

TEST(ReverbInstanceTest, InstanceIsNotNull) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);

  auto reverb = descriptor->instantiate(descriptor, 44000);
  ASSERT_NE(nullptr, reverb);
}

TEST(ReverbInstanceTest, LifecycleWithoutRunning) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);
  auto reverb = descriptor->instantiate(descriptor, 44000);
  ASSERT_NE(nullptr, reverb);

  std::vector<LADSPA_Data *> port_locations;
  for (std::size_t i = 0; i < descriptor->PortCount; ++i) {
    port_locations.push_back(new LADSPA_Data);
    descriptor->connect_port(reverb, i, port_locations.back());
  }

  descriptor->activate(reverb);

  descriptor->cleanup(reverb);
  for (std::size_t i = 0; i < descriptor->PortCount; ++i) {
    delete port_locations.at(i);
  }
}

TEST(ReverbInstanceTest, LifecycleWithRunning) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);
  auto reverb = descriptor->instantiate(descriptor, 44000);
  ASSERT_NE(nullptr, reverb);

  LADSPA_Data decay_control;
  LADSPA_Data wet_level_control;
  constexpr std::size_t number_of_samples   = 10000;
  constexpr std::size_t playground_size     = 4 * number_of_samples;
  constexpr std::size_t output_start_offset = 2 * number_of_samples;
  constexpr std::size_t output_end_offset   = 3 * number_of_samples;
  std::vector<LADSPA_Data> playground(4 * number_of_samples, 0);

  for (std::size_t i = 0; i < descriptor->PortCount; ++i) {
    const auto &port_descriptor = descriptor->PortDescriptors[i];
    if (IsAudioInput(port_descriptor)) {
      // Connect and fill input
      descriptor->connect_port(reverb, i, playground.data());
      for (std::size_t j = 0; j < number_of_samples; ++j) {
        playground[j] = static_cast<LADSPA_Data>(j);
      }
    } else if (IsAudioOutput(port_descriptor)) {
      // Connect output
      descriptor->connect_port(reverb, i, playground.data() + output_start_offset);
    } else if (std::string(descriptor->PortNames[i]) == "Decay (s)") {
      descriptor->connect_port(reverb, i, &decay_control);
    } else if (std::string(descriptor->PortNames[i]) == "Relative wet signal level") {
      descriptor->connect_port(reverb, i, &wet_level_control);
    }
  }

  descriptor->activate(reverb);
  constexpr std::size_t n_cycles = 10;
  for (std::size_t i = 0; i < n_cycles; ++i) {
    decay_control     = i * 0.5;
    wet_level_control = (n_cycles - i) * 0.05;
    descriptor->run(reverb, number_of_samples);

    int n_out_of_bounds_access_before = 0;
    for (std::size_t i = output_start_offset; i > number_of_samples; --i) {
      if (playground[i] != 0) { n_out_of_bounds_access_before = output_start_offset - i; }
    }
    EXPECT_EQ(0, n_out_of_bounds_access_before)
            << "Data written up to " << n_out_of_bounds_access_before << " places before the output buffer start.";

    int n_out_of_bounds_access_after = 0;
    for (std::size_t i = output_end_offset; i < playground_size; ++i) {
      if (playground[i] != 0) { n_out_of_bounds_access_after = i - output_end_offset + 1; }
    }
    EXPECT_EQ(0, n_out_of_bounds_access_after) << "Data written up to " << n_out_of_bounds_access_after
                                               << " places after the output buffer end. Decay time is " << decay_control
                                               << " s and wet level is " << wet_level_control << ".";
  }

  descriptor->cleanup(reverb);
}
