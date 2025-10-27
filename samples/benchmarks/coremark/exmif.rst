.. _exmif_external_memory:

EXMIF - External Memory Interface on nRF54H20
##############################################

.. contents::
   :local:
   :depth: 2

Overview
********

The **EXMIF (External Memory Interface)** is the primary hardware peripheral that enables external memory access on the nRF54H20 SoC.
It provides a high-speed interface to external memory devices such as QSPI/Octal-SPI flash or MRAM, enabling both data storage and execute-in-place (XiP) functionality.

EXMIF allows execute-in-place (XiP) access to external quad/octal serial memory, which means that code can be executed directly from external memory without the need to copy it to internal RAM first.

Key Features
************

Hardware Capabilities
=====================

* **Base peripheral**: Synopsys DesignWare SSI controller (``snps,designware-ssi``)
* **Number of devices**: Supports up to 2 external memory devices
* **Maximum device size**: 256 MB (0x10000000 bytes) per device
* **Clock frequency**: 400 MHz (as configured in device tree)
* **Interface modes**: Multi-bit SPI (MSPI) - Quad and Octal SPI
* **FIFO depth**: 32 entries
* **Packet data limit**: 65536 bytes

Multi-Core Support
==================

EXMIF is available across multiple processor cores and their XiP variants:

* Application Core (cpuapp)
* Radio Core (cpurad)
* Fast Lightweight Processor (cpuflpr)
* Peripheral Processor (cpuppr)
* All XiP variants of the above cores

Hardware Configuration
**********************

Device Tree Definition
======================

The EXMIF peripheral is defined in the nRF54H20 device tree as follows:

.. code-block:: dts

   exmif: exmif@95000 {
       compatible = "nordic,nrf-exmif", "snps,designware-ssi";
       #address-cells = <1>;
       #size-cells = <0>;
       reg = <0x95000 0x500 0x95500 0xb00>;
       reg-names = "wrapper", "core";
       interrupts = <149 NRF_DEFAULT_IRQ_PRIORITY>;
       clock-frequency = <DT_FREQ_M(400)>;
       packet-data-limit = <65536>;
       fifo-depth = <32>;
       status = "disabled";
   };

Configuration Registers
=======================

EXMIF provides configuration for each of the two external memory devices through dedicated registers:

**EXTCONF1 Register**
   Configures the first external memory device:
   
   * Address offset in the memory map
   * Size of the memory region
   * Enable/disable status

**EXTCONF2 Register**
   Configures the second external memory device with the same parameters as EXTCONF1.

GPIO Configuration Requirement
===============================

.. important::
   Access to external memory (EXMIF) requires a non-default configuration of the ``GPIO.CTRLSEL`` register.
   This must be configured through the UICR.PERIPHCONF mechanism provided by IronSide SE.

Performance Enhancement: L2CACHE
********************************

The **L2CACHE** (Level 2 Cache) is a global hardware peripheral that significantly enhances external memory performance on the nRF54H20.

How L2CACHE Works
=================

The L2CACHE is a global level 2 cache that accelerates accesses to slower global memories, including:

* **MRAM** (internal flash memory)
* **EXMIF** (external memory via the External Memory Interface)

Cache Characteristics
=====================

* **Status**: Enabled by default
* **Management**: Requires software management for coherency
* **Purpose**: Reduces latency and improves throughput for memory-mapped external memory operations

Performance Impact
==================

When code executes from external memory (XiP mode), the L2CACHE dramatically improves performance by:

#. **Reducing external memory accesses**: Caches frequently accessed data and code
#. **Providing fast cached access**: Serves repeated accesses from the cache instead of external memory
#. **Minimizing latency**: Reduces the impact of slower external memory access times
#. **Improving throughput**: Allows burst accesses and prefetching

Security Features
*****************

The EXMIF system includes several security features to protect external memory access:

Memory Protection Controller (MPC)
===================================

* Classifies memory regions as secure or non-secure
* Configured through the system Memory Protection Controller
* Allows fine-grained control over memory region access

System Protection Unit (SPU)
=============================

* Enforces secure access settings
* Works in conjunction with the MPC
* Ensures secure/non-secure boundaries are maintained

UICR Configuration
==================

External memory security attributes can be configured through:

* **UICR.MPCCONF**: Memory protection configuration entries
* **UICR.PERIPHCONF**: Peripheral configuration including GPIO.CTRLSEL for EXMIF access

Execute-in-Place (XiP) Support
*******************************

XiP Functionality
=================

Execute-in-Place allows code to run directly from external memory without copying it to RAM. This provides:

* **Increased code space**: Applications can exceed internal flash limitations
* **Reduced RAM usage**: Code remains in external memory instead of being copied to RAM
* **Flexible memory organization**: Mix of internal and external code execution

XiP Targets
===========

The nRF54H20 supports XiP execution on multiple cores:

* ``nrf54h20dk/nrf54h20/cpuapp/xip``
* ``nrf54h20dk/nrf54h20/cpurad/xip``
* ``nrf54h20dk/nrf54h20/cpuflpr/xip``
* ``nrf54h20dk/nrf54h20/cpuppr/xip``

