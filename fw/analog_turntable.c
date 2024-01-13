#include <stdint.h>

#include "analog_turntable.h"

void analog_turntable_init(analog_turntable* self, uint32_t deadzone, uint32_t sustain_ms, bool clear) {
  self->deadzone = deadzone;
  self->sustain_ms = sustain_ms;
  self->clear = clear;
  self->center = 0;
  self->center_valid = false;
  self->state = 0;
  timer_init(&self->sustain_timer);
}

int8_t analog_turntable_poll(analog_turntable* self, uint32_t current_value) {
  if (!self->center_valid) {
    self->center_valid = true;
    self->center = current_value;
  }

  uint8_t observed = current_value;
  int8_t delta = observed - self->center;

  // is the current value sufficiently far away from the center?
  uint8_t direction = 0;
  if (delta >= (int32_t)self->deadzone) {
    direction = 1;
  } else if (delta <= -(int32_t)self->deadzone) {
    direction = -1;
  }

  if (direction != 0) {
    // turntable is moving
    // keep updating the new center, and keep extending the sustain timer
    self->center = observed;
    timer_arm(&self->sustain_timer, self->sustain_ms);
  } else if (timer_check_if_expired_reset(&self->sustain_timer)) {
    // sustain timer expired, time to reset to neutral
    self->state = 0;
    self->center = observed;
  }

  if (direction == -self->state && self->clear) {
    self->state = direction;
    return 0;
  } else if (direction != 0) {
    self->state = direction;
  }

  return self->state;
}
