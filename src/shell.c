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

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(shell, 3);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_info(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    LOG_INF("** Info ***");

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int cmd_shell_test(const struct shell *sh, size_t argc, char *argv[])
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    LOG_INF("** Test ***");

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

SHELL_STATIC_SUBCMD_SET_CREATE(caliper_commands,
    SHELL_CMD(test, NULL, "Caliper Test\n", cmd_shell_test),
    SHELL_CMD(info, NULL, "Caliper Info\n", cmd_shell_info),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(caliper, &caliper_commands,
           "Caliper commands", NULL);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
int caliper_shell_init(void)
{
    static const struct device * uart_dev;

    uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));

#if 1
    if (!uart_dev) {
        //LOG_ERR("device not found. %s", DEVICE_DT_NAME(UART_NODEs);
        return -1;
    }

    LOG_INF("Board '%s', UART '%s', device %p",
            CONFIG_BOARD, uart_dev->name, uart_dev);

#endif

    return 0;
}