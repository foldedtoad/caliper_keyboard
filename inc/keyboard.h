/*
 *  hog.h  -- keyboard interface
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "ascii2hid.h"
#include <zephyr/usb/class/hid.h>  /* USB and BLE HID defs are same */

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void keyboard_send_string(char * value);

#endif /* KEYBOARD_H */
