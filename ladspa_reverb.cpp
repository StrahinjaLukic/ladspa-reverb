#include "ladspa_reverb.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "port_id.h"

namespace {
constexpr int kNDelays = 4;
constexpr int kNPasses = 4;
} // namespace

Reverb::Reverb(unsigned long sample_rate, double microsample_duration_s)
    : microsample_duration_s_(microsample_duration_s),
      sample_rate_(sample_rate) {
}

void Reverb::ResetWindow(double delay_factor) {
  window_.resize(std::lround(microsample_duration_s_ * delay_factor * sample_rate_), 0);
  window_write_position_ = 0;
  window_read_position_  = 0;
}

bool Reverb::UpdateDecay() {
  const auto decay_from_port = decay_s_port_ != nullptr ? std::max(*decay_s_port_, kMinimumDecayS) : kDefaultDecayS;
  if (decay_s_ != decay_from_port) {
    decay_s_ = decay_from_port;
    return true;
  }
  return false;
}

bool Reverb::UpdateWetLevel() {
  const auto wet_level_from_port = wet_level_port_ != nullptr
                                           ? std::clamp(*wet_level_port_, kMinimumWetLevel, kMaximumWetLevel)
                                           : kDefaultWetLevel;
  if (wet_level_ != wet_level_from_port) {
    wet_level_ = wet_level_from_port;
    return true;
  }
  return false;
}

void Reverb::UpdateWeight(bool force) {
  if (UpdateDecay() || force) {
    const auto t_buffer = static_cast<double>(window_.size()) / sample_rate_;
    weight_             = std::exp(-t_buffer / decay_s_);
    std::cout << "Reverb: Updating weight to " << weight_ << std::endl;
  }
}

void Reverb::ApplyWindow(unsigned long offset) {
  const auto i_end      = std::min(window_.size(), window_write_position_);
  const auto wet_factor = wet_level_ / weight_ / kNPasses;
  for (unsigned long i_window = window_read_position_; i_window < i_end; ++i_window) {
    output_[offset + i_window] += wet_factor * window_[i_window];
  }
  window_read_position_ = i_end;
  if (window_write_position_ == window_.size()) {
    window_write_position_ = 0;
    window_read_position_  = 0;
  }
}

void Reverb::Run(unsigned long sample_count) {
  UpdateWetLevel();
  UpdateWeight();
  std::size_t window_offset = 0;

  std::memcpy(output_, input_, sample_count * sizeof(LADSPA_Data));

  for (int i = 0; i < kNPasses; ++i) {

    const double decay_factor = 1 + 0.1765 * (i % kNDelays);
    ResetWindow(decay_factor);
    const double weight = weight_;

    for (unsigned long i_sample = 0; i_sample < sample_count; ++i_sample, ++window_write_position_) {
      if (window_offset + window_write_position_ >= sample_count) { break; }
      if (window_write_position_ >= window_.size()) {
        ApplyWindow(window_offset);
        window_offset += window_.size();
      }
      window_[window_write_position_] = weight * window_[window_write_position_] + (1. - weight) * output_[i_sample];
    }
    ApplyWindow(window_offset);
  }
}

void Reverb::ConnectPort(unsigned long port, LADSPA_Data *data_location) {
  switch (ToEnum(port)) {
  case PortId::kInput:
    input_ = data_location;
    return;
  case PortId::kOutput:
    output_ = data_location;
    return;
  case PortId::kDecay:
    decay_s_port_ = data_location;
    return;
  case PortId::kWetLevel:
    wet_level_port_ = data_location;
    return;
  case PortId::kPortCount:
    throw std::invalid_argument("PortId number out of range");
  }
  throw std::invalid_argument("PortId number unknown");
}
