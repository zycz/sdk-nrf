/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>
#include <soc.h>
#include <hal/nrf_gpio.h>

#define TRIGGER_NODE		DT_ALIAS(led1)
#define BUTTON_NODE		DT_ALIAS(sw0)

#define LATCH_SETTLE_US		10
#define CALLBACK_WAIT_MS	10
#define BUTTON_PAUSE_MS		2000

#define TEST_PIN_INIT(node, _name)						\
	{									\
		.name = _name,							\
		.port = DEVICE_DT_GET(DT_GPIO_CTLR(node, gpios)),		\
		.pin = DT_GPIO_PIN(node, gpios),				\
		.dt_flags = DT_GPIO_FLAGS(node, gpios),				\
		.psel = NRF_DT_GPIOS_TO_PSEL(node, gpios),			\
	}

enum state {
	STATE_ACTIVE,
	STATE_SUSPENDED,
};

struct test_pin {
	const char *name;
	const struct device *port;
	gpio_pin_t pin;
	gpio_flags_t dt_flags;
	uint32_t psel;
	struct gpio_callback cb;
	enum state state;
	atomic_t valid_cnt;
	atomic_t stale_cnt;
};

static struct test_pin trigger_pin = TEST_PIN_INIT(TRIGGER_NODE, "self-trigger (LED1 pin)");
static struct test_pin button_pin = TEST_PIN_INIT(BUTTON_NODE, "Button 0");
static struct k_spinlock lock;

/* Same sequence as wakeup_int_ctrl_nolock() in the nRF Desktop wheel module:
 * arm a level interrupt opposite to the current pin level, or disable it.
 */
static int wakeup_int_ctrl_nolock(struct test_pin *tp, bool enable)
{
	if (!enable) {
		int err = gpio_pin_interrupt_configure(tp->port, tp->pin, GPIO_INT_DISABLE);

		if (!err && IS_ENABLED(CONFIG_SAMPLE_CLEAR_LATCH)) {
			nrf_gpio_pin_latch_clear(tp->psel);
		}

		return err;
	}

	int val = gpio_pin_get_raw(tp->port, tp->pin);

	if (val < 0) {
		printk("%s: cannot read pin (%d)\n", tp->name, val);
		return val;
	}

	return gpio_pin_interrupt_configure(tp->port, tp->pin,
					    val ? GPIO_INT_LEVEL_LOW : GPIO_INT_LEVEL_HIGH);
}

static void wakeup_cb(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
	struct test_pin *tp = CONTAINER_OF(cb, struct test_pin, cb);
	k_spinlock_key_t key = k_spin_lock(&lock);

	if (tp->state == STATE_SUSPENDED) {
		atomic_inc(&tp->valid_cnt);
		(void)wakeup_int_ctrl_nolock(tp, false);
		tp->state = STATE_ACTIVE;
	} else {
		/* nRF Desktop wheel module asserts here. */
		atomic_inc(&tp->stale_cnt);
	}

	k_spin_unlock(&lock, key);
}

static int suspend(struct test_pin *tp)
{
	k_spinlock_key_t key = k_spin_lock(&lock);
	int err = wakeup_int_ctrl_nolock(tp, true);

	if (!err) {
		tp->state = STATE_SUSPENDED;
	}

	k_spin_unlock(&lock, key);

	return err;
}

static int resume_nolock(struct test_pin *tp)
{
	int err = 0;

	if (tp->state == STATE_SUSPENDED) {
		err = wakeup_int_ctrl_nolock(tp, false);
		if (!err) {
			tp->state = STATE_ACTIVE;
		}
	}

	return err;
}

static int pin_init(struct test_pin *tp, gpio_flags_t flags)
{
	if (!device_is_ready(tp->port)) {
		printk("%s: GPIO port not ready\n", tp->name);
		return -ENODEV;
	}

	int err = gpio_pin_configure(tp->port, tp->pin, flags | tp->dt_flags);

	if (err) {
		printk("%s: cannot configure pin (%d)\n", tp->name, err);
		return err;
	}

	tp->state = STATE_ACTIVE;
	gpio_init_callback(&tp->cb, wakeup_cb, BIT(tp->pin));

	err = gpio_add_callback(tp->port, &tp->cb);
	if (err) {
		printk("%s: cannot add callback (%d)\n", tp->name, err);
	}

	return err;
}

