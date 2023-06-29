/* 
 *  watchdog.c - watchdog timer & reset routines 
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/printk.h>

#include "watchdog.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(watchdog, LOG_LEVEL_INF);

/*
 *  To use this sample the devicetree's /aliases must 
 *  have a 'watchdog0' property.
 */
#if DT_HAS_COMPAT_STATUS_OKAY(st_stm32_window_watchdog)
  #define WDT_MAX_WINDOW  100U
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_wdt)
  #define WDT_ALLOW_CALLBACK 0
#elif DT_HAS_COMPAT_STATUS_OKAY(raspberrypi_pico_watchdog)
  #define WDT_ALLOW_CALLBACK 0
#elif DT_HAS_COMPAT_STATUS_OKAY(gd_gd32_wwdgt)
  #define WDT_MAX_WINDOW 24U
  #define WDT_MIN_WINDOW 18U
  #define WDG_FEED_INTERVAL 12U
#elif DT_HAS_COMPAT_STATUS_OKAY(intel_tco_wdt)
  #define WDT_ALLOW_CALLBACK 0
  #define WDT_MAX_WINDOW 3000U
#endif

#ifndef WDT_ALLOW_CALLBACK
  #define WDT_ALLOW_CALLBACK 1
#endif

#ifndef WDT_MAX_WINDOW
  #define WDT_MAX_WINDOW  1000U
#endif

#ifndef WDT_MIN_WINDOW
  #define WDT_MIN_WINDOW  0U
#endif

#ifndef WDG_FEED_INTERVAL
  #define WDG_FEED_INTERVAL 200U
#endif

#define WDT_FEED_TRIES 5

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
#if WDT_ALLOW_CALLBACK
static void wdt_callback(const struct device * wdt_dev, int channel_id)
{
	static bool handled_event;

	if (handled_event) {
		return;
	}

	wdt_feed(wdt_dev, channel_id);

	LOG_ERR("Handled things..ready to reset");
	handled_event = true;
}
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
void watchdog_init(void)
{
	int wdt_channel_id;
	const struct device *const wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));

	LOG_INF("begin...");

	if (!device_is_ready(wdt)) {
		LOG_ERR("%s: device not ready.", wdt->name);
		return;
	}

	struct wdt_timeout_cfg wdt_config = {
		/* Reset SoC when watchdog timer expires. */
		.flags = WDT_FLAG_RESET_SOC,

		/* Expire watchdog after max window */
		.window.min = WDT_MIN_WINDOW,
		.window.max = WDT_MAX_WINDOW,
		.callback   = NULL,
	};

#if WDT_ALLOW_CALLBACK
	wdt_config.callback = wdt_callback;
#endif

	wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
	if (wdt_channel_id == -ENOTSUP) {
		wdt_config.callback = NULL;
		wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
	}
	if (wdt_channel_id < 0) {
		LOG_ERR("Watchdog install error");
		return;
	}

	if (wdt_setup(wdt, WDT_OPT_PAUSE_HALTED_BY_DBG) < 0) {
		LOG_INF("Watchdog setup error");
		return;
	}

#if WDT_MIN_WINDOW != 0
	/* Wait opening window. */
	k_msleep(WDT_MIN_WINDOW);
#endif

	/* Feeding watchdog. */
	while (true) {
		wdt_feed(wdt, wdt_channel_id);
		k_sleep(K_MSEC(WDG_FEED_INTERVAL));
	}

	/* unreachable */	
}
