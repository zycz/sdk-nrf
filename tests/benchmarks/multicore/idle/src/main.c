/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <hal/nrf_gpio.h>



int main(void)
{
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 3));
	nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 3));

	while (1) {

		nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(1, 3));
		k_msleep(2000);
		// nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 3));
		// k_busy_wait(100000);
	}

	return 0;
}
