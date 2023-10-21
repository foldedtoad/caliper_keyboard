/*
 *  buzzer.c  -- buzzer driver
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/pwm.h> 
#include <inttypes.h>

#include "events.h"
#include "buzzer.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(buzzer, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static const struct pwm_dt_spec buzzer_spec = 
                        PWM_DT_SPEC_GET(DT_NODELABEL(pwm_buzzer0));

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void buzzer_timeout_callback(struct k_timer *timer);

K_TIMER_DEFINE(buzzer_timer, buzzer_timeout_callback, NULL);

static bool  in_play = false;

static uint32_t max_period;

#define MIN_PERIOD PWM_SEC(1) / 128U
#define MAX_PERIOD PWM_SEC(1)

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void set_frequency_and_duty_cycle(uint32_t frequency, 
                                         uint32_t duty_cycle_percent)
{
//    nrf_pwm_set_max_value((HFCLKHZ + (frequency / 2)) / frequency);
//    nrf_pwm_set_value(0, (HFCLKHZ / frequency) * duty_cycle_percent / 100);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_set_pwm_parms(void)
{
   
//    nrf_pwm_config_t pwm_config = PWM_DEFAULT_CONFIG;

    uint32_t frequency  = 2700;
    uint32_t duty_cycle = 50;

#if 0 
    pwm_config.mode          = PWM_MODE_BUZZER;
    pwm_config.num_channels  = 1;
    pwm_config.gpio_num[0]   = BUZZER_PIN;
    nrf_pwm_init(&pwm_config);
#endif

    set_frequency_and_duty_cycle(frequency, duty_cycle);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_process_playlist(buzzer_play_t * playlist)
{
//    gpio_pin_set_dt(buzzer_spec, 0);  // In sure buzzer GPIO is off.

    /*
     *   Single play.
     */
    if (playlist == NULL) {
        return;
    }

    /*
     *   Play list.
     */
    switch (playlist->action) {

        case BUZZER_PLAY_TONE:
            buzzer_set_pwm_parms();
//            nrf_pwm_set_enabled(true);

            k_timer_start(&buzzer_timer, K_MSEC(500), K_NO_WAIT);

//            app_timer_start(m_buzzer_timer_id,
//                            (playlist->duration * TIMER_DELAY_ONE_MS),
//                            &playlist[1]);
            break;

        case BUZZER_PLAY_QUIET:
            // Disable while quiet: conserve power.
//            nrf_pwm_set_enabled(false);

            k_timer_start(&buzzer_timer, K_MSEC(500), K_NO_WAIT);

//            app_timer_start(m_buzzer_timer_id,
//                            (playlist->duration * TIMER_DELAY_ONE_MS),
//                            &playlist[1]);
            break;

        case BUZZER_PLAY_DONE:
        default:
            // Disable when done: conserve power.
//            nrf_pwm_set_enabled(false);

            // Done: set BUZZER pin to "output".
            in_play = false;
            break;
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_timeout_callback(struct k_timer * timer)
{
    buzzer_play_t * playlist = (buzzer_play_t*) timer->user_data;

    buzzer_process_playlist(playlist);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint32_t buzzer_play(buzzer_play_t * playlist)
{
    if (in_play)
        return -1;
    in_play = true;

    buzzer_process_playlist(playlist);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_stop(void)
{
    k_timer_stop(&buzzer_timer);

//    nrf_pwm_set_enabled(false);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_init(void)
{
    if (!device_is_ready(buzzer_spec.dev)) {
        LOG_ERR("PWM device %s is not ready", buzzer_spec.dev->name);
        return;
    }

    LOG_INF("Calibrating pwm channel %d", buzzer_spec.channel);

    max_period = MAX_PERIOD;

    while (pwm_set_dt(&buzzer_spec, max_period, max_period / 2)) {
        max_period /= 2;
        if (max_period < (4 * MIN_PERIOD)) {
            LOG_ERR("Error: PWM device "
                   "does not support a period at least %lu\n",
                   4 * MIN_PERIOD);
            return;
        }
    }
    LOG_INF("Done calibrating; maximum/minimum periods %u/%lu nsec",
           max_period, MIN_PERIOD);

//    nrf_pwm_set_enabled(false);

}
