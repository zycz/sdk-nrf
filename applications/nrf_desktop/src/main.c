/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app_event_manager.h>

#define MODULE main
#include <caf/events/module_state_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE);

#if defined(CONFIG_SOC_NRF54H20_CPUAPP)
#include "periphconf_l2cache.h"
#endif

int main(void)
{
#if defined(CONFIG_SOC_NRF54H20_CPUAPP)
	nrf_desktop_l2cache_log_state();
#endif

	if (app_event_manager_init()) {
		LOG_ERR("Application Event Manager not initialized");
	} else {
		module_set_state(MODULE_STATE_READY);
	}
	return 0;
}
