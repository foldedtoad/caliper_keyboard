/*
 *   framer.h
 */
#ifndef __FRAMER_H
#define __FRAMER_H

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define CALIPER_POWER_OFF       false
#define CALIPER_POWER_ON        true

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void framer_init(void);
void framer_find_interframe_gap(void);
bool is_caliper_on(void);

/*---------------------------------------------------------------------------*/
/* Used with logic analyzer for debugging purposes.                          */
/*---------------------------------------------------------------------------*/

//#define logic_analyzer_testing 1

#if logic_analyzer_testing
void framerPulseDebug(void);
#endif

#endif  /* __FRAMER_H */
