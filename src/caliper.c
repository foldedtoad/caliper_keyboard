 /*
 * Copyright (c) 2023 Callender-Consulting
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *  caliper.c  -- Caliper eventing and decode functions
 */ 
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdlib.h>

#include "caliper.h"
#include "caliper_gpio.h"
#include "keyboard.h"
#include "buttons.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(caliper, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static const struct gpio_dt_spec clock_spec = 
                        GPIO_DT_SPEC_GET_OR(CLOCK_NODE, gpios, 0);

static const struct gpio_dt_spec data_spec = 
                        GPIO_DT_SPEC_GET_OR(DATA_NODE, gpios, 0);

#if 0
static const struct gpio_dt_spec debug_spec = 
                        GPIO_DT_SPEC_GET_OR(DEBUG_NODE, gpios, 0});
#endif

static struct gpio_callback clock_irq_cb_data;

static void caliperBackend(void * unused);

#define CALIPER_THREAD_STACK_SIZE  1024
// Cooperative thread priority 
#define CALIPER_THREAD_PRIORITY    -5

K_THREAD_DEFINE(caliper_thread, 
                CALIPER_THREAD_STACK_SIZE, 
                caliperBackend, 
                NULL, NULL, NULL, 
                CALIPER_THREAD_PRIORITY, 0, 0);

K_SEM_DEFINE(caliper_gpio_sem, 0, 1);

K_SEM_DEFINE(caliper_read_done_sem, 0, 1);

void caliper_inactive_callback(struct k_timer * timer);

K_TIMER_DEFINE(caliper_inactive_timer, caliper_inactive_callback, NULL);


/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define CALIPER_FRAME_VALUE_BITS  16
#define CALIPER_FRAME_FLAG_BITS   20
#define CALIPER_FRAME_TOTAL_BITS  24

#define CALIPER_FRAME_FLAG_SIGN_BIT 21

#define HIGH        1
#define LOW         0

static bool caliper_power_state = CALIPER_POWER_OFF;

