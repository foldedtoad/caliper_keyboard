/*
 *   buttons.h
 */
#ifndef __BUTTONS_H
#define __BUTTONS_H

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

 #define NO_PRESS   (-1)
 #define INVALID_ID (-1)

 typedef enum {
     BTN1_ID  = 1,
 //    BTN2_ID  = 2,
 //    BTN3_ID  = 3,
 //    BTN4_ID  = 4,
 } buttons_id_t;

typedef enum {
	BOOT_OPTIONS_NORMAL     = 0,
	BOOT_OPTIONS_ALTERNATE  = 1,
} boot_options_t;

typedef void (*buttons_notify_t)(buttons_id_t id);

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void buttons_init(void);
void buttons_register_notify_handler(buttons_notify_t notify);
void buttons_unregister_notify_handler(void);
void buttons_remote_button(void);
boot_options_t boot_button_state(void);

#endif  /* __BUTTONS_H */