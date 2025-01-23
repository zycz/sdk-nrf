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


#include <zephyr/ipc/ipc_service.h>
#include <zephyr/pm/policy.h>


static K_SEM_DEFINE(bound_sem, 0, 1);

static void ep_bound(void *priv)
{
	k_sem_give(&bound_sem);
}

static void ep_recv(const void *data, size_t len, void *priv)
{
	// k_busy_wait(100);

}

static struct ipc_ept_cfg ep_cfg = {
	.name = "ep0",
	.cb = {
		.bound    = ep_bound,
		.received = ep_recv,
	},
};





static uint8_t message[10];



int main(void)
{
	soc_lrcconf_poweron_release(&soc_node, NRF_LRCCONF_POWER_DOMAIN_0);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);

#if 1
	const struct device *ipc0_instance;
	struct ipc_ept ep;
	int ret;

	ipc0_instance = DEVICE_DT_GET(DT_NODELABEL(ipc0));

	ret = ipc_service_open_instance(ipc0_instance);
	if ((ret < 0) && (ret != -EALREADY)) {
		LOG_INF("ipc_service_open_instance() failure (%d)", ret);
		return ret;
	}

	ret = ipc_service_register_endpoint(ipc0_instance, &ep, &ep_cfg);
	if (ret < 0) {
		printf("ipc_service_register_endpoint() failure (%d)", ret);
		return ret;
	}

	k_sem_take(&bound_sem, K_FOREVER);
	k_sleep(K_FOREVER);
#endif

	// k_msleep(7);
// 	unsigned int cnt = 0;

// #if defined CONFIG_FIRST_SLEEP_OFFSET
// 	k_msleep(1000);
// #endif

	// LOG_INF("Multicore idle test on %s", CONFIG_BOARD_TARGET);

	// nrf_lrcconf_poweron_force_set(NRF_LRCCONF010, NRF_LRCCONF_POWER_MAIN, true);
	// NRF_LRCCONF010->EVENTS_HFXOSTARTED = 0x0;
	// NRF_LRCCONF010->TASKS_REQHFXO = 0x1;

	int64_t next_timeout = k_uptime_get();
	while (true) {
		// ret = ipc_service_send(&ep, message, sizeof(message));
		// if (ret == -ENOMEM) {
		// 	/* No space in the buffer. Retry. */
		// 	continue;
		// } else if (ret < 0) {
		// 	while (true);
		// }

		next_timeout += 1;
		k_sleep(K_TIMEOUT_ABS_MS(next_timeout));

	}


	return 0;
}
