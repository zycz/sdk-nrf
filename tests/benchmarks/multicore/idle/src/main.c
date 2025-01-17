/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#include <nrfx.h>
#include <hal/nrf_cache.h>

LOG_MODULE_REGISTER(idle);

static void icache_profiling_init(void)
{
	if (IS_ENABLED(CONFIG_CACHE_NRF_CACHE)) {
		nrf_cache_profiling_set(NRF_ICACHE, true);
		nrf_cache_profiling_counters_clear(NRF_ICACHE);
	}
}

static void icache_profiling_print(void)
{
	if (IS_ENABLED(CONFIG_CACHE_NRF_CACHE)) {
		uint32_t icache_hit = nrf_cache_instruction_hit_counter_get(NRF_ICACHE,
									      NRF_CACHE_REGION_FLASH);
		uint32_t icache_miss = nrf_cache_instruction_miss_counter_get(NRF_ICACHE,
										NRF_CACHE_REGION_FLASH);

		LOG_INF("ICache hits: %u, misses: %u", icache_hit, icache_miss);
	}
}

int main(void)
{
// 	unsigned int cnt = 0;

// #if defined CONFIG_FIRST_SLEEP_OFFSET
// 	k_msleep(1000);
// #endif

	LOG_INF("Multicore idle test on %s", CONFIG_BOARD_TARGET);
// 	while (1) {
// 		LOG_INF("Multicore idle test iteration %u", cnt++);
// 		k_msleep(2000);
// 	}

	icache_profiling_init();
	icache_profiling_print();

	while (1)
	{
		LOG_INF("Multicore idle test on %s", CONFIG_BOARD_TARGET);
		icache_profiling_print();

		for(int i = 0; i < 100; i++)
		{

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");
			__asm__ volatile ("nop");

			LOG_INF("Multicore idle test after 100 nops, interation: %d", i);
			icache_profiling_print();
		}
		k_msleep(2000);
	}

	return 0;
}
