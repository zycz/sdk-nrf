.. _multicore_idle_test:

Multicore idle test with firmware relocated to radio core TCM 
#############################################################

.. contents::
   :local:
   :depth: 2

The test benchmarks the idle behavior of an application that runs on multiple cores.
It demonstrates a radio loader pattern where the radio core firmware is loaded from MRAM into TCM (Tightly Coupled Memory) at runtime.

Requirements
************

The test supports the following development kit:

.. table-from-rows:: /includes/sample_board_rows.txt
   :header: heading
   :rows: nrf54h20dk_nrf54h20_cpuapp

Overview
********

This test demonstrates how to build a multicore idle application with :ref:`configuration_system_overview_sysbuild` using a two-stage boot process for the radio core:

1. **Radio Loader** - A small bootloader that runs on the radio core, copies firmware from MRAM to TCM, and jumps to it
2. **Remote Firmware** - The actual application that runs from TCM after being loaded

The test automatically relocates the remote firmware binary to the correct MRAM address during build time, ensuring it can be loaded by the radio loader.

Architecture
============

The system uses the following memory layout:

* **MRAM (Non-volatile):**
  
  * ``cpurad_loader_partition`` @ 0x0e092000 - Contains the radio loader (92 KB)
  * ``cpurad_loaded_fw`` @ 0x0e0a9000 - Contains the remote firmware binary (128 KB)

* **TCM (Volatile, fast execution):**
  
  * ``cpurad_ram0`` @ 0x23000000 - Code execution region (128 KB)
  * ``cpurad_data_ram`` @ 0x23020000 - Data region (64 KB)

Additional Files
================

The test comes with the following additional files:

* :file:`sysbuild.conf` - Enables the radio loader via ``CONFIG_NRF_RADIO_LOADER=y``
* :file:`sysbuild.cmake` - Adds remote firmware using ``nrf_cpurad_image()`` helper for automatic relocation
* :file:`boards/memory_map.overlay` - Shared memory map configuration for both loader and remote firmware
* :file:`sysbuild/radio_loader/` - Radio loader configuration overrides (prj.conf, overlay)
* :file:`remote/CMakeLists.txt` - Standard application CMakeLists.txt (no relocation logic needed)

Enabling the Radio Loader
**************************

The radio loader is automatically added to the build when you enable it in sysbuild configuration.

In :file:`sysbuild.conf`:

.. code-block:: kconfig

   CONFIG_NRF_RADIO_LOADER=y

This single configuration option:

1. Automatically adds the ``radio_loader`` application from ``nrf/samples/radio_loader``
2. Builds it for the CPURAD core
3. No manual ``ExternalZephyrProject_Add()`` needed in sysbuild.cmake!

Configuring the Radio Loader
*****************************

The radio loader is responsible for copying the remote firmware from MRAM to TCM and jumping to it. 
You configure the memory map in devicetree overlays.

Memory Map Configuration
========================

The memory map is defined in :file:`boards/memory_map.overlay` and is shared between the radio loader and remote firmware to ensure consistency.

The overlay defines:

1. **TCM regions:**

   .. code-block:: devicetree

      cpurad_ram0: sram@23000000 {
          compatible = "mmio-sram";
          reg = <0x23000000 0x20000>;  /* 128 KB for code */
      };

      cpurad_data_ram: sram@23020000 {
          compatible = "mmio-sram";
          reg = <0x23020000 0x10000>;  /* 64 KB for data */
      };

2. **MRAM partitions:**

   .. code-block:: devicetree

      &{/soc/mram@e000000/partitions} {
          cpurad_loader_partition: partition@92000 {
              label = "cpurad_loader_partition";
              reg = <0x92000 DT_SIZE_K(92)>;
          };

          cpurad_loaded_fw: partition@a9000 {
              label = "cpurad_loaded_fw";
              reg = <0xa9000 0x20000>;  /* 128 KB */
          };
      };

Chosen Nodes
============

Each image (radio loader and remote firmware) requires specific ``chosen`` nodes in its overlay:

**Radio Loader** (:file:`sysbuild/radio_loader/boards/nrf54h20dk_nrf54h20_cpurad.overlay`):

.. code-block:: devicetree

   /{
       chosen {
           zephyr,code-partition = &cpurad_loader_partition;
           zephyr,sram = &cpurad_data_ram;
           zephyr,loaded-fw-src = &cpurad_loaded_fw;  /* Source in MRAM */
           zephyr,loaded-fw-dst = &cpurad_ram0;       /* Destination in TCM */
       };
   };

**Remote Firmware** (:file:`remote/boards/nrf54h20dk_nrf54h20_cpurad.overlay`):

.. code-block:: devicetree

   /{
       chosen {
           // CONFIG_BUILD_OUTPUT_ADJUST_LMA automatically calculates LMA adjustment
           zephyr,code-partition = &cpurad_loaded_fw;  // LMA: load from MRAM
           zephyr,sram = &cpurad_ram0;                  // VMA: run from TCM
       };
   };

Automatic Firmware Relocation
******************************

The remote firmware must be relocated to match the MRAM partition address where it will be stored.
This is done **completely automatically** by Zephyr's ``CONFIG_BUILD_OUTPUT_ADJUST_LMA`` feature when the devicetree chosen nodes are configured correctly.

How It Works
============

Firmware relocation is handled automatically by Zephyr's build system using the ``CONFIG_BUILD_OUTPUT_ADJUST_LMA`` configuration option, which is configured in ``zephyr/soc/nordic/nrf54h/Kconfig.defconfig.nrf54h20_cpurad`` for all nRF54H20 CPURAD projects.

