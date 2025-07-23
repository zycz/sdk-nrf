/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <string.h>

#include <zephyr/logging/log.h>

#include <zephyr/ipc/ipc_service.h>

#include <hal/nrf_gpio.h>

struct payload {
	unsigned long cnt;
	unsigned long size;
	uint8_t data[];
};

struct payload *p_payload;
const struct device *ipc0_instance;
struct ipc_ept ep;

#if defined(CONFIG_SOC_NRF54H20_CPUAPP)

#include <zephyr/drivers/gpio.h>

#define BUTTON0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(BUTTON0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET_OR(BUTTON0_NODE, gpios, {0});
static struct gpio_callback button0_cb_data;

void button0_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	nrf_gpio_pin_toggle(NRF_GPIO_PIN_MAP(1, 2));
	ipc_service_send(&ep, p_payload, CONFIG_APP_IPC_SERVICE_MESSAGE_LEN);
	p_payload->cnt++;
}

static int button0_init(void)
{
	int ret;

	if (!device_is_ready(button0.port)) {
		printk("Error: button device %s is not ready\n", button0.port->name);
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
			   ret, button0.port->name, button0.pin);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&button0, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			   ret, button0.port->name, button0.pin);
		return ret;
	}

	gpio_init_callback(&button0_cb_data, button0_pressed, BIT(button0.pin));
	gpio_add_callback(button0.port, &button0_cb_data);

	// printk("Set up button at %s pin %d\n", button0.port->name, button0.pin);

	return 0;
}

/* Call button0_init() early in main() */
// __attribute__((constructor))
static void button0_setup_constructor(void)
{
	button0_init();
}

#endif /* CONFIG_SOC_NRF54H20_CPUAPP */


#ifdef CONFIG_TEST_EXTRA_STACK_SIZE
#define STACKSIZE	(1024 + CONFIG_TEST_EXTRA_STACK_SIZE)
#else
#define STACKSIZE	(1024)
#endif

K_THREAD_STACK_DEFINE(ipc0_stack, STACKSIZE);

LOG_MODULE_REGISTER(host, LOG_LEVEL_INF);




static K_SEM_DEFINE(bound_sem, 0, 1);

static void ep_bound(void *priv)
{
	k_sem_give(&bound_sem);
}

static void ep_recv(const void *data, size_t len, void *priv)
{
	nrf_gpio_pin_toggle(NRF_GPIO_PIN_MAP(1, 4));

	uint8_t received_val = *((uint8_t *)data);
	static uint8_t expected_val;


	if ((received_val != expected_val) || (len != CONFIG_APP_IPC_SERVICE_MESSAGE_LEN)) {
		printk("Unexpected message received_val: %d , expected_val: %d\n",
			received_val,
			expected_val);
	}

	expected_val++;
}

static struct ipc_ept_cfg ep_cfg = {
	.name = "ep0",
	.cb = {
		.bound    = ep_bound,
		.received = ep_recv,
	},
};

static void check_task(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	unsigned long last_cnt = p_payload->cnt;
	unsigned long delta;

	while (1) {
		k_sleep(K_MSEC(100000));

		delta = p_payload->cnt - last_cnt;

		printk("Δpkt: %ld (%ld B/pkt) | throughput: %ld bit/s\n",
			delta, p_payload->size, delta * CONFIG_APP_IPC_SERVICE_MESSAGE_LEN * 8);

		last_cnt = p_payload->cnt;
	}
}

K_THREAD_DEFINE(thread_check_id, STACKSIZE, check_task, NULL, NULL, NULL,
		K_PRIO_COOP(1), 0, -1);

int main(void)
{
#if defined(CONFIG_SOC_NRF54H20_CPURAD)
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 4));
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 5));
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 6));
#else
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 2));
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 3));
	nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(1, 8));
	button0_setup_constructor();
#endif


	int ret;

	p_payload = (struct payload *) k_malloc(CONFIG_APP_IPC_SERVICE_MESSAGE_LEN);
	if (!p_payload) {
		printk("k_malloc() failure\n");
		return -ENOMEM;
	}

	memset(p_payload->data, 0xA5, CONFIG_APP_IPC_SERVICE_MESSAGE_LEN - sizeof(struct payload));

	p_payload->size = CONFIG_APP_IPC_SERVICE_MESSAGE_LEN;
	p_payload->cnt = 0;

	printk("IPC-service %s demo started\n", CONFIG_BOARD_TARGET);

	ipc0_instance = DEVICE_DT_GET(DT_NODELABEL(ipc0));

#if defined(CONFIG_IPC_SERVICE_BACKEND_ICBMSG)
	/* Wait a bit for the remote core to boot. */
	k_msleep(100);
#endif

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

#if defined(CONFIG_SOC_NRF54H20_CPURAD)
	k_msleep(99999);
#endif

	k_sem_take(&bound_sem, K_FOREVER);
	k_thread_start(thread_check_id);

	while (true) {
		// nrf_gpio_pin_toggle(NRF_GPIO_PIN_MAP(1, 2));
		// ret = ipc_service_send(&ep, p_payload, CONFIG_APP_IPC_SERVICE_MESSAGE_LEN);
		// if (ret == -ENOMEM) {
		// 	/* No space in the buffer. Retry. */
		// 	continue;
		// } else if (ret < 0) {
		// 	printk("send_message(%ld) failed with ret %d\n", p_payload->cnt, ret);
		// 	break;
		// }

		// p_payload->cnt++;


		/* Quasi minimal busy wait time which allows to continuously send
		 * data without -ENOMEM error code. The purpose is to test max
		 * throughput. Determined experimentally.
		 */
		// if (CONFIG_APP_IPC_SERVICE_SEND_INTERVAL < 1000) {
		// 	k_busy_wait(CONFIG_APP_IPC_SERVICE_SEND_INTERVAL);
		// } else {
			// k_msleep(CONFIG_APP_IPC_SERVICE_SEND_INTERVAL/1000);
			k_msleep(99999);
		// }
	}

	return 0;
}
