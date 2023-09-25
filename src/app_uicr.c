/*
 *   app_uicr.c
 *
 * Copyright (c) 2023, Callender-Consulting LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/uart.h> 
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>

#include <nrf.h>
#include <nrfx_nvmc.h>
#include <nrf_erratas.h>

#include "app_uicr.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(uicr, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define __UNINITIALIZED__ (-1)

#define __LINE_END__    0
#define __STANDARD__    1

#define REG_LINE_END  CUSTOMER[__LINE_END__] 
#define REG_STANDARD  CUSTOMER[__STANDARD__] 

static const struct device * const device =
                  DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_flash_controller));

/*
 *  Notes:
 *      flash erase flash-controller@4001e000 10001000 308  // for UICR page
 *      flash read  flash-controller@4001e000 10001080
 *      flash write flash-controller@4001e000 10001080 2
 *      (off_t)NRF_UICR = 0x10001000
 *      sizeof(*NRF_UICR) = 0x308
 */

static uint8_t copy_buffer [sizeof(*NRF_UICR)];


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
uint8_t app_uicr_read_one(off_t addr)
{
    uint8_t data;

    int ret = flash_read(device, addr, (void*)&data, sizeof(uint8_t));
    if (ret != 0) {
        LOG_ERR("%s: Flash_read failed: %d", __func__, ret);
        return ret;
    }

    LOG_DBG("%s: addr(%p): value(0x%x)", __func__, (void*)addr, data);

    return data;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void app_uicr_write_one(off_t addr, uint8_t data)
{
    int    ret;
    off_t  uicr_page_base = (off_t)NRF_UICR;
    size_t uicr_page_size = sizeof(*NRF_UICR);

    LOG_DBG("%s: addr: %p, data(%d)", __func__, (void*)addr, data);

    LOG_DBG("%s: read UICR", __func__);
    ret = flash_read(device, uicr_page_base,
                    (void*)&copy_buffer, uicr_page_size);
    if (ret != 0) {
        LOG_ERR("Flash_write: read UICR failed: %d", ret);
        return;
    }

    LOG_DBG("%s: erase UICR", __func__);
    ret = flash_erase(device, uicr_page_base, uicr_page_size);
    if (ret != 0) {
        LOG_ERR("Flash_write: erase UICR failed: %d", ret);
        return;
    }

    copy_buffer[addr - uicr_page_base] = data;

    LOG_DBG("%s: write UICR", __func__);
    ret = flash_write(device, uicr_page_base,
                      (void*)&copy_buffer, uicr_page_size);
    if (ret < 0) {
        LOG_ERR("Flash_write: write failed: %d", ret);
        return;
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
line_end_t app_uicr_get_line_end(void)
{
    line_end_t line_end = app_uicr_read_one((uint32_t)&NRF_UICR->REG_LINE_END);

    LOG_DBG("%s: Get LINE_END: addr [0x%x], value: 0x%x", __func__,
            (uint32_t)&NRF_UICR->REG_LINE_END, (uint8_t)line_end);    

    return line_end;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void app_uicr_set_line_end(line_end_t line_end)
{
    LOG_DBG("%s: Set LINE_END: addr [0x%x], value: 0x%x", __func__, 
            (uint32_t)&NRF_UICR->REG_LINE_END, line_end);

    app_uicr_write_one((off_t)&NRF_UICR->REG_LINE_END, (uint8_t)line_end);   
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
standard_t app_uicr_get_standard(void)
{
    standard_t standard = app_uicr_read_one((uint32_t)&NRF_UICR->REG_STANDARD);

    LOG_DBG("%s: Get STANDARD: addr: [0x%x], value: 0x%x", __func__,
            (uint32_t)&NRF_UICR->REG_STANDARD, (uint8_t)standard);

    return standard;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void app_uicr_set_standard(standard_t standard)
{
    LOG_DBG("%s: Set STANDARD: addr: [0x%x], value: 0x%x", __func__,
            (uint32_t)&NRF_UICR->REG_STANDARD, standard);

    app_uicr_write_one((uint32_t)&NRF_UICR->REG_STANDARD, (uint8_t)standard);   
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void app_uicr_init(void)
{
    LOG_INF("%s", __func__);

    line_end_t line_end;
    standard_t standard;

    if (device) {
        LOG_INF("%s: Flash '%s'", __func__, device->name);
    }
    else {
        LOG_ERR("%s: Flash device not found", __func__);
    }

    line_end = app_uicr_get_line_end();

    if (line_end == __UNINITIALIZED__ || line_end == INVALID_LINE_END) {
        app_uicr_set_line_end(NEWLINE);               // set default
        line_end = app_uicr_get_line_end();
    }        

    standard = app_uicr_get_standard();

    if (standard == __UNINITIALIZED__ || standard == INVALID_STANDARD) {
        app_uicr_set_standard(INCLUDE);               // set default
        standard = app_uicr_get_standard();
    }

    LOG_INF("[LINE_END] 0x%x", line_end);
    LOG_INF("[STANDARD] 0x%x", standard);
}
