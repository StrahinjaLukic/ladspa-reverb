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

  std::vector<LADSPA_Data *> port_locations;
  LADSPA_Data *decay_control;
  LADSPA_Data *wet_level_control;
  constexpr std::size_t number_of_samples = 10000;

  for (std::size_t i = 0; i < descriptor->PortCount; ++i) {
    if (LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[i])) {
      port_locations.push_back(new LADSPA_Data[10000]);
    } else {
      port_locations.push_back(new LADSPA_Data);
    }
    descriptor->connect_port(reverb, i, port_locations.back());

    if (std::string(descriptor->PortNames[i]) == "Decay (s)") { decay_control = port_locations.back(); }
    if (std::string(descriptor->PortNames[i]) == "Relative wet signal level") {
      wet_level_control = port_locations.back();
    }
    if (std::string(descriptor->PortNames[i]) == "Input") {
      for (std::size_t i = 0; i < number_of_samples; ++i) {
        port_locations.back()[i] = static_cast<LADSPA_Data>(i);
      }
    }
  }

  descriptor->activate(reverb);
  constexpr std::size_t n_cycles = 10;
  for (std::size_t i = 0; i < n_cycles; ++i) {
    *decay_control     = i * 0.5;
    *wet_level_control = (n_cycles - i) * 0.05;
    // TODO: Crashes if run with the full number of samples
    descriptor->run(reverb, number_of_samples - 1000);
  }

  descriptor->cleanup(reverb);

  for (std::size_t i = 0; i < descriptor->PortCount; ++i) {
    if (LADSPA_IS_PORT_AUDIO(descriptor->PortDescriptors[i])) {
      delete[] port_locations.at(i);
    } else {
      delete port_locations.at(i);
    }
  }
}
