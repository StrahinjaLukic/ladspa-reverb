#include "ladspa.h"

#include <gtest/gtest.h>

TEST(LadspaDescriptorTest, GetDescriptor) {
  auto descriptor = ladspa_descriptor(0);
  ASSERT_NE(nullptr, descriptor);
}
