#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ac_dimmer_forward_phase,
    ac_dimmer_reverse_phase,
    ac_dimmer_center_notch,
} ac_dimmer_type_t;


typedef struct {
    uint8_t pwm_gpio;
    ac_dimmer_type_t type;
    bool inverse;
} ac_dimmer_config_t;


int ac_dimmer_init(uint8_t zero_crossing_gpio);
void ac_dimmer_set_frequency(uint8_t ac_frequency);

int ac_dimmer_create(ac_dimmer_config_t config);
void ac_dimmer_delete(uint8_t pwm_gpio);
void ac_dimmer_set_duty(uint8_t pwm_gpio, uint8_t duty);
