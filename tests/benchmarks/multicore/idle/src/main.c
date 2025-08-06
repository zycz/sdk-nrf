/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#include "ld_dvfs_handler.h"

LOG_MODULE_REGISTER(idle);

int main(void)
{
	unsigned int cnt = 0;

#ifdef CONFIG_FIRST_SLEEP_OFFSET
	k_msleep(1000);
#endif

	int status = dvfs_service_handler_change_freq_setting(DVFS_FREQ_LOW);

	if (status == -EAGAIN) {
		LOG_WRN("DVFS not initialized, try again.");
	} else if (status == -EBUSY) {
		LOG_WRN("DVFS frequency change in progress");
	} else if (status != 0) {
		LOG_ERR("DVFS freq change returned with error: %d", status);
	} else {
		LOG_INF("Requesting frequency setting DVFS_FREQ_LOW");
	}

	LOG_INF("Multicore idle test on %s", CONFIG_BOARD_TARGET);
	while (1) {
		LOG_INF("Multicore idle test iteration %u", cnt++);
#ifdef CONFIG_FPU
		LOG_INF("FPU test: 1/%d = %f", cnt, (double)1/cnt);
#endif
		k_msleep(2000);
	}

	return 0;
}
