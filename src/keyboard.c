/*
 *  keyboard.c  -- HOG keyboard interface
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "keyboard.h"
#include "ascii2hid.h"
#include "ble_base.h"
#include "caliper.h"

#define LOG_LEVEL 3
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(keyboard);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
enum {
    HIDS_REMOTE_WAKE = BIT(0),
    HIDS_NORMALLY_CONNECTABLE = BIT(1),
};

struct hids_info {
    uint16_t version; /* version number of base USB HID Specification */
    uint8_t code;     /* country HID Device hardware is localized for.*/
    uint8_t flags;
} __packed;

struct hids_report {
    uint8_t id;   /* report id   */
    uint8_t type; /* report type */
} __packed;

static struct hids_info info = {
    .version = 0x0000,
    .code = 0x00,
    .flags = HIDS_NORMALLY_CONNECTABLE,
};

enum {
    HIDS_INPUT = 0x01,
    HIDS_OUTPUT = 0x02,
    HIDS_FEATURE = 0x03,
};

static struct hids_report input = {
    .id = 0x01,
    .type = HIDS_INPUT,
};

static uint8_t simulate_input;
static uint8_t ctrl_point;

/*
 *   User predefined keyboard mapping
 */
#define INPUT_REP_KEYS_REF_ID            1
#define OUTPUT_REP_KEYS_REF_ID           1

static const uint8_t report_map[] = {
    0x05, 0x01,       /* Usage Page (Generic Desktop) */
    0x09, 0x06,       /* Usage (Keyboard) */
    0xA1, 0x01,       /* Collection (Application) */

    /* Keys */
#if INPUT_REP_KEYS_REF_ID
    0x85, INPUT_REP_KEYS_REF_ID,
#endif
    0x05, 0x07,       /* Usage Page (Key Codes) */
    0x19, 0xe0,       /* Usage Minimum (224) */
    0x29, 0xe7,       /* Usage Maximum (231) */
    0x15, 0x00,       /* Logical Minimum (0) */
    0x25, 0x01,       /* Logical Maximum (1) */
    0x75, 0x01,       /* Report Size (1) */
    0x95, 0x08,       /* Report Count (8) */
    0x81, 0x02,       /* Input (Data, Variable, Absolute) */

    0x95, 0x01,       /* Report Count (1) */
    0x75, 0x08,       /* Report Size (8) */
    0x81, 0x01,       /* Input (Constant) reserved byte(1) */

    0x95, 0x06,       /* Report Count (6) */
    0x75, 0x08,       /* Report Size (8) */
    0x15, 0x00,       /* Logical Minimum (0) */
    0x25, 0x65,       /* Logical Maximum (101) */
    0x05, 0x07,       /* Usage Page (Key codes) */
    0x19, 0x00,       /* Usage Minimum (0) */
    0x29, 0x65,       /* Usage Maximum (101) */
    0x81, 0x00,       /* Input (Data, Array) Key array(6 bytes) */

    /* LED */
#if OUTPUT_REP_KEYS_REF_ID
    0x85, OUTPUT_REP_KEYS_REF_ID,
#endif
    0x95, 0x05,       /* Report Count (5) */
    0x75, 0x01,       /* Report Size (1) */
    0x05, 0x08,       /* Usage Page (Page# for LEDs) */
    0x19, 0x01,       /* Usage Minimum (1) */
    0x29, 0x05,       /* Usage Maximum (5) */
    0x91, 0x02,       /* Output (Data, Variable, Absolute), */
    /* Led report */
    0x95, 0x01,       /* Report Count (1) */
    0x75, 0x03,       /* Report Size (3) */
    0x91, 0x01,       /* Output (Data, Variable, Absolute), */
    /* Led report padding */

    0xC0              /* End Collection (Application) */
  };

/* 
 *  String descriptor
 */
typedef struct {
    char * string;
    int    length;
    int    index;
} string_desc_t;