static void counters_reset(struct test_pin *tp)
{
	atomic_clear(&tp->valid_cnt);
	atomic_clear(&tp->stale_cnt);
}

/* Toggle the pin while the lock is held, so the level is latched but the
 * GPIOTE interrupt cannot be handled. Optionally resume (disable the wake-up
 * interrupt) before releasing the lock, as the wheel module does on wake-up.
 */
static int self_trigger_iteration(struct test_pin *tp, bool resume_before_unlock)
{
	int err = suspend(tp);

	if (err) {
		return err;
	}

	k_spinlock_key_t key = k_spin_lock(&lock);

	err = gpio_pin_set_raw(tp->port, tp->pin, 1);
	k_busy_wait(LATCH_SETTLE_US);

	int set_err = gpio_pin_set_raw(tp->port, tp->pin, 0);

	err = err ? err : set_err;

	if (!err && resume_before_unlock) {
		err = resume_nolock(tp);
	}

	k_spin_unlock(&lock, key);

	k_msleep(CALLBACK_WAIT_MS);

	key = k_spin_lock(&lock);

	int resume_err = resume_nolock(tp);

	k_spin_unlock(&lock, key);

	return err ? err : resume_err;
}

static int self_trigger_test(struct test_pin *tp, bool resume_before_unlock, int iterations)
{
	counters_reset(tp);

	for (int i = 0; i < iterations; i++) {
		int err = self_trigger_iteration(tp, resume_before_unlock);

		if (err) {
			printk("%s: iteration %d failed (%d)\n", tp->name, i, err);
			return err;
		}
	}

	printk("%s, %s: valid callbacks: %ld, stale callbacks: %ld (of %d)\n",
	       tp->name,
	       resume_before_unlock ? "disabled before handling" : "control, kept enabled",
	       atomic_get(&tp->valid_cnt), atomic_get(&tp->stale_cnt), iterations);

	return 0;
}

static int button_iteration(struct test_pin *tp)
{
	counters_reset(tp);

	int err = suspend(tp);

	if (err) {
		return err;
	}

	printk("\nPress %s within %d ms (interrupts masked, button interrupt armed)...\n",
	       tp->name, CONFIG_SAMPLE_BUTTON_WINDOW_MS);

	k_spinlock_key_t key = k_spin_lock(&lock);

	k_busy_wait(CONFIG_SAMPLE_BUTTON_WINDOW_MS * USEC_PER_MSEC);
	err = resume_nolock(tp);

	k_spin_unlock(&lock, key);

	k_msleep(CALLBACK_WAIT_MS);

	printk("%s: interrupt disabled before handling, stale callbacks: %ld -> %s\n",
	       tp->name, atomic_get(&tp->stale_cnt),
	       atomic_get(&tp->stale_cnt) ? "BUG: callback after GPIO_INT_DISABLE" :
					    "no callback (not pressed in window, or fixed)");

	return err;
}

int main(void)
{
	printk("GPIO stale latch sample, latch clear workaround: %s\n",
	       IS_ENABLED(CONFIG_SAMPLE_CLEAR_LATCH) ? "enabled" : "disabled");

	int err = pin_init(&trigger_pin, GPIO_INPUT | GPIO_OUTPUT_INACTIVE);

	err = err ? err : pin_init(&button_pin, GPIO_INPUT);
	err = err ? err : self_trigger_test(&trigger_pin, false,
					    CONFIG_SAMPLE_SELF_TRIGGER_ITERATIONS);
	err = err ? err : self_trigger_test(&trigger_pin, true,
					    CONFIG_SAMPLE_SELF_TRIGGER_ITERATIONS);
	if (err) {
		printk("Sample failed (%d)\n", err);
		return err;
	}

	printk("Self-trigger test done\n");

	while (true) {
		err = button_iteration(&button_pin);
		if (err) {
			printk("Button test failed (%d)\n", err);
			return err;
		}

		k_msleep(BUTTON_PAUSE_MS);
	}

	return 0;
}
