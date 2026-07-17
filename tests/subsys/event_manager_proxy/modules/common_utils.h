/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <app_event_manager.h>


/**
 * @brief IPC device used to communicate with other CORE.
 *
 * The REMOTE prefix here means the core remote in relation to the core on
 * which the code is executed.
 */
#define REMOTE_IPC_DEV DEVICE_DT_GET(DT_NODELABEL(ipc0))

/** @brief IPC instance node used by this test. */
#define EMP_IPC_NODE DT_NODELABEL(ipc0)

/** @brief Number of bytes captured/printed per shared-memory region. */
#define EMP_SHMEM_DUMP_LEN 64

/** @brief Delay before printing the captured snapshot, in seconds.
 *
 * The snapshot is taken immediately at startup, but printing is delayed so a
 * serial terminal can be attached in time to capture it - in particular the
 * very first boot after power-on, when the memory should still be clean.
 */
#define EMP_SHMEM_PRINT_DELAY_S 5

/**
 * @brief Capture the IPC shared-memory regions at startup and print them later.
 *
 * The raw bytes of the tx/rx shared-memory regions (the area that holds the
 * ICMsg indexes and the handshake "magic" bytes) are copied into a local
 * buffer BEFORE the IPC backend re-initializes them. Printing is then delayed
 * by @ref EMP_SHMEM_PRINT_DELAY_S seconds so the output can be captured even on
 * the first boot after power-on.
 *
 * The purpose is purely diagnostic:
 *  - on the first (cold) boot it shows the initial, clean memory,
 *  - on any boot after a reset it shows that the memory is NOT cleared -
 *    the old indexes/magic from the previous run are still present.
 *
 * @param core_name Human readable name of the core doing the dump.
 */
static inline void ipc_shared_memory_startup_dump(const char *core_name)
{
#if DT_NODE_HAS_PROP(EMP_IPC_NODE, tx_region) && DT_NODE_HAS_PROP(EMP_IPC_NODE, rx_region)
	const uintptr_t regions[] = {
		DT_REG_ADDR(DT_PHANDLE(EMP_IPC_NODE, tx_region)),
		DT_REG_ADDR(DT_PHANDLE(EMP_IPC_NODE, rx_region)),
	};
	static const char *const names[] = { "tx", "rx" };
	static uint8_t snapshot[ARRAY_SIZE(regions)][EMP_SHMEM_DUMP_LEN];

	/* Snapshot the content as early as possible, before the IPC backend
	 * re-initializes the shared memory.
	 */
	for (size_t r = 0; r < ARRAY_SIZE(regions); r++) {
		const volatile uint8_t *p = (const volatile uint8_t *)regions[r];

		for (size_t i = 0; i < EMP_SHMEM_DUMP_LEN; i++) {
			snapshot[r][i] = p[i];
		}
	}

	/* Delay so a serial terminal can be attached before the values are
	 * printed (needed to catch the initial, clean values on a cold boot).
	 */
	k_sleep(K_SECONDS(EMP_SHMEM_PRINT_DELAY_S));

	printk("\n[%s] IPC shared memory captured at startup (before backend init):\n",
	       core_name);
	for (size_t r = 0; r < ARRAY_SIZE(regions); r++) {
		printk("  %s-region @ 0x%08lx:\n", names[r], (unsigned long)regions[r]);
		for (size_t i = 0; i < EMP_SHMEM_DUMP_LEN; i += 16) {
			printk("    +0x%02zx:", i);
			for (size_t j = 0; j < 16; j++) {
				printk(" %02x", snapshot[r][i + j]);
			}
			printk("\n");
		}
	}
#else
	ARG_UNUSED(core_name);
	printk("[%s] ipc0 has no tx/rx region property, cannot dump shared memory\n",
	       core_name);
#endif
}

/**
 * @brief Auxiliary macro to subscribe to remote event.
 *
 * The macro that provides error checking and status logging about the status of the
 * subscription process.
 *
 * @param _ipc The IPC instance to subscribe.
 * @param _ev  The event mnemonic to use.
 */
#define REMOTE_EVENT_SUBSCRIBE(_ipc, _ev) do {                                            \
		int ret = EVENT_MANAGER_PROXY_SUBSCRIBE(_ipc, _ev);                       \
		if (ret) {                                                                \
			LOG_ERR("Cannot register to remote %s: %d", STRINGIFY(_ev), ret); \
			__ASSERT_NO_MSG(false);                                           \
		}                                                                         \
		LOG_INF("Event proxy %s registered", STRINGIFY(_ev));                     \
	} while (0)

/**
 * @brief Auxiliary macro to subscribe to remote event.
 *
 * The macro that provides error checking using zassert macros.
 *
 * @param _ipc The IPC instance to subscribe.
 * @param _ev  The event mnemonic to use.
 */
#define REMOTE_EVENT_SUBSCRIBE_TEST(_ipc, _ev) do {                                       \
		int ret = EVENT_MANAGER_PROXY_SUBSCRIBE(_ipc, _ev);                       \
		zassert_ok(ret, "Cannot register to remote %s: %d", STRINGIFY(_ev), ret); \
	} while (0)

/**
 * @brief Send event message directly to proxy.
 *
 * This function transmits given message directly to
 * the proxy (bypassing the app_event_manager).
 *
 * @param eh Event header.
 */
void proxy_direct_submit_event(struct app_event_header *eh);

/**
 * @brief Transfer a bulk of simple events directly to proxy.
 *
 * This function transmits a bulk of simple messages directly to
 * the proxy (bypassing the app_event_manager).
 * This allows to check real throughput of the proxy itself.
 *
 * @return 0 or error code.
 */
int proxy_burst_simple_events(void);

/**
 * @brief Transfer a bulk of simple pong events directly to proxy.
 *
 * The function similar like @ref proxy_bulk_simple_events but is
 * prepared to be used from remote core.
 *
 * @return 0 or error code.
 *
 * @sa proxy_bulk_simple_events
 */
int proxy_burst_simple_pong_events(void);

/**
 * @brief Transfer a bulk of data events directly to proxy.
 *
 * The function transmits a bulk of data messages directly to
 * the proxy (bypassing the app_event_manager).
 * This allows to check real throughput of the proxy itself.
 *
 * @return 0 or error code.
 */
int proxy_burst_data_events(void);

/**
 * @brief Transfer a bulk of data response events directly to proxy.
 *
 * The function transmits a bulk of data response messages directly to
 * the proxy (bypassing the app_event_manager).
 * This allows to check real throughput of the proxy itself.
 *
 * @return 0 or error code.
 */
int proxy_burst_data_response_events(void);

/**
 * @brief Transfer a bulk of data big events directly to proxy.
 *
 * The function transmits a bulk of data big messages directly to
 * the proxy (bypassing the app_event_manager).
 * This allows to check real throughput of the proxy itself.
 *
 * @return 0 or error code.
 */
int proxy_burst_data_big_events(void);

/**
 * @brief Transfer a bulk of data big response events directly to proxy.
 *
 * The function transmits a bulk of data big response messages directly to
 * the proxy (bypassing the app_event_manager).
 * This allows to check real throughput of the proxy itself.
 *
 * @return 0 or error code.
 */
int proxy_burst_data_big_response_events(void);

#endif /* _COMMON_UTILS_H_ */
