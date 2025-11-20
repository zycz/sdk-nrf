#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#



# include_guard(GLOBAL)
# # Add remote project
# message(MESSAGE "Adding remote project")




# # if (CONFIG_NCS_IS_VARIANT_IMAGE)
# #     ExternalZephyrProject_Add(
# #         APPLICATION remote_rad
# #         SOURCE_DIR ${APP_DIR}/remote
# #         BOARD nrf54h20dk/nrf54h20/cpurad
# #         BOARD_REVISION ${BOARD_REVISION}
# #     )
# # endif()

# ExternalZephyrProject_Add(
#     APPLICATION remote_rad
#     SOURCE_DIR ${APP_DIR}/remote
#     BOARD nrf54h20dk/nrf54h20/cpurad
#     BOARD_REVISION ${BOARD_REVISION}
# )

# UpdateableImage_Add(APPLICATION remote_rad)


# sysbuild_add_dependencies(CONFIGURE ${DEFAULT_IMAGE} remote_rad)
# Add dependency so that the remote image is flashed first.
# sysbuild_add_dependencies(FLASH ${DEFAULT_IMAGE} remote_rad)
