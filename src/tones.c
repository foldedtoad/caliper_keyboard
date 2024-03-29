/*
 *  tones.c  -- predefined tones
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <inttypes.h>
#include <stdint.h>

#include "buzzer.h"

/*---------------------------------------------------------------------------*/
/* Collection of sounds and tones                                            */
/*---------------------------------------------------------------------------*/

buzzer_play_t startup_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz   200ms
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=700},   // long buzz    700ms
    {.action = BUZZER_PLAY_DONE,  .duration=0},     // stop
};

buzzer_play_t send_completed_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200},  // long buzz   200ms      
    {.action = BUZZER_PLAY_DONE,  .duration=0},    // stop
};

buzzer_play_t caliper_off_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=500},  // long buzz   500ms
    {.action = BUZZER_PLAY_QUIET, .duration=200},  // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=1000}, // long buzz   1000ms        
    {.action = BUZZER_PLAY_DONE,  .duration=0},    // stop
};

buzzer_play_t ble_not_connected_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=500},  // long buzz   500ms
    {.action = BUZZER_PLAY_QUIET, .duration=200},  // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=500},  // long buzz   500ms
    {.action = BUZZER_PLAY_QUIET, .duration=200},  // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=1000}, // long buzz   1000ms     
    {.action = BUZZER_PLAY_DONE,  .duration=0},    // stop
};

buzzer_play_t error_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz   200ms
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz
    {.action = BUZZER_PLAY_QUIET, .duration=200},   // short quiet
    {.action = BUZZER_PLAY_DONE,  .duration=0},     // stop
};
