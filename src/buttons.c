/*
 *   buttons.c
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

#include "buttons.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(buttons, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define SW_GPIO_DEV    DT_NODELABEL(gpio0)

#define SW0_PIN        DT_GPIO_PIN(DT_ALIAS(sw0), gpios)
#define SW0_FLAGS      DT_GPIO_FLAGS(DT_ALIAS(sw0), gpios)
//#define SW1_PIN        DT_GPIO_PIN(DT_ALIAS(sw1), gpios)
//#define SW2_PIN        DT_GPIO_PIN(DT_ALIAS(sw2), gpios)
//#define SW3_PIN        DT_GPIO_PIN(DT_ALIAS(sw3), gpios)

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

typedef struct {
    uint8_t    id;
    uint8_t    pin;
    uint32_t   bit;
    char  * name;
} button_info_t; 

static const button_info_t button_info [] = {
    { .id = BTN1_ID, .pin = SW0_PIN, .bit = BIT(SW0_PIN), .name = "BTN1" },
//    { .id = BTN2_ID, .pin = SW1_PIN, .bit = BIT(SW1_PIN), .name = "BTN2" },
//    { .id = BTN3_ID, .pin = SW2_PIN, .bit = BIT(SW2_PIN), .name = "BTN3" },
//    { .id = BTN4_ID, .pin = SW3_PIN, .bit = BIT(SW3_PIN), .name = "BTN4" },
};
#define BUTTONS_COUNT (sizeof(button_info)/sizeof(button_info_t))

static const button_info_t unknown = {.id=INVALID_ID, .pin=0 , .bit= 0, .name= "???"};

#define DEBOUNCE_MS 150

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static const struct device * gpiob;

static struct gpio_callback buttons_cb;

typedef struct {
    struct k_work      work;
    button_info_t    * current;
    buttons_notify_t   notify;
} buttons_t;

static buttons_t  buttons;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static const button_info_t * button_get_info(uint32_t pins)
{
    for (int i=0; i < BUTTONS_COUNT; i++) {
        if (button_info[i].bit & pins) {
            return &button_info[i];
        }
    }
    return &unknown;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buttons_event(const struct device * gpiob,
                   struct gpio_callback * cb,
                   uint32_t pins)
{
    button_info_t * current = (button_info_t *) button_get_info(pins);
    if (current->id == INVALID_ID)
        return;
    buttons.current = current;

    k_work_submit(&buttons.work);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void buttons_worker(struct k_work * work)
{
    if (gpio_pin_get(gpiob, buttons.current->pin) == 1) {
        k_msleep(DEBOUNCE_MS);
        if (gpio_pin_get(gpiob, buttons.current->pin) == 1) {
            if (buttons.notify) {
                buttons.notify(buttons.current->id);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buttons_register_notify_handler(buttons_notify_t notify)
{
    buttons.notify = notify;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buttons_unregister_notify_handler(void)
{
    buttons.notify = NULL;
}

/*---------------------------------------------------------------------------*/
/*  Trigger remote button event -- driven by shell "snap" cmd.               */
/*---------------------------------------------------------------------------*/
void buttons_remote_button(void)
{
    if (buttons.notify) {
        buttons.notify(0);   // NOTE: passed value is ignored.
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
boot_options_t boot_button_state(void)
{
    if (gpio_pin_get(gpiob, SW0_PIN) == 1) {
        return BOOT_OPTIONS_ALTERNATE;
    }
    return BOOT_OPTIONS_NORMAL;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buttons_init(void)
{
    LOG_INF("%s", __func__);

    gpiob = DEVICE_DT_GET(SW_GPIO_DEV);
    if (!gpiob) {
        LOG_ERR("device not found. %s", DEVICE_DT_NAME(SW_GPIO_DEV));
        return;
    }

    k_work_init(&buttons.work, buttons_worker);

    //LOG_INF("SW0_PIN: %d", SW0_PIN);

    /* Init Button Interrupt */
    int flags = (GPIO_INPUT       | 
                 SW0_FLAGS        |   
                 GPIO_INT_EDGE);

    gpio_pin_configure(gpiob, SW0_PIN, flags);
//    gpio_pin_configure(gpiob, SW1_PIN, flags);
//    gpio_pin_configure(gpiob, SW2_PIN, flags);
//    gpio_pin_configure(gpiob, SW3_PIN, flags);

    gpio_pin_interrupt_configure(gpiob, SW0_PIN, GPIO_INT_EDGE_TO_ACTIVE);
//    gpio_pin_interrupt_configure(gpiob, SW1_PIN, GPIO_INT_EDGE_TO_ACTIVE);
//    gpio_pin_interrupt_configure(gpiob, SW2_PIN, GPIO_INT_EDGE_TO_ACTIVE);
//    gpio_pin_interrupt_configure(gpiob, SW3_PIN, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_init_callback(&buttons_cb, buttons_event,
                       BIT(SW0_PIN) 
//                       | BIT(SW1_PIN) 
//                       | BIT(SW2_PIN) 
//                       | BIT(SW3_PIN)
                      );

    gpio_add_callback(gpiob, &buttons_cb);

}
