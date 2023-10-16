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

#if 1
static const struct gpio_dt_spec debug_spec = 
                        GPIO_DT_SPEC_GET_OR(DEBUG_NODE, gpios, 0});
#endif

void framerInactiveCallback(struct k_timer * timer);
void framerInterframeCallback(struct k_timer * timer);

K_TIMER_DEFINE(framer_inactive_timer,  framerInactiveCallback,   NULL);
K_TIMER_DEFINE(framer_interframe_timer, framerInterframeCallback, NULL);

static bool inactive = false;
static bool running = false;

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
void framerInactiveCallback(struct k_timer * timer)
{
    LOG_INF("Caliper is \"OFF\"");

    running  = false;
    inactive = true;

    k_timer_stop(&framer_inactive_timer);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void framerInterframeCallback(struct k_timer * timer)
{
    LOG_INF("%s", __func__);

    k_timer_stop(&framer_interframe_timer);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
int framer_find_interframe_gap(void)
{
    LOG_INF("%s", __func__);

    running = true;
    inactive = false;

    k_timer_start(&framer_interframe_timer, K_MSEC(500), K_NO_WAIT);

    while (running) {

        k_timer_start(&framer_interframe_timer, K_MSEC(100), K_NO_WAIT);

        for (int i=1; i < (FRAMER_FRAME_TOTAL_BITS); i++) {
            while (framerRead(&clock_spec) == LOW)  { /*spin*/}
            while (framerRead(&clock_spec) == HIGH) { /*spin*/}        
        }
        
        k_timer_stop(&framer_inactive_timer);
        
        return 1;  // successfully found interframe
    }

    if (inactive)
        return -1;  // caliper is powered off.

    return 0;       // ????
}


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void framer_init(void)
{
    LOG_INF("%s", __func__);

#if 1
    /*
     *  optional: Initialize debug pin: output to logic analyzer
     */
    gpio_pin_configure_dt(&debug_spec, (GPIO_PULL_DOWN | GPIO_OUTPUT));
#endif
}