Usage Modes
===========

**RAM Execution Mode**
   Standard mode where code runs from internal RAM. External memory used for data storage only.

**XiP Execution Mode**
   Code executes directly from external memory through the EXMIF interface, accelerated by L2CACHE.

MSPI Driver Integration
************************

The EXMIF peripheral is accessed through the MSPI (Multi-bit SPI) driver in Zephyr, which provides:

Configuration API
=================

* ``mspi_dev_config()``: Configure device parameters (frequency, I/O mode)
* ``mspi_transceive()``: Perform data transfers
* ``device_is_ready()``: Check device readiness

Transfer Modes
==============

* **MSPI_IO_MODE_SINGLE**: Single-bit SPI mode
* **MSPI_IO_MODE_QUAD**: Quad SPI mode (4 data lines)
* **MSPI_IO_MODE_OCTAL**: Octal SPI mode (8 data lines)

Power Management
================

* Runtime power management support via ``pm_device_runtime_get()`` and ``pm_device_runtime_put()``
* Allows external memory to enter low-power states when not in use

Configuration and Setup
***********************

Enabling EXMIF
==============

To use EXMIF in your application:

#. Enable the EXMIF peripheral in device tree (set ``status = "okay"``)
#. Configure GPIO pins for MSPI through pinctrl
#. Set up UICR.PERIPHCONF to configure GPIO.CTRLSEL registers
#. Initialize the MSPI driver in your application code

Example MSPI Configuration
===========================

.. code-block:: c

   #include <zephyr/drivers/mspi.h>
   
   #define MSPI_BUS DT_BUS(DT_ALIAS(external_memory))
   static const struct device *controller = DEVICE_DT_GET(MSPI_BUS);
   static struct mspi_dev_id dev_id = MSPI_DEVICE_ID_DT(DT_ALIAS(external_memory));
   
   // Configure for Octal SPI at 1 MHz
   const struct mspi_dev_cfg qspi_transfer_cfg = {
       .freq = MHZ(1),
       .io_mode = MSPI_IO_MODE_OCTAL,
   };
   
   ret = mspi_dev_config(controller, &dev_id,
                         MSPI_DEVICE_CONFIG_FREQUENCY | MSPI_DEVICE_CONFIG_IO_MODE,
                         &qspi_transfer_cfg);

HAL API Reference
=================

The nRFx HAL provides low-level access to EXMIF through:

* ``nrf_exmif_config_t``: Configuration structure
* Enable/disable functions for external devices
* Task trigger functions (start/stop)
* Register access for EXTCONF1 and EXTCONF2

Architecture Integration
*************************

Memory Map
==========

External memory accessed through EXMIF is mapped into the processor's memory space, allowing:

* Direct memory-mapped access
* Execute-in-place for code regions
* DMA transfers to/from external memory

Boot and Initialization
========================

During boot, IronSide SE:

#. Applies UICR.PERIPHCONF settings for GPIO configuration
#. Configures memory protection through UICR.MPCCONF
#. Enables L2CACHE for external memory acceleration
#. Sets up secure/non-secure boundaries

The application then:

#. Initializes the MSPI driver
#. Configures the external memory device
#. Enables XiP if required

Best Practices
**************

Performance Optimization
========================

#. **Use L2CACHE**: Ensure L2CACHE is enabled for best performance
#. **Align code sections**: Align code to cache line boundaries for optimal caching
#. **Minimize random access**: Sequential access patterns perform better
#. **Consider code placement**: Place frequently executed code in internal memory

Power Efficiency
================

#. Use runtime power management to suspend EXMIF when not needed
#. Consider power modes of the external memory device
#. Balance performance vs. power consumption based on application needs

Security Considerations
=======================

#. Configure MPC appropriately for secure/non-secure boundaries
#. Use UICR.PROTECTEDMEM for integrity-checked regions
#. Lock down UICR settings in production with UICR.LOCK
#. Enable access port protection (UICR.APPROTECT) for production devices

Limitations and Considerations
*******************************

Hardware Limitations
====================

* Maximum 2 external memory devices supported
* Each device limited to 256 MB
* Requires specific GPIO pin configuration
* External memory access slower than internal MRAM (mitigated by L2CACHE)

Software Limitations
====================

* L2CACHE requires software coherency management
* XiP code cannot be updated while executing
* Debug access may require special configuration
* Some real-time constraints may not be met from external memory

References
**********

Related Documentation
=====================

* nRF54H20 Product Specification
* IronSide Secure Element documentation (:ref:`ug_nrf54h20_ironside`)
* MSPI driver documentation in Zephyr
* nRFx EXMIF HAL API documentation

Hardware Resources
==================

* EXMIF peripheral registers at address ``0x5f095000``
* L2CACHE for performance acceleration
* GPIO pins for MSPI interface
* Interrupt line 149 for EXMIF events

See Also
========

* :ref:`ug_nrf54h20_architecture_memory` - Memory architecture overview
* :ref:`ug_nrf54h20_ironside` - IronSide SE configuration including UICR.PERIPHCONF
* Zephyr MSPI driver documentation
* Code relocation and XiP samples in nRF Connect SDK

