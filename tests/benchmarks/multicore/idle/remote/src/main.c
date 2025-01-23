/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <soc_lrcconf.h>

LOG_MODULE_REGISTER(idle);

extern sys_snode_t soc_node;

int main(void)
{
	// k_sleep(K_FOREVER);
// 	unsigned int cnt = 0;

// #if defined CONFIG_FIRST_SLEEP_OFFSET
// 	k_msleep(1000);
// #endif
	soc_lrcconf_poweron_release(&soc_node, NRF_LRCCONF_POWER_DOMAIN_0);
	// LOG_INF("Multicore idle test on %s", CONFIG_BOARD_TARGET);

	// nrf_lrcconf_poweron_force_set(NRF_LRCCONF010, NRF_LRCCONF_POWER_MAIN, true);
	// NRF_LRCCONF010->EVENTS_HFXOSTARTED = 0x0;
	// NRF_LRCCONF010->TASKS_REQHFXO = 0x1;
	int64_t next_timeout = k_uptime_get();
	while (1) {
		// LOG_INF("Multicore idle test iteration %u", cnt++);
		next_timeout += 10;
		k_sleep(K_TIMEOUT_ABS_MS(next_timeout));

	}

	return 0;
}
