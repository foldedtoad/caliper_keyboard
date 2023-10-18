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
void caliper_init(void);
int  caliper_read_value(short * value, int * standard);

#endif  /* __CALIPER_H */
