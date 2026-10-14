/*
 *  buzzer.c  -- buzzer driver
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Drives a passive piezo buzzer (CMT-322-65-SMT-TR) with a 4 kHz, 50% duty
 *  square wave from a PWM channel (via a FET on the board).  The part does
 *  not self-oscillate, so a plain GPIO level will not make it sound.
 *
 *  Zephyr 4.4 has no in-tree buzzer driver, so this uses the generic PWM API
 *  on the "caliper-pwm-buzzer" node aliased as "caliper-buzzer".
 *
 *  All PWM operations happen on the system workqueue (never in ISR context),
 *  because the playlist is stepped by a timer and PWM drivers may sleep.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

#include "caliper_gpio.h"
#include "buzzer.h"
#include "tones.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(buzzer, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*  CMT-322-65-SMT-TR drive requirement: 4000 Hz square wave, 50% duty.      */
/*---------------------------------------------------------------------------*/

#define BUZZER_TONE_HZ      4000
#define BUZZER_PERIOD_NS    PWM_HZ(BUZZER_TONE_HZ)
#define BUZZER_PULSE_NS     (BUZZER_PERIOD_NS / 2)

static const struct pwm_dt_spec buzzer_pwm = PWM_DT_SPEC_GET(BUZZER_NODE);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static void buzzer_step_handler(struct k_work * work);

static K_WORK_DELAYABLE_DEFINE(buzzer_work, buzzer_step_handler);

static buzzer_play_t * next_step;       /* next playlist entry to execute    */
static bool            in_play = false; /* a playlist is currently running   */

/*---------------------------------------------------------------------------*/
/*  Sound the buzzer. Duration is handled by our own work item, so the tone  */
/*  plays until buzzer_off() is called.                                      */
/*---------------------------------------------------------------------------*/
static void buzzer_on(void)
{
    int ret = pwm_set_dt(&buzzer_pwm, BUZZER_PERIOD_NS, BUZZER_PULSE_NS);
    if (ret) {
        LOG_ERR("pwm_set_dt failed: %d", ret);
    }
}

/*---------------------------------------------------------------------------*/
/*  Silence the buzzer. Pulse = 0 leaves the output idle (low, FET off).     */
/*---------------------------------------------------------------------------*/
static void buzzer_off(void)
{
    int ret = pwm_set_pulse_dt(&buzzer_pwm, 0);
    if (ret) {
        LOG_ERR("pwm_set_pulse_dt failed: %d", ret);
    }
}

/*---------------------------------------------------------------------------*/
/*  Execute one playlist entry, then schedule the next one.                  */
/*---------------------------------------------------------------------------*/
static void buzzer_step_handler(struct k_work * work)
{
    buzzer_play_t * step = next_step;

    if (step == NULL) {
        buzzer_off();
        in_play = false;
        return;
    }

    switch (step->action) {

        case BUZZER_PLAY_TONE:
            buzzer_on();
            next_step = &step[1];
            k_work_reschedule(&buzzer_work, K_MSEC(step->duration));
            break;

        case BUZZER_PLAY_QUIET:
            buzzer_off();
            next_step = &step[1];
            k_work_reschedule(&buzzer_work, K_MSEC(step->duration));
            break;

        case BUZZER_PLAY_DONE:
        default:
            buzzer_off();
            next_step = NULL;
            in_play = false;
            break;
    }
}

/*---------------------------------------------------------------------------*/
/*  Start playing a list. Returns -1 if a list is already playing.           */
/*---------------------------------------------------------------------------*/
uint32_t buzzer_play(buzzer_play_t * playlist)
{
    LOG_DBG("%s", __func__);

    if (in_play || playlist == NULL) {
        return -1;
    }

    in_play   = true;
    next_step = playlist;

    k_work_reschedule(&buzzer_work, K_NO_WAIT);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*  Stop whatever is playing, immediately.                                   */
/*---------------------------------------------------------------------------*/
void buzzer_stop(void)
{
    k_work_cancel_delayable(&buzzer_work);
    next_step = NULL;
    in_play   = false;
    buzzer_off();
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buzzer_init(void)
{
    LOG_INF("%s", __func__);

    if (!pwm_is_ready_dt(&buzzer_pwm)) {
        LOG_ERR("Error: buzzer PWM device %s is not ready",
                buzzer_pwm.dev->name);
        return;
    }

    LOG_INF("buzzer PWM '%s' channel %u, tone %d Hz",
            buzzer_pwm.dev->name, buzzer_pwm.channel, BUZZER_TONE_HZ);

    buzzer_off();

    buzzer_play(&startup_sound);
}
