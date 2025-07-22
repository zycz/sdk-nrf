/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

#include <hal/nrf_gpio.h>

LOG_MODULE_REGISTER(idle);

int main(void)
{
#if defined(CONFIG_SOC_NRF54H20_CPURAD)
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 5));
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 6));
#else
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 3));
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 8));
#endif

	unsigned int cnt = 0;

#if defined CONFIG_FIRST_SLEEP_OFFSET
	k_msleep(1000);
#endif

	LOG_INF("Multicore idle test on %s", CONFIG_BOARD_TARGET);
	while (1) {
		LOG_INF("Multicore idle test iteration %u", cnt++);
		k_msleep(2000);
	}

	return 0;
}
