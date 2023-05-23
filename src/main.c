/* 
 *  main.c - Application main entry point 
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "events.h"
#include "watchdog.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, 3);

#define STACKSIZE 1024
#define PRIORITY 7

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void main_thread(void * id, void * unused1, void * unused2)
{
    LOG_INF("%s", __func__);

    buttons_init();

    caliper_init();

    events_init();

    watchdog_init();

    while (1) {
        k_sleep(K_SECONDS(60));
    }
}


K_THREAD_DEFINE(main_id, STACKSIZE, main_thread,
                NULL, NULL, NULL, PRIORITY, 0, 0);
