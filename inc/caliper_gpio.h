/*
 * Copyright (c) 2023 Callender-Consulting
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *   caliper_gpio.h
 */
#ifndef __CALIPER_GPIO_H
#define __CALIPER_GPIO_H

/*---------------------------------------------------------------------------*/
/*  Pins related to caliper                                                  */
/*---------------------------------------------------------------------------*/

#define CLOCK_NODE      DT_ALIAS(caliper_clock)
#define CLOCK           DT_GPIO_PIN(DT_ALIAS(caliper_clock), gpios)
#define CLOCK_PORT      DT_LABEL(DT_PHANDLE_BY_IDX(DT_ALIAS(caliper_clock), gpios, 0))
#define CLOCK_FLAGS     DT_PHA_BY_IDX(DT_ALIAS(caliper_clock), gpios, 0, flags)
#define CLOCK_LABEL     DT_PROP(DT_ALIAS(caliper_clock), label)

#define DATA_NODE       DT_ALIAS(caliper_data)
#define DATA            DT_GPIO_PIN(DT_ALIAS(caliper_data), gpios)
#define DATA_PORT       DT_LABEL(DT_PHANDLE_BY_IDX(DT_ALIAS(caliper_data), gpios, 0))
#define DATA_FLAGS      DT_PHA_BY_IDX(DT_ALIAS(caliper_data), gpios, 0, flags)
#define DATA_LABEL      DT_PROP(DT_ALIAS(caliper_data), label)

#define BUZZER_NODE     DT_ALIAS(caliper_buzzer)
#define BUZZER          DT_GPIO_PIN(DT_ALIAS(caliper_buzzer), gpios)
#define BUZZER_PORT     DT_LABEL(DT_PHANDLE_BY_IDX(DT_ALIAS(caliper_buzzer), gpios, 0))
#define BUZZER_FLAGS    DT_PHA_BY_IDX(DT_ALIAS(caliper_buzzer), gpios, 0, flags)
#define BUZZER_LABEL    DT_PROP(DT_ALIAS(caliper_buzzer), label)

#endif  /* __CALIPER_GPIO_H */