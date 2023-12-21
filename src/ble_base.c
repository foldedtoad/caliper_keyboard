 /*
 * Copyright (c) 2023 Callender-Consulting
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *  ble_base.c  -- Bluetooth base functions
 */
#include <stddef.h>
#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/types.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h> 
#include <zephyr/bluetooth/services/dis.h> 

#include "ble_base.h"
#include "keyboard.h"
#include "main.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ble_base, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define STACKSIZE 1024
#define PRIORITY 7

#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static ble_connected_callback_t ble_connected_callback = NULL;

static const char * levels[] = {
    "L0",
    "L1",
    "L2",
    "L3",
    "L4",
};

static const char * errors[] = {
    "SUCCESS",
    "AUTH_FAIL",
    "PIN_OR_KEY_MISSING",
    "OOB_NOT_AVAILABLE",
    "AUTH_REQUIREMENT",
    "PAIR_NOT_SUPPORTED",
    "PAIR_NOT_ALLOWED",
    "INVALID_PARAM",
    "KEY_REJECTED",
    "UNSPECIFIED",
};

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static const struct bt_data advert[] = {
    BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE,
                  (CONFIG_BT_DEVICE_APPEARANCE >> 0) & 0xff,
                  (CONFIG_BT_DEVICE_APPEARANCE >> 8) & 0xff),
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
              BT_UUID_16_ENCODE(BT_UUID_HIDS_VAL),
              BT_UUID_16_ENCODE(BT_UUID_BAS_VAL)),
};

static const struct bt_data scand[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_le_adv_param *advert_param = 
    BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME,
                    BT_GAP_ADV_FAST_INT_MIN_2,
                    BT_GAP_ADV_FAST_INT_MAX_2,
                    NULL);

static bool bt_connected = false;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
bool is_bt_connected(void)
{
    //LOG_INF("bt_connected %s", bt_connected ? "true" : "false");
    return bt_connected;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void start_advertising(void)
{
    int err;

    err = bt_le_adv_start(advert_param, 
                          advert, ARRAY_SIZE(advert), 
                          scand, ARRAY_SIZE(scand));
    if (err) {
        if (err == -EALREADY) {
            LOG_INF("Advertising continued");
        } 
        else {
            LOG_ERR("Start advertising failed: %d", err);
        }
        return;
    }

    LOG_INF("Advertising started");
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    if (is_alt_running()) return;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err) {
        LOG_ERR("Failed to connect to %s (%u)", addr, err);
        return;
    }

    LOG_INF("Connected %s", addr);

    int ret = bt_conn_set_security(conn, BT_SECURITY_L2);
    if (ret) {
        LOG_ERR("Failed to set security: %d", ret);
    }

    /* Create connected event */
    if (ble_connected_callback) {
        ble_connected_callback(true);
    }

    bt_connected = true;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    if (is_alt_running()) return;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Disconnected from %s, reason %d", addr, reason);

    /* Create disconnected event */
    if (ble_connected_callback) {
        ble_connected_callback(false);
    }

    bt_connected = false;

    start_advertising();
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void security_changed(struct bt_conn *conn, bt_security_t level,
                 enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    if (is_alt_running()) return;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (!err) {
        LOG_INF("Security changed: %s, level %s", addr, levels[level]);
    }
    else {
        LOG_ERR("Security failed: %s, level %s, err (%d) %s", 
                 addr, levels[level], err, errors[err]);
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
    char addr[BT_ADDR_LE_STR_LEN];

    if (is_alt_running()) return false;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("LE conn param req: %s int (0x%04x, 0x%04x) lat %d to %d",
            addr, param->interval_min, param->interval_max, param->latency,
            param->timeout);

    return true;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void le_param_updated(struct bt_conn *conn, uint16_t interval,
                             uint16_t latency, uint16_t timeout)
{
    char addr[BT_ADDR_LE_STR_LEN];

    if (is_alt_running()) return;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("LE conn param updated: %s int 0x%04x lat %d to %d",
            addr, interval, latency, timeout);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected        = connected,
    .disconnected     = disconnected,
    .le_param_req     = le_param_req,
    .le_param_updated = le_param_updated,
    .security_changed = security_changed,
};

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void bt_ready(int err)
{
    if (err) {
        LOG_ERR("Bluetooth init failed: %d", err);
        return;
    }

    LOG_INF("Bluetooth initialized");

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    start_advertising();
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_WRN("Pairing cancelled: %s", addr);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void auth_pairing_confirm(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    bt_conn_auth_pairing_confirm(conn);

    LOG_INF("Pairing confirm: %s", addr);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static struct bt_conn_auth_cb auth_cb_display = {
    .pairing_confirm = auth_pairing_confirm,
    .passkey_entry   = NULL,
    .cancel          = auth_cancel,   
};

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void ble_register_connect_handler(ble_connected_callback_t callback)
{
    ble_connected_callback = callback;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void ble_unregister_notify_handler(void)
{
    ble_connected_callback = NULL;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
int ble_base_init(void)
{
    int err;

    LOG_INF("%s", __func__);

    bt_connected = false;

    err = bt_enable(bt_ready);
    if (err) {
        LOG_ERR("Bluetooth not ready: %d", err);
        return err;
    }

    bt_conn_auth_cb_register(&auth_cb_display); 

    return 0; 
}
