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

buzzer_play_t simple_beep_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=200},   // short buzz   200ms
    {.action = BUZZER_PLAY_DONE,  .duration=0},     // stop
};


buzzer_play_t end_of_cycle_sound [] = {
    {.action = BUZZER_PLAY_TONE,  .duration=1000},  // long buzz   1s
    {.action = BUZZER_PLAY_DONE,  .duration=0},     // stop
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

buzzer_play_t find_me_sound [] = {
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
    {.action = BUZZER_PLAY_DONE,  .duration=0},     // stop
};
