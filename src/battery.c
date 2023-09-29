/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/types.h>

#include <soc.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h> 
#include <zephyr/sys/atomic.h>
#include <hal/nrf_saadc.h>
#include <zephyr/dt-bindings/adc/nrf-adc.h>
#include <zephyr/bluetooth/services/bas.h>  

#include "ble_base.h"
#include "battery_def.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(battery, 3);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define ADC_NODE             DT_NODELABEL(adc)
#define ADC_RESOLUTION       12
#define ADC_OVERSAMPLING     4 /* 2^ADC_OVERSAMPLING samples are averaged */
#define ADC_MAX              4096
#define ADC_GAIN             BATTERY_MEAS_ADC_GAIN
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_REF_INTERNAL_MV  600UL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)
#define ADC_CHANNEL_ID       0
#define ADC_CHANNEL_INPUT    BATTERY_MEAS_ADC_INPUT

#define BATTERY_WORK_INTERVAL   (BATTERY_MEAS_POLL_INTERVAL_MS / 2)

#define BATTERY_VOLTAGE(sample) (sample * BATTERY_MEAS_VOLTAGE_GAIN * \
                                 ADC_REF_INTERNAL_MV / ADC_MAX)

static const struct device * const adc_dev = DEVICE_DT_GET_OR_NULL(ADC_NODE);

static int16_t adc_buffer;

static bool    adc_async_read_pending;

static struct k_work_delayable battery_lvl_read;

static struct k_poll_signal async_sig = K_POLL_SIGNAL_INITIALIZER(async_sig);

static struct k_poll_event  async_evt =
                               K_POLL_EVENT_INITIALIZER(K_POLL_TYPE_SIGNAL,
                               K_POLL_MODE_NOTIFY_ONLY,
                               &async_sig);

#define STACKSIZE 1024
#define PRIORITY 7

static atomic_t active;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int init_adc(void)
{
    LOG_INF("%s", __func__);

    if (!device_is_ready(adc_dev)) {
        LOG_ERR("ADC device not ready");
        return -ENODEV;
    }
    LOG_INF("ADC '%s'", adc_dev->name);

    static const struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN,
        .reference        = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id       = ADC_CHANNEL_ID,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
        .input_positive   = ADC_CHANNEL_INPUT,
#endif
    };

    int err = adc_channel_setup(adc_dev, &channel_cfg);
    if (err) {
        LOG_ERR("Setting up the ADC channel failed");
        return err;
    }

    /* Check if number of elements in LUT is proper */

    size_t levels = (BATTERY_MEAS_MAX_LEVEL - BATTERY_MEAS_MIN_LEVEL);
    size_t steps  = ((ARRAY_SIZE(battery_voltage_to_soc)-1) * VOLTAGE_TO_SOC_DELTA);

    LOG_INF("levels: %d, steps: %d", levels, steps);

    if ((BATTERY_MEAS_MAX_LEVEL - BATTERY_MEAS_MIN_LEVEL) ==
        ((ARRAY_SIZE(battery_voltage_to_soc) - 1) * VOLTAGE_TO_SOC_DELTA)) {
        LOG_ERR("Improper number of elements in lookup table");
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void battery_start(void)
{
    if (atomic_get(&active) == 1) {
        return;
    }

    atomic_set(&active, 1);
    LOG_INF("%s", __func__);
    k_work_reschedule(&battery_lvl_read, K_MSEC(BATTERY_WORK_INTERVAL));
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void battery_stop(void)
{
    if (atomic_get(&active) == 0) {
        return;
    }

    LOG_INF("%s", __func__);
    k_work_cancel_delayable(&battery_lvl_read);
    atomic_set(&active, 0);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void battery_lvl_process(void)
{
    //LOG_INF("%s", __func__);

    uint32_t milliVolts = BATTERY_VOLTAGE(adc_buffer);
    uint8_t  percentage;
    size_t   lut_id = -1;

    if (milliVolts > BATTERY_MEAS_MAX_LEVEL) {
        percentage = 100;
        LOG_WRN("Battery high");
    }
    else if (milliVolts < BATTERY_MEAS_MIN_LEVEL) {
        percentage = 0;
        LOG_WRN("Battery low");
    }
    else {
        /* Using lookup table to convert voltage[mV] to SoC[%] */
        lut_id = (milliVolts - BATTERY_MEAS_MIN_LEVEL +
                 (VOLTAGE_TO_SOC_DELTA >> 1)) / VOLTAGE_TO_SOC_DELTA;

        percentage = battery_voltage_to_soc[lut_id];
    }

    if (is_bt_connected()) {
#if 1        
        LOG_INF("Battery level: %u%%", percentage);
#else
        LOG_INF("Battery level: %u%% (%u mV) [%d], raw(%u)", 
             percentage, milliVolts, lut_id, adc_buffer);
#endif        
        bt_bas_set_battery_level(percentage);
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void battery_lvl_read_fn(struct k_work * work)
{
    int err;

    //LOG_INF("%s", __func__);

    if (!adc_async_read_pending) {
        static const struct adc_sequence sequence = {
            .options      = NULL,
            .channels     = BIT(ADC_CHANNEL_ID),
            .buffer       = &adc_buffer,
            .buffer_size  = sizeof(adc_buffer),
            .resolution   = ADC_RESOLUTION,
            .oversampling = ADC_OVERSAMPLING,
            .calibrate    = false,
        };
        static const struct adc_sequence sequence_calibrate = {
            .options      = NULL,
            .channels     = BIT(ADC_CHANNEL_ID),
            .buffer       = &adc_buffer,
            .buffer_size  = sizeof(adc_buffer),
            .resolution   = ADC_RESOLUTION,
            .oversampling = ADC_OVERSAMPLING,
            .calibrate    = true,
        };

        static bool calibrated;

        if (likely(calibrated)) {
            err = adc_read_async(adc_dev, &sequence, &async_sig);
            if (err) {
                LOG_ERR("adc_read_async failed: %d", err);
            } 
            else {
                adc_async_read_pending = true;
            }
        } 
        else {
            err = adc_read_async(adc_dev, &sequence_calibrate, &async_sig);
            calibrated = true;
            if (err) {
                LOG_ERR("adc_read_async calibrate failed: %d", err);
            }
            else {
                adc_async_read_pending = true;
            }
        }
    }
    else {
        err = k_poll(&async_evt, 1, K_NO_WAIT);
        if (err) {
            LOG_WRN("Battery level poll failed");
        }
        else {
            adc_async_read_pending = false;
            battery_lvl_process();
        }
    }

    if (atomic_get(&active) || adc_async_read_pending) {
        k_work_reschedule(&battery_lvl_read, K_MSEC(BATTERY_WORK_INTERVAL));
    }
    else {
        battery_stop();
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void battery_ble_events(bool connected)
{
    if (connected) {
        battery_start();
    }
    else {
        battery_stop();
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void battery_init(void)
{
    int err = 0;

    LOG_INF("%s", __func__);;

    err = init_adc();
    if (err) {
        LOG_ERR("ADC init failed");
        return;
    }

    k_work_init_delayable(&battery_lvl_read, battery_lvl_read_fn);
}
