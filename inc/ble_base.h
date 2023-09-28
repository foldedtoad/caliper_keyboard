/*
 *  ble_base.h
 */
#ifndef __BLE_BASE_H__
#define __BLE_BASE_H__

#include <stdbool.h>

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
typedef void (*ble_connected_callback_t)(bool connected);

void ble_register_connect_handler(ble_connected_callback_t callback);
void ble_unregister_notify_handler(void);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
int ble_base_init(void);
bool is_bt_connected(void);

#endif  // __BLE_BASE_H__