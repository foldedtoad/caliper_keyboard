/*
 * Copyright (c) 2020, Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/reboot.h> 
#include <zephyr/bluetooth/services/bas.h>

#include "shell.h"
#include "keyboard.h" 
#include "app_uicr.h" 
#include "ble_base.h"
#include "framer.h"
#include "caliper.h"
#include "battery.h"
#include "buttons.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_shell, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static const struct device * const device =
                  DEVICE_DT_GET_OR_NULL(DT_NODELABEL(uart0));

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_info(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    char * line_end;
    char * standard;
    int8_t level;

    framer_find_interframe_gap();

    switch (app_uicr_get_line_end()) { 
        case ASCIIZ:  line_end = "ASCIIZ";    break;
        case NEWLINE: line_end = "NEWLINE";   break;
        default:      line_end = "<unknown>"; break;
    }

    switch (app_uicr_get_standard()) {
        case INCLUDE: standard = "INCLUDE";   break;
        case EXCLUDE: standard = "EXCLUDE";   break;
        default:      standard = "<unknown>"; break;
    }

    shell_print(sh, "** Welcome to Caliper Keyboard");
    shell_print(sh, "** Built on %s at %s", __DATE__, __TIME__);
    shell_print(sh, "** Board '%s'", CONFIG_BOARD);
    shell_print(sh, "** Caliper %s", is_caliper_on()?"ON":"OFF");
    shell_print(sh, "** BLE connected: %s", is_bt_connected()?"yes":"no");

    level = bt_bas_get_battery_level();
    if (level <= BATTERY_LEVEL_INVALID)
        shell_print(sh, "** Battery level: inactive");
    else
        shell_print(sh, "** Battery level: %u%%", level);

    shell_print(sh, "** Parameters --");
    shell_print(sh, "**   [line_end] %s", line_end);
    shell_print(sh, "**   [standard] %s", standard);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_snap(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    buttons_remote_button();

    shell_print(sh, "snap!");

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_reboot(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "reboot!");

    k_sleep(K_SECONDS(2));
    sys_reboot(SYS_REBOOT_WARM);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_line_end(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    char * string;

    /* Rotate to next line_end type */
    switch (app_uicr_get_line_end()) {
        case ASCIIZ:  app_uicr_set_line_end(NEWLINE); string = "NEWLINE"; break;
        case NEWLINE: app_uicr_set_line_end(ASCIIZ);  string = "ASCIIZ";  break;
        default:                                      string = "???";     break;
    }

    shell_print(sh, "[line_end] %s", string);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_standard(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    char * string;

    switch (app_uicr_get_standard()) {
        case INCLUDE: app_uicr_set_standard(EXCLUDE); string = "EXCLUDE"; break;
        case EXCLUDE: app_uicr_set_standard(INCLUDE); string = "INCLUDE"; break;
        default:                                      string = "???";     break;
    }

    shell_print(sh, "[standard] %s", string);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_test(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    char * string = (char*) argv[1];

    shell_print(sh, "test: %s\n", string);

    keyboard_send_string(string);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

SHELL_STATIC_SUBCMD_SET_CREATE(caliper_cmds,
    SHELL_CMD_ARG(test, NULL, "caliper test <string>", cmd_shell_test, 2, 0),
    SHELL_CMD(line_end, NULL, "caliper line_end (toggle)", cmd_shell_line_end),
    SHELL_CMD(standard, NULL, "caliper standard (toggle)", cmd_shell_standard),
    SHELL_CMD(info,     NULL, "caliper info", cmd_shell_info),
    SHELL_CMD(snap,     NULL, "caliper snap (snapshot)", cmd_shell_snap),
    SHELL_CMD(reboot,   NULL, "caliper reboot", cmd_shell_reboot),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(caliper, &caliper_cmds, "Caliper commands", NULL);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
int caliper_shell_init(void)
{
    if (!device) {
        LOG_ERR("device not found");
        return -1;
    }

    LOG_INF("UART '%s'", device->name);

    return 0;
}
