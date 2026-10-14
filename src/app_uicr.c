/*
 *   app_uicr.c  -- persistent application parameters
 *
 * Copyright (c) 2023, Callender-Consulting LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * History: these two parameters (line_end, standard) used to be kept in the
 * nRF52840 UICR "CUSTOMER" registers, updated by erasing and rewriting the
 * whole UICR page.  That is unsafe on boards that use the Adafruit/Seeed UF2
 * bootloader (XIAO nRF52840): the bootloader's start address lives in UICR,
 * so a power loss between the erase and the rewrite leaves a board that
 * will not boot.
 *
 * The values are now stored with the Zephyr settings subsystem (NVS) in the
 * board's "storage" partition.  The API and the file name are unchanged so
 * the rest of the application is unaffected.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/settings/settings.h>
#include <errno.h>

#include "app_uicr.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(uicr, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define SETTINGS_TREE        "caliper"
#define SETTINGS_KEY_LINE    SETTINGS_TREE "/line_end"
#define SETTINGS_KEY_STD     SETTINGS_TREE "/standard"

static uint8_t current_line_end = INVALID_LINE_END;
static uint8_t current_standard = INVALID_STANDARD;

/*---------------------------------------------------------------------------*/
/*  Called by the settings subsystem for each stored "caliper/..." item.     */
/*---------------------------------------------------------------------------*/
static int caliper_settings_set(const char * name, size_t len,
                                settings_read_cb read_cb, void * cb_arg)
{
    const char * next;
    uint8_t      value;

    if (len != sizeof(value)) {
        return -EINVAL;
    }

    if (settings_name_steq(name, "line_end", &next) && !next) {
        if (read_cb(cb_arg, &value, sizeof(value)) < 0) {
            return -EIO;
        }
        current_line_end = value;
        return 0;
    }

    if (settings_name_steq(name, "standard", &next) && !next) {
        if (read_cb(cb_arg, &value, sizeof(value)) < 0) {
            return -EIO;
        }
        current_standard = value;
        return 0;
    }

    return -ENOENT;
}

SETTINGS_STATIC_HANDLER_DEFINE(caliper_params, SETTINGS_TREE, NULL,
                               caliper_settings_set, NULL, NULL);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
line_end_t app_uicr_get_line_end(void)
{
    LOG_DBG("%s: LINE_END: 0x%x", __func__, current_line_end);

    return (line_end_t) current_line_end;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void app_uicr_set_line_end(line_end_t line_end)
{
    uint8_t value = (uint8_t) line_end;
    int     ret;

    LOG_DBG("%s: LINE_END: 0x%x", __func__, value);

    ret = settings_save_one(SETTINGS_KEY_LINE, &value, sizeof(value));
    if (ret) {
        LOG_ERR("%s: save failed: %d", __func__, ret);
        return;
    }
    current_line_end = value;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
standard_t app_uicr_get_standard(void)
{
    LOG_DBG("%s: STANDARD: 0x%x", __func__, current_standard);

    return (standard_t) current_standard;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void app_uicr_set_standard(standard_t standard)
{
    uint8_t value = (uint8_t) standard;
    int     ret;

    LOG_DBG("%s: STANDARD: 0x%x", __func__, value);

    ret = settings_save_one(SETTINGS_KEY_STD, &value, sizeof(value));
    if (ret) {
        LOG_ERR("%s: save failed: %d", __func__, ret);
        return;
    }
    current_standard = value;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void app_uicr_init(void)
{
    int ret;

    LOG_INF("%s", __func__);

    /*
     *  CONFIG_BT_SETTINGS is off, so Bluetooth does not initialize the
     *  settings subsystem for us.  This call is harmless if repeated.
     */
    ret = settings_subsys_init();
    if (ret) {
        LOG_ERR("%s: settings_subsys_init failed: %d", __func__, ret);
    }

    ret = settings_load_subtree(SETTINGS_TREE);
    if (ret) {
        LOG_ERR("%s: settings_load_subtree failed: %d", __func__, ret);
    }

    if (current_line_end != ASCIIZ && current_line_end != NEWLINE) {
        app_uicr_set_line_end(NEWLINE);               // set default
    }

    if (current_standard != INCLUDE && current_standard != EXCLUDE) {
        app_uicr_set_standard(INCLUDE);               // set default
    }

    LOG_INF("[LINE_END] 0x%x", current_line_end);
    LOG_INF("[STANDARD] 0x%x", current_standard);
}
