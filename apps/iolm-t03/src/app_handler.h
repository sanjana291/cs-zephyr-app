/**
 ******************************************************************************
 * @file    app_handler.h
 * @author  Sanjana S, Calixto Firmware Team
 * @version V2.0.0
 * @date    30-Mar-2026
 * @brief   IO-Link application handler — public types and API.
 *          Zephyr port: all FreeRTOS / NXP-SDK dependencies removed.
 *          Uses only the OSAL abstraction layer (osal.h) and the
 *          IO-Link stack public headers.
 *
 * <h2><center>&copy; COPYRIGHT 2026 Calixto Systems Pvt Ltd</center></h2>
 ******************************************************************************
 */

#ifndef APP_HANDLER_H
#define APP_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include <iol.h>       /* iolink_m_cfg_t, iolink_m_t, IOLINK_PD_MAX_SIZE    */
#include <iol_api.h>   /* Master_init(), Master_setPortMode(), PortMode       */
#include <osal.h>      /* os_event_t                                          */

/* ---------------------------------------------------------------------------
 * Configuration constants
 * ---------------------------------------------------------------------------*/

/** Maximum number of device events stored per port. */
#define PORT_EVENT_COUNT  10

/* ---------------------------------------------------------------------------
 * Types
 * ---------------------------------------------------------------------------*/

/**
 * @brief Per-port application context.
 *
 * Holds the latest process-data snapshot and a small event/diagnostic ring
 * that the middleware fills via app_user_cb().
 */
typedef struct app_port_ctx
{
    /** 1-based port number (matches the IO-Link stack convention). */
    uint8_t portnumber;

    /** Diagnostic / event log received from the device. */
    struct
    {
        uint8_t              count;
        diag_entry_t         diag_entry[PORT_EVENT_COUNT];
        iolink_eventcode_t   eventcode[PORT_EVENT_COUNT];
    } events;

    /** Latest PDIn snapshot copied inside app_user_cb(). */
    struct
    {
        uint8_t data[IOLINK_PD_MAX_SIZE];
        uint8_t data_len;
    } pdin;

} app_port_ctx_t;

/**
 * @brief Master-level application context.
 *
 * One instance is held as a file-scope variable inside app_handler.c.
 * It groups the master handle, all port contexts, and the shared event
 * object used to wake the application loop.
 */
typedef struct app_master_ctx
{
    /** Handle returned by Master_init(). */
    iolink_m_t     *master;

    /** Per-port contexts, indexed 0 … (IOLINK_NUM_PORTS - 1). */
    app_port_ctx_t  app_port[IOLINK_NUM_PORTS];

    /** OSAL event object — bits are set by app_user_cb() to wake the loop. */
    os_event_t     *app_event;

} app_master_ctx_t;

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/

/**
 * @brief Application entry point for the IO-Link master.
 *
 * Initialises the master stack, activates ports, then enters the event
 * loop.  This function never returns.
 *
 * @param m_cfg   Master configuration (port count, thread priorities, etc.).
 *                The cb_arg and user_cb fields are overwritten inside this
 *                function before Master_init() is called.
 */
void app_handler(iolink_m_cfg_t m_cfg);

#ifdef __cplusplus
}
#endif

#endif /* APP_HANDLER_H */