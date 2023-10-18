 /*
 * Copyright (c) 2023 Callender-Consulting
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *  framer.c  -- Find start of caliper frame.
 */ 
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdlib.h>

#include "framer.h"
#include "caliper.h"
#include "caliper_gpio.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(framer, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define FRAMER_FRAME_TOTAL_BITS  24

#define HIGH        1
#define LOW         0

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static const struct gpio_dt_spec clock_spec = 
                        GPIO_DT_SPEC_GET_OR(CLOCK_NODE, gpios, 0);

#if logic_analyzer_testing
static const struct gpio_dt_spec debug_spec = 
                        GPIO_DT_SPEC_GET_OR(DEBUG_NODE, gpios, 0);
#endif

void framerActiveCallback(struct k_timer * timer);
void framerAlignmentCallback(struct k_timer * timer);

K_TIMER_DEFINE(framer_active_timer,    framerActiveCallback,    NULL);
K_TIMER_DEFINE(framer_alignment_timer, framerAlignmentCallback, NULL);

static bool active = false;
static bool aligned = false;

static bool caliper_power_state = CALIPER_POWER_OFF;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int framerRead(const struct gpio_dt_spec * spec)
{
    return gpio_pin_get_dt(spec);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void framerWrite(const struct gpio_dt_spec * spec, int val)
{
    gpio_pin_set_dt(spec, val);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
bool is_caliper_on(void)
{
    LOG_INF("%s: %s", __func__, 
           (caliper_power_state == CALIPER_POWER_ON) ? "ON" : "OFF");

    return caliper_power_state;
}

#if logic_analyzer_testing
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void framerPulseDebug(void)
{
    framerWrite(&debug_spec, 0);
    framerWrite(&debug_spec, 1);  // toggle debug line
    framerWrite(&debug_spec, 0);
}
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void framerActiveCallback(struct k_timer * timer)
{
    LOG_DBG("Caliper is \"OFF\"");

    active = false;
    caliper_power_state = CALIPER_POWER_OFF;

    k_timer_stop(&framer_active_timer);
    k_timer_stop(&framer_alignment_timer);

#if logic_analyzer_testing
    framerPulseDebug();
    framerPulseDebug();
    framerPulseDebug();
#endif    
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void framerAlignmentCallback(struct k_timer * timer)
{
    LOG_DBG("%s", __func__);

    aligned = false;

    k_timer_stop(&framer_alignment_timer);
}

/*---------------------------------------------------------------------------*/
/* NOTE: this function runs on workqueue thread, not interrupt level         */
/*---------------------------------------------------------------------------*/
void framer_find_interframe_gap(void)
{
    int i;

    LOG_DBG("%s", __func__);

    active  = true;
    aligned = true;

#if logic_analyzer_testing
    framerPulseDebug();  // indicate start of search
#endif

    /*
     *  Start active_timer to catch caliper Powered-Off state.
     */
    k_timer_start(&framer_active_timer, K_MSEC(500), K_NO_WAIT);

    while (active) {

        k_timer_start(&framer_alignment_timer, K_MSEC(30), K_NO_WAIT);

        for (i=0; i < (FRAMER_FRAME_TOTAL_BITS); i++) {
            while (active && aligned && framerRead(&clock_spec) == LOW)  { /*spin*/}
            while (active && aligned && framerRead(&clock_spec) == HIGH) { /*spin*/}
        }
        /*
         *  If framerActiveCallback has set active = false,
         *  which indicates the caliper is not powered on, then return.
         */
        if (!active) {
            return;
        }

        /*
         *  If started in middle of frame, e.g. misaligned, then
         *  delay a bit (20ms) and keep searching.
         */
        if (!aligned) {
            k_sleep(K_MSEC(20));
            aligned = true;
            continue;
        }

        if (i == FRAMER_FRAME_TOTAL_BITS) {
             /*
              *  Wait for clock to finally go high: it's interframe value.
              */
            while (framerRead(&clock_spec) == LOW)  { /*spin*/}

            caliper_power_state = CALIPER_POWER_ON;

            /*
             *  Delay to expected start of next frame.
             *  Interframe gap is 120ms, but use a bit less that this.
             */
            k_sleep(K_MSEC(110));

#if logic_analyzer_testing             
            framerPulseDebug();  // indicate near start of next frame.
#endif
            /*
             *  Insure all timers are stopped.
             */
            k_timer_stop(&framer_active_timer);
            k_timer_stop(&framer_alignment_timer);

            LOG_DBG("Found interframe gap");
            return;
        }
    }
}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void framer_init(void)
{
    LOG_INF("%s", __func__);

    (void) framerRead;
    (void) framerWrite;

#if logic_analyzer_testing
    /*
     *  optional: Initialize debug pin: output to logic analyzer
     */
    gpio_pin_configure_dt(&debug_spec, (GPIO_PULL_DOWN | GPIO_OUTPUT));
#endif
}
