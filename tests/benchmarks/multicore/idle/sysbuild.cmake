#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

if(SB_CONFIG_SOC_NRF54H20_CPURAD)
  # Add remote project
  ExternalZephyrProject_Add(
      APPLICATION remote
      SOURCE_DIR ${APP_DIR}/remote
      BOARD "${BOARD}/nrf54h20/cpuapp"
      BOARD_REVISION ${BOARD_REVISION}
    )
endif()
