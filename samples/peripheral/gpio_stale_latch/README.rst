.. _gpio_stale_latch_sample:

GPIO stale latch
################

.. contents::
   :local:
   :depth: 2

The sample demonstrates that a GPIO callback can be reported after the pin interrupt was disabled with ``gpio_pin_interrupt_configure(..., GPIO_INT_DISABLE)``.

Overview
********

On SoCs with the GPIO ``LATCH`` register, level interrupts are detected with the pin ``SENSE`` mechanism.
Disabling the interrupt sets ``SENSE`` to disabled, but it does not clear the ``LATCH`` bit.
If the level was latched right before the interrupt was disabled, the GPIOTE PORT event handler still finds the latched pin later and calls the GPIO callback.
Modules that use a level interrupt for wake-up and then switch to another state, for example a button or a wheel driver, may receive such an unexpected callback.

The sample runs two tests:

* Self-triggered test on the **LED1** pin, configured as both input and output.
  With interrupts masked, the sample drives the pin high so that the level is latched, drives it back low, and disables the pin interrupt.
  After interrupts are unmasked, the callback is still reported.
  A control run keeps the interrupt enabled to show that the trigger works.
* Interactive test on **Button 0**.
  The button interrupt is armed and interrupts are masked for :kconfig:option:`CONFIG_SAMPLE_BUTTON_WINDOW_MS`.
  If you press the button within that window, the callback is reported even though the interrupt was disabled before unmasking.

Set the :kconfig:option:`CONFIG_SAMPLE_CLEAR_LATCH` Kconfig option to clear the pin ``LATCH`` bit after disabling the interrupt.
With the workaround enabled, no stale callback is reported.

Requirements
************

The sample supports the following development kit:

* nRF54L15 DK (``nrf54l15dk/nrf54l15/cpuapp``)

Building and running
********************

.. |sample path| replace:: :file:`samples/peripheral/gpio_stale_latch`

.. include:: /includes/build_and_run.txt

Testing
=======

1. Connect to the kit with a terminal emulator (115200 baud).
#. Reset the kit and observe the self-triggered test results:

   .. code-block:: console

      self-trigger (LED1 pin), control, kept enabled: valid callbacks: 10, stale callbacks: 0 (of 10)
      self-trigger (LED1 pin), disabled before handling: valid callbacks: 0, stale callbacks: 10 (of 10)

#. When prompted, press **Button 0** within the window and observe that a stale callback is reported.
#. Build with :kconfig:option:`CONFIG_SAMPLE_CLEAR_LATCH` set to ``y`` and observe that no stale callback is reported.
