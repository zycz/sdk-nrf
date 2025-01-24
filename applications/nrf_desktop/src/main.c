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
#include <hal/nrf_gpio.h>

static void gpio_init(void)
{
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 3));
	nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 3));
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 4));
	nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 4));
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 5));
	nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(1, 5));
}

int main(void)
{
	gpio_init();
	if (app_event_manager_init()) {
		LOG_ERR("Application Event Manager not initialized");
	} else {
		module_set_state(MODULE_STATE_READY);
	}
	return 0;
}
