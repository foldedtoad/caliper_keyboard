/*
 *   battery.h
 */
#ifndef __BATTERY_H
#define __BATTERY_H

/* Set invalid (uninitialized) percentage value to something very low */
#define BATTERY_LEVEL_INVALID ((uint8_t)5)

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
int  battery_init(void);
void battery_start(void);
void battery_stop(void);
void battery_ble_events(bool connected);

#endif  /* __BATTERY_H */