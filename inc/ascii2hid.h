/*
 *  ascii2hid.h
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 */
#ifndef ASCII2HID_H
#define ASCII2HID_H

#include <stdint.h>
#include <stdbool.h>

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

int ascii_to_hid(uint8_t ascii);
bool needs_shift(uint8_t ascii);

#endif /* ASCII2HID_H */