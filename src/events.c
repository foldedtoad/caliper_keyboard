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
#include "app_uicr.h"
#include "ble_base.h"
#include "battery.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(events, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static char string[16];

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void events_build_string(short value, int standard)
{
    float value_float = ((float) value);
    char * line_end;

    memset(&string, 0, sizeof(string));

    if (standard == CALIPER_STANDARD_MM)
        value_float /= 100.0;
    else
        value_float /= 1000.0;

    snprintf(string, sizeof(string), "%.2f", value_float);

    if (app_uicr_get_standard() == INCLUDE) {
        switch (standard) {
            default:
            case CALIPER_STANDARD_MM:
                strncat(string, " mm", sizeof(" mm")-1);
                break;
            case CALIPER_STANDARD_INCH:
                strncat(string, " inch", sizeof(" inch")-1);             
                break;
        }
    }

    switch (app_uicr_get_line_end()) { 
        default:
        case ASCIIZ:  line_end = "";    break;
        case NEWLINE: line_end = "\n";  break;
    }
    strncat(string, line_end, strlen(line_end));

    LOG_INF("string: %s", string);
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
        LOG_WRN("Caliper is off");
        return;
    }

    if (is_bt_connected() == false) {
        LOG_ERR("BLE not connected");
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
static void events_ble_connect(bool connected)
{
    LOG_INF("%s: connected(%s)", __func__, 
            (connected == true) ? "true" : "false");

    battery_ble_events(connected);

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

    /*
     *  Register for BLE connect/disconnect events.
     */
    ble_register_connect_handler(events_ble_connect);
}