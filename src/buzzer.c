/*
 *  buzzer.c  -- buzzer driver
 *
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Drives a CMT-322-65-SMT-TR piezo buzzer through Zephyr's generic
 *  buzzer driver API (zephyr/drivers/buzzer.h). Unlike the original
 *  buzzer, this part is passive and requires an externally supplied
 *  square-wave drive signal (it does not self-oscillate); the "pwm-buzzer"
 *  backend (bound to P0.03 in the board DTS) supplies that signal.
 *
 *  Volume is fixed at BUZZER_VOLUME_MAX so the backend always drives a
 *  50% duty-cycle square wave -- perceived loudest for a piezo, and the
 *  duty cycle called for by the CMT-322-65-SMT-TR datasheet.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/buzzer.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

#include "caliper_gpio.h"
#include "buzzer.h"
#include "tones.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(buzzer, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*  CMT-322-65-SMT-TR drive requirement: 4000 Hz square wave.               */
/*---------------------------------------------------------------------------*/

#define BUZZER_TONE_HZ      4000

static const struct device * const buzzer_dev = DEVICE_DT_GET(BUZZER_PWM_NODE);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void buzzer_timeout_callback(struct k_timer *timer);

K_TIMER_DEFINE(buzzer_timer, buzzer_timeout_callback, NULL);

#define TIMER_DELAY_ONE_MS  K_MSEC(1)

static bool  in_play = false;

/*---------------------------------------------------------------------------*/
/*  Drive the buzzer at BUZZER_TONE_HZ. Duration is left to our own timer   */
/*  (see buzzer_process_playlist below), so the tone plays until explicitly */
/*  stopped rather than the driver auto-silencing it after some duration.  */
/*---------------------------------------------------------------------------*/
void buzzer_on(void)
{
    buzzer_tone(buzzer_dev, BUZZER_TONE_HZ, BUZZER_DURATION_FOREVER);
}

/*---------------------------------------------------------------------------*/
/*  Silence the buzzer.                                                     */
/*---------------------------------------------------------------------------*/
void buzzer_off(void)
{
    buzzer_stop(buzzer_dev);
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
            break;

        case BUZZER_PLAY_QUIET:
            buzzer_off();
            buzzer_timer.user_data = &playlist[1];
            duration = K_MSEC(playlist->duration);
            k_timer_start(&buzzer_timer, duration, K_NO_WAIT);
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
    LOG_DBG("%s", __func__);

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
void buzzer_init(void)
{
    (void) buzzer_on;
    (void) buzzer_off;

    LOG_INF("%s", __func__);

    if (!device_is_ready(buzzer_dev)) {
        LOG_ERR("Error: buzzer device %s is not ready", buzzer_dev->name);
        return;
    }

    LOG_INF("buzzer '%s', tone %d Hz", buzzer_dev->name, BUZZER_TONE_HZ);

    /* Force max volume so the backend always drives a 50% duty cycle. */
    buzzer_set_volume(buzzer_dev, BUZZER_VOLUME_MAX);

    buzzer_off();

    buzzer_play(&startup_sound);
}
