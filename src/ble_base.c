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

#include "keyboard.h" 

#define LOG_LEVEL 3 //CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ble_base);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define STACKSIZE 1024
#define PRIORITY 7

#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

K_SEM_DEFINE(ble_base_sem, 0, 1);

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

    bt_connected = true;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    LOG_INF("Disconnected from %s, reason %d", addr, reason);

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
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected        = connected,
    .disconnected     = disconnected,
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
static void bas_notify(void)
{
    uint8_t battery_level = bt_bas_get_battery_level();

    battery_level--;

    if (battery_level == 50) {
        battery_level = 100U;
    }

    bt_bas_set_battery_level(battery_level);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void ble_base_thread(void * id, void * unused1, void * unused2)
{
    int err;

    LOG_INF("%s starting...", __func__);

    bt_connected = false;

    err = bt_enable(bt_ready);
    if (err) {
        LOG_ERR("Bluetooth not ready: %d", err);
        return;
    }

    bt_conn_auth_cb_register(&auth_cb_display); 

    /* Battery level */
    while (1) {
        k_sleep(K_SECONDS(30));
        bas_notify();
    }    
}

/*---------------------------------------------------------------------------*/
/*  Define working thread for Bluetooth Broadcaster to run on                */
/*---------------------------------------------------------------------------*/
K_THREAD_DEFINE(ble_base_id, STACKSIZE, ble_base_thread, 
                NULL, NULL, NULL, PRIORITY, 0, 0);
