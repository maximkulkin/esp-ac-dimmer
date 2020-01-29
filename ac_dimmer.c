#ifdef AC_DIMMER_DEBUG
#define ac_dimmer_debug(message, ...) printf("AC dimmer: " message, ## __VA_ARGS__)
#else
#define ac_dimmer_debug(message, ...)
#endif

#include <esp/gpio.h>
#include <esp/timer.h>
#include "ac_dimmer.h"


#define MIN(a, b) (((b) < (a)) ? (b) : (a))


#ifndef AC_DIMMER_MAX_COUNT
#define AC_DIMMER_MAX_COUNT 2
#endif


#define AC_DIMMER_DEFAULT_FREQUENCY 60
#define AC_DIMMER_MAX_DUTY 100


typedef struct {
    ac_dimmer_config_t config;
    uint8_t duty;
} ac_dimmer_t;


static volatile uint8_t counter;

static ac_dimmer_t dimmers[AC_DIMMER_MAX_COUNT];
static uint8_t dimmers_count = 0;



static void IRAM ac_dimmer_timer_handler(void *arg) {
    counter++;

    for (uint8_t i=0; i<dimmers_count; i++) {
        ac_dimmer_t *dimmer = &dimmers[i];

        bool on = false;

        switch (dimmer->config.type) {
            case ac_dimmer_forward_phase:
                on = (counter >= AC_DIMMER_MAX_DUTY-dimmer->duty);
                break;
            case ac_dimmer_reverse_phase:
                on = (counter <= dimmer->duty);
                break;
            case ac_dimmer_center_notch: {
                int d = (counter - AC_DIMMER_MAX_DUTY/2 + dimmer->duty/2);
                on = (d >= 0 && d < dimmer->duty);
                break;
            }
        }

        gpio_write(dimmer->config.pwm_gpio, dimmer->config.inverse ? !on : on);
    }
}


void IRAM zero_crossing_interrupt(uint8_t gpio) {
    counter = 0;
}


static uint32_t ac_dimmer_timer_frequency(uint8_t ac_frequency) {
    // Split each half-wave into duty resolution periods
    return ac_frequency * 2 * AC_DIMMER_MAX_DUTY;
}


int ac_dimmer_init(uint8_t zero_crossing_gpio) {
    timer_set_interrupts(FRC1, false);
    timer_set_run(FRC1, false);

    _xt_isr_attach(INUM_TIMER_FRC1, ac_dimmer_timer_handler, NULL);
    timer_set_reload(FRC1, true);

    timer_set_frequency(FRC1, ac_dimmer_timer_frequency(AC_DIMMER_DEFAULT_FREQUENCY));
    timer_set_interrupts(FRC1, true);

    gpio_enable(zero_crossing_gpio, GPIO_INPUT);
    gpio_set_interrupt(zero_crossing_gpio, GPIO_INTTYPE_EDGE_ANY, zero_crossing_interrupt);

    return 0;
}


void ac_dimmer_set_frequency(uint8_t ac_frequency) {
    timer_set_frequency(FRC1, ac_dimmer_timer_frequency(ac_frequency));
}


int ac_dimmer_create(ac_dimmer_config_t config) {
    if (dimmers_count >= AC_DIMMER_MAX_COUNT-1) {
        return -1;
    }

    ac_dimmer_t *dimmer = &dimmers[dimmers_count];
    dimmer->config = config;
    dimmer->duty = 0;

    gpio_enable(dimmer->config.pwm_gpio, GPIO_OUTPUT);

    dimmers_count++;

    if (dimmers_count == 1) {
        timer_set_run(FRC1, true);
    }

    return 0;
}


void ac_dimmer_delete(uint8_t pwm_gpio) {
    int idx = -1;
    for (int i=0; i<dimmers_count; i++) {
        if (dimmers[i].config.pwm_gpio == pwm_gpio) {
            idx = i;
            break;
        }
    }
    if (idx == -1)
        return;

    dimmers_count--;
    if (idx < dimmers_count)
        dimmers[idx] = dimmers[dimmers_count];

    if (dimmers_count == 0) {
        timer_set_run(FRC1, false);
    }
}


static ac_dimmer_t *ac_dimmer_find_by_pwm(uint8_t pwm_gpio) {
    for (int i=0; i<dimmers_count; i++) {
        if (dimmers[i].config.pwm_gpio == pwm_gpio)
            return &dimmers[i];
    }
    return NULL;
}


void ac_dimmer_set_duty(uint8_t pwm_gpio, uint8_t duty) {
    ac_dimmer_t *dimmer = ac_dimmer_find_by_pwm(pwm_gpio);
    if (!dimmer)
        return;

    dimmer->duty = MIN(duty, AC_DIMMER_MAX_DUTY);
}
