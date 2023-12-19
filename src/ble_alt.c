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

#include "ble_alt.h"
#include "nus.h" 
#include "shell_bt_nus.h" 

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ble_alt, LOG_LEVEL_INF);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define DEVICE_NAME     ALTERNATE_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data advert[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data scan[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

static struct bt_conn * current_conn;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection failed (err %u)", err);
        return;
    }

    LOG_INF("Connected");
    current_conn = bt_conn_ref(conn);
    shell_bt_nus_enable(conn);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason %u)", reason);

    shell_bt_nus_disable();
    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static char *log_addr(struct bt_conn *conn)
{
    static char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    return addr;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void __attribute__((unused)) security_changed(struct bt_conn *conn,
                             bt_security_t level,
                             enum bt_security_err err)
{
    char *addr = log_addr(conn);

    if (!err) {
        LOG_INF("Security changed: %s level %u", addr, level);
    } else {
        LOG_INF("Security failed: %s level %u err %d", addr, level,
            err);
    }
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected    = connected,
    .disconnected = disconnected,
    COND_CODE_1(CONFIG_BT_SMP,
            (.security_changed = security_changed), ())
};

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    LOG_INF("Passkey for %s: %06u", log_addr(conn), passkey);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void auth_cancel(struct bt_conn *conn)
{
    LOG_INF("Pairing cancelled: %s", log_addr(conn));
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void pairing_complete(struct bt_conn *conn, bool bonded)
{
    LOG_INF("Pairing completed: %s, bonded: %d", log_addr(conn), bonded);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
    LOG_INF("Pairing failed conn: %s, reason %d", log_addr(conn), reason);
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

static struct bt_conn_auth_cb conn_auth_callbacks = {
    .passkey_display = auth_passkey_display,
    .cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed
};

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
int ble_alt_init(void)
{
    int err;

    LOG_INF("Starting Bluetooth ALT");

    if (IS_ENABLED(CONFIG_BT_SMP)) {
        err = bt_conn_auth_cb_register(&conn_auth_callbacks);
        if (err) {
            LOG_INF("Failed to register authorization callbacks.");
            return 0;
        }

        err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
        if (err) {
            LOG_INF("Failed to register authorization info callbacks.");
            return 0;
        }
    }

    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("BLE ALT enable failed (err: %d)", err);
        return 0;
    } 

    err = shell_bt_nus_init();
    if (err) {
        LOG_ERR("Failed to initialize BT NUS shell (err: %d)", err);
        return 0;
    }

    err = bt_le_adv_start(BT_LE_ADV_CONN, 
                          advert, ARRAY_SIZE(advert), 
                          scan,   ARRAY_SIZE(scan));
    if (err) {
        LOG_ERR("ALT advertising start failed: %d", err);
        return 0;
    }

    LOG_INF("Bluetooth ALT ready. Advertising started.");

    return 0;
}
