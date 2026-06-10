/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <ironside_zephyr/se/uicr_periphconf.h>

#include <ironside/se/api.h>
#include <ironside/se/periphconf.h>
#include <hal/nrf_cache.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(periphconf_l2cache, LOG_LEVEL_INF);

UICR_PERIPHCONF_ENTRY(PERIPHCONF_L2CACHE_ENABLE(false));

void nrf_desktop_l2cache_log_state(void)
{
	struct periphconf_entry entry = { .regptr = PERIPHCONF_L2CACHE_ENABLE_REGPTR() };
	struct ironside_se_periphconf_status st = ironside_se_periphconf_read(&entry, 1);

	if (st.status != 0) {
		LOG_ERR("L2CACHE PERIPHCONF read failed: %d", st.status);
		return;
	}

	bool enabled = (entry.value & CACHE_ENABLE_ENABLE_Msk) ==
		       (CACHE_ENABLE_ENABLE_Enabled << CACHE_ENABLE_ENABLE_Pos);

	LOG_INF("L2CACHE ENABLE=%u (%s)", enabled, enabled ? "ON" : "OFF");
}
