/*
 * Copyright (c) 2023 Callender-Consulting
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *   caliper_gpio.h
 */
#ifndef __CALIPER_GPIO_H
#define __CALIPER_GPIO_H

#include <zephyr/devicetree.h>

/*---------------------------------------------------------------------------*/
/*  Pins related to caliper.  The board (or an overlay) provides the nodes   */
/*  and these aliases:  caliper-clock, caliper-data, caliper-buzzer.         */
/*  Optional:           caliper-debug  (logic-analyzer test point).          */
/*---------------------------------------------------------------------------*/

#define CLOCK_NODE      DT_ALIAS(caliper_clock)
#define CLOCK           DT_GPIO_PIN(CLOCK_NODE, gpios)
#define CLOCK_LABEL     DT_PROP(CLOCK_NODE, label)

#define DATA_NODE       DT_ALIAS(caliper_data)
#define DATA            DT_GPIO_PIN(DATA_NODE, gpios)
#define DATA_LABEL      DT_PROP(DATA_NODE, label)

#define DEBUG_NODE      DT_ALIAS(caliper_debug)

/* Passive piezo buzzer, driven by a PWM channel ("caliper-pwm-buzzer"). */
#define BUZZER_NODE     DT_ALIAS(caliper_buzzer)

#endif  /* __CALIPER_GPIO_H */
