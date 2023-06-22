/*
 *  events.c  -- events handler
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

#include "events.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(events, 3);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static char string[24];

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void events_build_string(short value, int standard)
{
    float value_float = ((float) value);

    if (standard == CALIPER_STANDARD_MM)
        value_float /= 100.0;
    else
        value_float /= 1000.0;

    snprintf(string, sizeof(string), "%.2f %s\n", value_float,
            (standard == CALIPER_STANDARD_MM) ? "mm" : "inch");

    LOG_INF("string: \"%s\"", string);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void events_snapshot(buttons_id_t btn_id)
{
    int   ret;
    short value;
    int   standard;

    LOG_INF("%s: Snapshot", __func__);

    if (is_caliper_on() == CALIPER_POWER_OFF) {
        LOG_INF("Caliper is off");
        return;
    }

    if (is_bt_connected() == false) {
        LOG_INF("BLE not connected");
        return;
    }

    ret = caliper_read_value(&value, &standard);
    if (ret != 0) {
        LOG_ERR("Read failed");
        return;
    }

    events_build_string(value, standard);

    keyboard_send_string((char*)&string);   
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void events_init(void)
{
    LOG_INF("%s", __func__);

    /* 
     *  Register for button press notifications.
     */
    buttons_register_notify_handler(events_snapshot);
}