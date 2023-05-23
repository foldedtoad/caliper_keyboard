/*
 *  events.h
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 */
#ifndef EVENTS_H
#define EVENTS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "keyboard.h"
#include "buttons.h"
#include "caliper.h"
#include "ble_base.h"

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void events_init(void);

#endif /* EVENTS_H */