static int   current_frame;
static short current_value;
static int   current_standard;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static int caliperRead(const struct gpio_dt_spec * spec)
{
    return gpio_pin_get_dt(spec);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void caliperWrite(const struct gpio_dt_spec * spec, int val)
{
    gpio_pin_set_dt(spec, val);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void caliperBackend(void * unused)
{
    int i;
    int sign;
    int bit;
    int lockkey;

    LOG_INF("%s: waiting for work...", __func__);

    while (1) {

        k_sem_take(&caliper_gpio_sem, K_FOREVER);

        i = 0;
        sign = 1;

        current_frame = 0;
        current_value = 0;
        current_standard = 0;

        caliper_power_state = CALIPER_POWER_ON;

        /*
         *  Save initial data value caused by interrupt
         */
        if (caliperRead(&data_spec) == 0) {
            current_value |= 1 << i;
            current_frame |= 1 << i;
        }

        for (i=1; i < (CALIPER_FRAME_TOTAL_BITS); i++) {

            /*  Disable all interrupts while reading next data bit. */
            lockkey = irq_lock();

            while (caliperRead(&clock_spec) == LOW)  { /*spin*/}
            while (caliperRead(&clock_spec) == HIGH) { /*spin*/}

            bit = caliperRead(&data_spec);

            /*  Re-enable all interrupts */
            irq_unlock(lockkey);

            if (bit == HIGH) {
                current_frame |= 1 << i;

                if (i < CALIPER_FRAME_VALUE_BITS) {
                    current_value |= 1 << i;
                }

                if (i == CALIPER_FRAME_FLAG_SIGN_BIT) {
                    sign = -1;
                }
            }
            else {
                current_frame &= ~(1 << i);
            }           
        }

        /*
         *   Delay 280us, then do final data read for mode.
         *   This catches the odd, out-of-frame mode flag after the frame.
         *   For mm-mode, the data read will be high at end of this wait.
         *   For in-mode, the data read will be low at end of this wait.
         *   The 280us was derived from logic analyzer traces (emperically).
         */
        k_busy_wait(280);   // hard spin, not a true wait.

        bit = caliperRead(&data_spec);
        if (bit == HIGH) {
            current_standard = CALIPER_STANDARD_INCH;
        }
        else {
            current_standard = CALIPER_STANDARD_MM;
        }

        current_value = current_value / 2;
        if (current_standard == CALIPER_STANDARD_INCH) 
            current_value /= 2;
        current_value *= sign;

        k_sem_give(&caliper_read_done_sem);

        gpio_pin_interrupt_configure_dt(&clock_spec, GPIO_INT_DISABLE);
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
int caliper_read_value(short * value, int * standard)
{
    LOG_DBG("%s ", __func__);

    /*
     *  Start inactive timer to determine if calipers are powered off.
     */
    k_timer_start(&caliper_inactive_timer, K_MSEC(500), K_NO_WAIT);

    /* 
     *  Set interrupts on falling edge (HIGH --> LOW)
     */
    gpio_pin_interrupt_configure_dt(&clock_spec, GPIO_INT_EDGE_FALLING);

    /*
     *  Wait for value-read to complete
     */
    k_sem_take(&caliper_read_done_sem, K_FOREVER);

    *value    = current_value;
    *standard = current_standard;

    return 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
bool is_caliper_on(void)
{
    LOG_DBG("%s: %s", __func__, 
           (caliper_power_state == CALIPER_POWER_ON) ? "ON" : "OFF");

    return caliper_power_state;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void caliper_inactive_callback(struct k_timer * timer)
{
    LOG_INF("Caliper is \"OFF\"");

    caliper_power_state = CALIPER_POWER_OFF;

    k_timer_stop(&caliper_inactive_timer);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void caliperInterrupt(const struct device * dev, 
                      struct gpio_callback * cb,
                      uint32_t bitarray)
{
    if (bitarray & (1 << CLOCK)) {
        /*
         *  Disable interrupts, then signal backend thread to do work.
         */
        gpio_pin_interrupt_configure_dt(&clock_spec, GPIO_INT_DISABLE);

        k_timer_stop(&caliper_inactive_timer);

        k_sem_give(&caliper_gpio_sem);
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void caliperInitInterrupts(void)
{
    int ret;

    if (!device_is_ready(clock_spec.port)) {
        LOG_ERR("Error: Caliper %s is not ready", clock_spec.port->name);
        return;
    }

    ret = gpio_pin_configure_dt(&clock_spec, (GPIO_PULL_DOWN | GPIO_INPUT));
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d",
               ret, clock_spec.port->name, clock_spec.pin);
        return;
    }

    /* 
     *  Set interrupts on rising edge (Low --> High)
     */
    ret = gpio_pin_interrupt_configure_dt(&clock_spec, GPIO_INT_DISABLE);
    if (ret != 0) {
        LOG_ERR("%s failed %d", __func__, ret);
        return;
    }

    gpio_init_callback(&clock_irq_cb_data, caliperInterrupt, BIT(clock_spec.pin));
    gpio_add_callback(clock_spec.port, &clock_irq_cb_data);

    LOG_INF("Configure interrupts for %s on pin %d", CLOCK_LABEL, CLOCK);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void caliper_init(void)
{
    (void) caliperRead;
    (void) caliperWrite;

    LOG_INF("%s", __func__);

    /*
     *  Initialize interrupts on CLOCK pin
     */
    caliperInitInterrupts();

    /*
     *  Initialize DATA pin
     */
    gpio_pin_configure_dt(&data_spec, (GPIO_PULL_DOWN | GPIO_INPUT));

#if 0
    /*
     *  optional: Initialize debug pin: output to logic analyzer
     */
    gpio_pin_configure_dt(&debug_spec, (GPIO_PULL_DOWN | GPIO_OUTPUT));
#endif

    caliper_power_state = CALIPER_POWER_ON;
}
