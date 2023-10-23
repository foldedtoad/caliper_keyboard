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
#include <inttypes.h>

#include "caliper_gpio.h"
#include "buzzer.h"
#include "tones.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(buzzer, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static const struct gpio_dt_spec buzzer_spec = 
                        GPIO_DT_SPEC_GET_OR(BUZZER_NODE, gpios, 0);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void buzzer_timeout_callback(struct k_timer *timer);

K_TIMER_DEFINE(buzzer_timer, buzzer_timeout_callback, NULL);

#define TIMER_DELAY_ONE_MS  K_MSEC(1)

static bool  in_play = false;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_on(void)
{
    gpio_pin_set_dt(&buzzer_spec, 1);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_off(void)
{
    gpio_pin_set_dt(&buzzer_spec, 0);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buzzer_process_playlist(buzzer_play_t * playlist)
{
    k_timeout_t duration;

    LOG_DBG("%s", __func__);

    buzzer_off();

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
            buzzer_on();
            buzzer_timer.user_data = &playlist[1];
            duration = K_MSEC(playlist->duration);
            k_timer_start(&buzzer_timer, duration, K_NO_WAIT);
            LOG_INF("PLAY:  duration(%d)", playlist->duration);
            break;

        case BUZZER_PLAY_QUIET:
            buzzer_off();
            buzzer_timer.user_data = &playlist[1];
            duration = K_MSEC(playlist->duration);
            k_timer_start(&buzzer_timer, duration, K_NO_WAIT);
            LOG_INF("QUIET: duration(%d)", playlist->duration);
            break;

        case BUZZER_PLAY_DONE:
        default:
            buzzer_off();
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
    LOG_INF("%s", __func__);

    if (in_play) {
        return -1;
    }

    in_play = true;

    buzzer_process_playlist(playlist);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint32_t buzzer_work(buzzer_play_t * playlist)
{
    LOG_INF("%s", __func__);

    if (in_play) {
        return -1;
    }

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

    buzzer_off(); 
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_init(void)
{
    (void) buzzer_on;
    (void) buzzer_off;

    LOG_INF("%s", __func__);

    if (!device_is_ready(buzzer_spec.port)) {
        LOG_ERR("Error: Caliper %s is not ready", buzzer_spec.port->name);
        return;
    }

    LOG_INF("buzzer port '%s', pin %d", buzzer_spec.port->name, buzzer_spec.pin);

    gpio_pin_configure_dt(&buzzer_spec, (GPIO_PULL_DOWN | GPIO_OUTPUT));

    buzzer_play(&startup_sound);
}
