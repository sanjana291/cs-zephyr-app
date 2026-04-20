/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * src/osal/osal_irq_stub.c — Stub for the OSAL IRQ helper.
 *
 * WHY THIS FILE EXISTS
 * ────────────────────
 * osal_irq.h declares _iolink_setup_int(), a helper from the original
 * NXP/FreeRTOS port that registered a GPIO interrupt via a platform-
 * specific mechanism.
 *
 * In the Zephyr port, GPIO interrupt setup is done directly in main.c
 * using the Zephyr GPIO API (gpio_pin_interrupt_configure_dt +
 * gpio_add_callback).  _iolink_setup_int() is therefore never called.
 *
 * However, osal_irq.h is still #included by iol_cce4511_pl.c (legacy
 * include left over from the original driver).  As long as the function
 * is never called the linker does not need a definition — but providing
 * this stub makes the dependency explicit, avoids future "undefined
 * reference" surprises if the call site is accidentally un-commented,
 * and documents the architectural decision clearly.
 *
 * LONG-TERM RECOMMENDATION
 * ────────────────────────
 * Remove the #include "osal_irq.h" line from iol_cce4511_pl.c and from
 * main.c, then delete this file and remove it from CMakeLists.txt.
 * The Zephyr port has no need for _iolink_setup_int().
 */

#include "osal_irq.h"
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(iol_osal, CONFIG_IOL_MASTER_LOG_LEVEL);

int _iolink_setup_int(int gpio_pin, isr_func_t isr_func, void *irq_arg)
{
    /*
     * Not implemented in the Zephyr port.
     * GPIO interrupt setup is handled in main.c via:
     *   gpio_pin_interrupt_configure_dt()
     *   gpio_add_callback()
     */
    ARG_UNUSED(gpio_pin);
    ARG_UNUSED(isr_func);
    ARG_UNUSED(irq_arg);

    LOG_ERR("_iolink_setup_int() called — this function is not implemented "
            "in the Zephyr port.  Use the Zephyr GPIO API directly.");
    return -ENOSYS;
}
