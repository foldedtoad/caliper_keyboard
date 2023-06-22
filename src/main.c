/* 
 *  main.c - Application main entry point 
 */
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/hwinfo.h> 

#include "events.h"
#include "watchdog.h"
#include "shell.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, 3);

#define STACKSIZE 1024
#define PRIORITY 7

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/

typedef struct {
    uint32_t code;
    char *   text;
} reason_t; 

static reason_t reasons[] = {
    { RESET_PIN,            "Reset Pin"     },  // BIT(0)
    { RESET_SOFTWARE,       "Software"      },  // BIT(1)
    { RESET_BROWNOUT,       "Brown Out"     },  // BIT(2)
    { RESET_POR,            "POR"           },  // BIT(3)
    { RESET_WATCHDOG,       "Watchdog"      },  // BIT(4)
    { RESET_DEBUG,          "Debug"         },  // BIT(5)
    { RESET_SECURITY,       "Security"      },  // BIT(6)
    { RESET_LOW_POWER_WAKE, "Low Pwr Wake"  },  // BIT(7)
    { RESET_CPU_LOCKUP,     "CPU Lockup"    },  // BIT(8)
    { RESET_PARITY,         "Parity"        },  // BIT(9)
    { RESET_PLL,            "PLL"           },  // BIT(10)
    { RESET_CLOCK,          "Clock"         },  // BIT(11)
    { RESET_HARDWARE,       "Hardware"      },  // BIT(12)
    { RESET_USER,           "User"          },  // BIT(13)
    { RESET_TEMPERATURE,    "Temperature"   },  // BIT(14)
};
#define REASONS_COUNT (sizeof(reasons)/sizeof(reason_t))

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
static char * get_reason_text(uint32_t reset_reason)
{
    static char string[48];

    string[0] = 0;

    for(int i=0; i < REASONS_COUNT; i++) {
        if (reset_reason & reasons[i].code) {
            reset_reason &= ~reasons[i].code;
            strcat(string, reasons[i].text);
            if (reset_reason)
                strcat(string, ", ");
        }
    }
    return (char*) &string;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void main_thread(void * id, void * unused1, void * unused2)
{
    uint32_t reset_reason = 0;

    hwinfo_get_reset_cause(&reset_reason);
    hwinfo_clear_reset_cause();

    LOG_INF("*** Reset Reason: (0x%02X) %s", 
            reset_reason, get_reason_text(reset_reason));

    buttons_init();

    caliper_init();

    events_init();

    caliper_shell_init();

    watchdog_init();

    while (1) {
        k_sleep(K_SECONDS(60));
    }
}


K_THREAD_DEFINE(main_id, STACKSIZE, main_thread,
                NULL, NULL, NULL, PRIORITY, 0, 0);
