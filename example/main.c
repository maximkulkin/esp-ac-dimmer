#include "ac_dimmer.h"


void user_init() {
    ac_dimmer_init(12);

    ac_dimmer_create((ac_dimmer_config_t){
        .pwm_gpio=13,
        .type=ac_dimmer_forward_phase,
    });
    ac_dimmer_set_duty(13, 50);
}