static void notify_callback(struct bt_conn * conn, void *user_data);
static void keyboard_send_char(string_desc_t * string_desc);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static ssize_t read_info(struct bt_conn *conn,
              const struct bt_gatt_attr *attr, void *buf,
              uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,
                 sizeof(struct hids_info));
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static ssize_t read_report_map(struct bt_conn *conn,
                   const struct bt_gatt_attr *attr, void *buf,
                   uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, report_map,
                 sizeof(report_map));
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static ssize_t read_report(struct bt_conn *conn,
               const struct bt_gatt_attr *attr, void *buf,
               uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,
                 sizeof(struct hids_report));
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void input_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    simulate_input = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static ssize_t read_input_report(struct bt_conn *conn,
                 const struct bt_gatt_attr *attr, void *buf,
                 uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, NULL, 0);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static ssize_t write_ctrl_point(struct bt_conn *conn,
                const struct bt_gatt_attr *attr,
                const void *buf, uint16_t len, uint16_t offset,
                uint8_t flags)
{
    uint8_t *value = attr->user_data;

    if (offset + len > sizeof(ctrl_point)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value + offset, buf, len);

    return len;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
#if CONFIG_SAMPLE_BT_USE_AUTHENTICATION
/* Require encryption using authenticated link-key. */
#define SAMPLE_BT_PERM_READ BT_GATT_PERM_READ_AUTHEN
#define SAMPLE_BT_PERM_WRITE BT_GATT_PERM_WRITE_AUTHEN
#else
/* Require encryption. */
#define SAMPLE_BT_PERM_READ BT_GATT_PERM_READ_ENCRYPT
#define SAMPLE_BT_PERM_WRITE BT_GATT_PERM_WRITE_ENCRYPT
#endif

/* HID Service Declaration */
BT_GATT_SERVICE_DEFINE(hog_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_HIDS),
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_INFO,
                           BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ, 
                           read_info, NULL, &info),
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT_MAP,
                           BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ, 
                           read_report_map, NULL, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           SAMPLE_BT_PERM_READ,
                           read_input_report, NULL, NULL),
    BT_GATT_CCC(input_ccc_changed,
                           SAMPLE_BT_PERM_READ | SAMPLE_BT_PERM_WRITE),
    BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, 
                           BT_GATT_PERM_READ,
                           read_report, NULL, &input),
    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_CTRL_POINT,
                           BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_WRITE,
                           NULL, write_ctrl_point, &ctrl_point),
);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static uint8_t report[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static struct bt_gatt_notify_params params = {
    .attr = &hog_svc.attrs[5],
    .data = report,
    .len  = sizeof(report),
    .func = notify_callback,
//  .user_data = &string_desc,  // dynamically set
};

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void notify_callback(struct bt_conn * conn, void * user_data)
{
    string_desc_t * string_desc = user_data;

#if 0
    LOG_INF("string_info: %c, %d, %d", 
             (string_desc->string[string_desc->index] >= 32) ? 
              string_desc->string[string_desc->index] : '.',
             string_desc->length,
             string_desc->index);
#endif

    string_desc->index++;

    if (string_desc->string[string_desc->index] == 0)
        return;

    if (string_desc->index <= string_desc->length) {
        keyboard_send_char(string_desc);
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void keyboard_send_char(string_desc_t * string_desc)
{
    int ret;
    int keycode;

    /*
     *  Encode Report with Key code
     */
    keycode = ascii_to_hid(string_desc->string[string_desc->index]);

    if (keycode == -1) {
        LOG_WRN("bad char in string: 0x%02X", 
                string_desc->string[string_desc->index]);
        return;
    }

#if 0
    LOG_INF("send keycode 0x%02X -- '%c'", keycode, 
        (string_desc->string[string_desc->index] >= 32) ? 
            string_desc->string[string_desc->index] : '.');
#endif

    if (needs_shift(string_desc->string[string_desc->index])) {
        report[0] |= HID_KBD_MODIFIER_RIGHT_SHIFT;
    }
    report[2] = keycode;

    params.user_data = string_desc;

    /*
     *  Send Key Press
     */
    ret = bt_gatt_notify_cb(NULL, &params);
    if (ret) {
        LOG_WRN("bt_gatt_notify_cb: ret(%d)", ret);
    }

    /*
     *  Send Key Release
     */
    memset(&report, 0, sizeof(report));

    bt_gatt_notify(NULL, &hog_svc.attrs[5], report, sizeof(report));
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void keyboard_send_string(char * string)
{
    if (is_bt_connected()) {

      static string_desc_t string_desc = {NULL, 0, 0};
      
      string_desc.string = string;
      string_desc.length = strlen(string);
      string_desc.index  = 0; 
      
      keyboard_send_char( &string_desc );
    }
}
