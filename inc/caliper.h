/*
 *   caliper.h
 */
#ifndef __CALIPER_H
#define __CALIPER_H

/*---------------------------------------------------------------------------*/
/* Caliper Standards values                                                  */
/*---------------------------------------------------------------------------*/

#define CALIPER_STANDARD_INCH      0x00
#define CALIPER_STANDARD_MM        0x01

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define CALIPER_POWER_OFF       false
#define CALIPER_POWER_ON        true

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void caliper_init(void);
bool is_caliper_on(void);
int  caliper_read_value(short * value, int * standard);

#endif  /* __CALIPER_H */
