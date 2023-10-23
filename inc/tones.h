/*
 *  tpmes.h  -- predefined tones
 *
 *  Copyright (c) 2023   Callender-Consulting
 *
 *  SPDX-License-Identifier: Apache-2.0
 */
#ifndef __TONES_H__
#define __TONES_H__

#include "buzzer.h"

/*---------------------------------------------------------------------------*/
/* Collection of sounds and tones                                            */
/*---------------------------------------------------------------------------*/

extern buzzer_play_t startup_sound;
extern buzzer_play_t simple_beep_sound;
extern buzzer_play_t end_of_cycle_sound;
extern buzzer_play_t error_sound;
extern buzzer_play_t find_me_sound;

#endif /* __TONES_H__ */