The configuration automatically detects the ``fw-to-relocate`` chosen node in your devicetree. If present, it calculates the LMA adjustment. If not, it falls back to standard behavior.

Simply configure the devicetree chosen nodes correctly in your firmware's overlay:

.. code-block:: devicetree

   /{
       chosen {
           // LMA: where to load from (MRAM partition)
           zephyr,code-partition = &cpurad_loaded_fw;
           
           // VMA: where to run (TCM)
           zephyr,sram = &cpurad_ram0;
       };
   };

Zephyr automatically calculates the Load Memory Address (LMA) adjustment based on your chosen nodes:

**With fw-to-relocate chosen node** (for radio loader pattern):

.. code-block:: text

   LMA_adjustment = fw-to-relocate address - zephyr,code-partition address
                  = cpurad_loaded_fw - cpurad_ram0
                  = 0x0e0a9000 - 0x23000000

**Without fw-to-relocate** (standard behavior):

.. code-block:: text

   LMA_adjustment = zephyr,code-partition address - zephyr,sram address

The build system then adjusts the hex file so that the firmware:
- Is **loaded from** MRAM (``0x0e0a9000``)
- But **runs from** TCM (``0x23000000``)

**No manual objcopy or CMake manipulation needed!** ✨

Adding Remote Firmware in Sysbuild
===================================

Simply add the remote firmware in :file:`sysbuild.cmake`:

.. code-block:: cmake

   ExternalZephyrProject_Add(
       APPLICATION remote
       SOURCE_DIR ${APP_DIR}/remote
       BOARD nrf54h20dk/nrf54h20/cpurad
       BOARD_REVISION ${BOARD_REVISION}
   )

That's it! The relocation is **completely automatic** thanks to:

1. ``CONFIG_BUILD_OUTPUT_ADJUST_LMA`` in the SoC defconfig
2. ``fw-to-relocate`` chosen node in your devicetree overlay

**No helper functions, no custom CMake, no manual configuration needed!**

Adapting to Your Project
=========================

To use the radio loader pattern in your own nRF54H20 CPURAD project:

1. **Enable radio loader in sysbuild.conf:**
   
   Create :file:`sysbuild.conf` with:

   .. code-block:: kconfig

      CONFIG_NRF_RADIO_LOADER=y

2. **Define memory map:**
   
   Create a devicetree overlay with your partition layout:

   .. code-block:: devicetree

      &{/soc/mram@e000000/partitions} {
          my_firmware: partition@a9000 {
              reg = <0xa9000 0x20000>;
          };
      };

3. **Configure chosen nodes:**
   
   Add the chosen nodes to your firmware's overlay:

   .. code-block:: devicetree

      /{
          chosen {
              // VMA: where code runs
              zephyr,code-partition = &my_tcm_region;  // TCM
              zephyr,sram = &my_data_ram;              // Data RAM
              
              // LMA: where code is loaded from (enables relocation)
              fw-to-relocate = &my_firmware;           // MRAM partition
          };
      };

4. **Add remote firmware in sysbuild.cmake:**
   
   Simply add your firmware as an external project:

   .. code-block:: cmake

      ExternalZephyrProject_Add(
          APPLICATION my_remote
          SOURCE_DIR ${APP_DIR}/remote
          BOARD nrf54h20dk/nrf54h20/cpurad
      )

That's it! **No changes needed in your firmware's CMakeLists.txt!** Everything happens automatically:

* Radio loader is added via Kconfig (``CONFIG_NRF_RADIO_LOADER=y``)
* Firmware relocation handled by ``CONFIG_BUILD_OUTPUT_ADJUST_LMA`` (configured in SoC defconfig)
* Just add the ``fw-to-relocate`` chosen node in your devicetree overlay!

Building and running
********************

.. |test path| replace:: :file:`tests/benchmarks/multicore/idle_relocated_tcm`

.. include:: /includes/build_and_run_test.txt

Build the test for application and radio cores as follows:

.. code-block:: console

   west build -p -b nrf54h20dk/nrf54h20/cpuapp -T benchmarks.multicore.idle.nrf54h20dk_cpuapp_cpurad .

The build will proceed normally. Firmware relocation happens automatically during the build process based on your devicetree configuration.

.. include:: /includes/nRF54H20_erase_UICR.txt

Testing
=======

After programming the test to your development kit, complete the following steps to test it:

1. |connect_terminal|
#. Reset the kit.
#. Observe the console output for both cores:

   * For the application core, the output should be as follows:

     .. code-block:: console

       *** Booting nRF Connect SDK zephyr-v3.5.0-3517-g9458a1aaf744 ***
       Multi-core idle test on nrf54h20dk/nrf54h20/cpuapp
       Multi-core idle test iteration 0
       Multi-core idle test iteration 1
       ...

   * For the radio core, the output should be as follows:

     .. code-block:: console

        *** Booting nRF Connect SDK zephyr-v3.5.0-3517-g9458a1aaf744 ***
     Multicore idle test on nrf54h20dk@0.9.0/cpuapp
     Current PC (program counter) address: 0x23000ab8
     Multicore idle test iteration 0
     Multicore idle test iteration 1
     ...

The radio loader first loads the firmware from MRAM (``0x0e0a9000``) to TCM (``0x23000000``) and then jumps to the loaded firmware.
This process is transparent and happens during the early boot stage.
