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

#include "shell.h"
#include "keyboard.h" 
#include "app_uicr.h" 

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(shell, 3);

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

    line_end = (app_uicr_get_line_end() == NEWLINE) ? "NEWLINE" : "ASCIIZ";

    standard = (app_uicr_get_standard() == INCLUDE) ? "INCLUDE" : "EXCLUDE";

    shell_print(sh, "** Welcome to Caliper Keyboard");
    shell_print(sh, "** Built on %s at %s", __DATE__, __TIME__);
    shell_print(sh, "** Board '%s'", CONFIG_BOARD);
    shell_print(sh, "** Parameters --");
    shell_print(sh, "**   [line_end] %s", line_end);
    shell_print(sh, "**   [standard] %s", standard);

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_line_end(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    line_end_t state;
    char * string;

    state = app_uicr_get_line_end();

    switch (state) {

        case ASCIIZ:
            app_uicr_set_line_end(NEWLINE);
            string = "NEWLINE"; 
            break;

        case NEWLINE:
            app_uicr_set_standard(ASCIIZ);
            string = "ASCIIZ";
            break;

        default:
            shell_print(sh, "unknown [line_end] state %d", state);
            string = "???";
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_standard(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    standard_t state;
    char * string;

    state = app_uicr_get_standard();

    switch (state) {

        case INCLUDE:
            app_uicr_set_standard(EXCLUDE);
            string = "EXCLUDE"; 
            break;

        case EXCLUDE:
            app_uicr_set_standard(INCLUDE);
            string = "INCLUDE";
            break;

        default:
            shell_print(sh, "unknown [standard] state %d", state);
            string = "???";
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

    shell_print(sh, "test: %s", string);

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
