/**
 ******************************************************************************
 * @file    app_handler.c
 * @author  Sanjana S, Calixto Firmware Team
 * @version V2.0.0
 * @date    30-Mar-2026
 * @brief   IO-Link application handler — implementation.
 * <h2><center>&copy; COPYRIGHT 2026 Calixto Systems Pvt Ltd</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/kernel.h>

#include <iol_log.h>   
#include <iol.h>       
#include <iol_main.h>  
#include <iol_mw.h>    
#include <iol_api.h>   
#include <osal.h>     

#include "app_handler.h"

#define APP_PD_EVENT       BIT(0)
#define APP_OPERATE_EVENT  BIT(4)
#define APP_COMLOST_EVENT  BIT(8)
#define APP_EVENTS_EVENT   BIT(12)

#define APP_ALL_EVENTS     0xFFFFFFFFu

static app_master_ctx_t app_master;

static bool device_ident_read[IOLINK_NUM_PORTS];

static void read_device_parameters(app_port_ctx_t *app_port)
{
    uint8_t        data[64];
    uint8_t        len;
    iolink_error_t err;

    for (uint16_t index = 17U; index <= 26U; index++)
    {
        memset(data, 0, sizeof(data));
        len = sizeof(data);

        err = Master_readParameter((uint8_t)(app_port->portnumber + 1U),
                                   index, 0U, data, &len);
        if (err == IOLINK_ERROR_NONE)
        {
            printk("\r\nIndex 0x%X : ", index);
            for (uint8_t i = 0U; i < len; i++)
            {
                printk("%c", data[i]);
            }
            printk("\r\n\n");
        }
        else
        {
            printk("Index 0x%X read failed: %d\r\n", index, (int)err);
            break;
        }
        
        os_usleep(50000U);
    }

    memset(data, 0, sizeof(data));
    data[0] = 0x00U;   /* 0 = mm, 1 = inch */

    err = Master_writeParameter((uint8_t)(app_port->portnumber + 1U),
                                0x0114U, 0U, data, 1U);
    if (err != IOLINK_ERROR_NONE)
    {
        printk("Index 0x0114 (Measurement Unit) write failed, err=0x%04X\r\n",
               (unsigned int)err);
        return;
    }
    printk("Index 0x0114 (Measurement Unit) write OK\r\n");

    os_usleep(50000U);

    memset(data, 0, sizeof(data));
    len = sizeof(data);

    err = Master_readParameter((uint8_t)(app_port->portnumber + 1U),
                               0x0114U, 0U, data, &len);
    if (err != IOLINK_ERROR_NONE)
    {
        printk("Index 0x0114 read back failed, err=0x%04X\r\n",
               (unsigned int)err);
        return;
    }
    printk("Index 0x0114 (Measurement Unit) readback: %s\r\n",
           (data[0] == 0U) ? "Millimeter" : "Inch");
}

static void app_user_cb(
    uint8_t          portnumber,
    void            *arg,
    IOL_data_ind     data_type,
    uint8_t          data_len,
    const uint8_t   *inputdata)
{
    uint8_t            port_index = (uint8_t)(portnumber - 1U);
    app_master_ctx_t  *app_m      = (app_master_ctx_t *)arg;
    app_port_ctx_t    *app_port   = &app_m->app_port[port_index];

    /* Guard against out-of-range port numbers. */
    if (port_index >= IOLINK_NUM_PORTS)
    {
        IOL_LOG_ERR("app_user_cb: invalid portnumber %u", portnumber);
        return;
    }

    switch (data_type)
    {
        case PD_DATA_IND:
            if (data_len != 0U && inputdata != NULL)
            {
                memcpy(app_port->pdin.data, inputdata, data_len);
                app_port->pdin.data_len = data_len;
                os_event_set(app_m->app_event,
                             APP_PD_EVENT << port_index);
            }
            break;

        case PORT_OPERATE_IND:
            device_ident_read[port_index] = false;
            os_event_set(app_m->app_event,
                         APP_OPERATE_EVENT << port_index);
            break;

        case PORT_COMLOST_IND:
            IOL_LOG_WRN("port (%u): COMLOST — device disconnected", portnumber);
            device_ident_read[port_index] = false;
            os_event_set(app_m->app_event,
                         APP_COMLOST_EVENT << port_index);
            break;

        case EVENT_DATA_IND:
            os_event_set(app_m->app_event,
                         APP_EVENTS_EVENT << port_index);
            break;

        default:
            IOL_LOG_WRN("app_user_cb: unknown data_type %d", (int)data_type);
            break;
    }
}

void app_handler(iolink_m_cfg_t m_cfg)
{
    uint8_t port_index;

    memset(&app_master, 0, sizeof(app_master));
    memset(device_ident_read, 0, sizeof(device_ident_read));

    for (port_index = 0U; port_index < m_cfg.port_cnt; port_index++)
    {
        app_master.app_port[port_index].portnumber = port_index;
    }

    app_master.app_event = os_event_create();
    CC_ASSERT(app_master.app_event != NULL);

    m_cfg.cb_arg   = &app_master;
    m_cfg.user_cb  = app_user_cb;

    iolink_m_t *master = Master_init(&m_cfg);
    if (master == NULL)
    {
        IOL_LOG_ERR("%s: Master_init() failed!", __func__);
        CC_ASSERT(0);
    }
    app_master.master = master;

    for (port_index = 0U; port_index < m_cfg.port_cnt; port_index++)
    {
        iolink_error_t err = Master_ConfigPDFilter(
            (uint8_t)(port_index + 1U), 0U, 2U);
        if (err != IOLINK_ERROR_NONE)
        {
            IOL_LOG_WRN("Port %u: Master_ConfigPDFilter failed (%d)",
                        (unsigned)(port_index + 1U), (int)err);
        }

        err = Master_setPortMode((uint8_t)(port_index + 1U), PORT_ACTIVE);
        if (err != IOLINK_ERROR_NONE)
        {
            IOL_LOG_WRN("Port %u: Master_setPortMode(ACTIVE) failed (%d)",
                        (unsigned)(port_index + 1U), (int)err);
        }
    }

    IOL_LOG_INF("app_handler: master started, %u port(s) active",
                (unsigned)m_cfg.port_cnt);

    while (1)
    {
        uint32_t event_value = 0U;

        if (!os_event_wait(app_master.app_event,
                           APP_ALL_EVENTS, &event_value, 1000U))
        {
            os_event_clr(app_master.app_event, event_value);

            for (port_index = 0U; port_index < m_cfg.port_cnt; port_index++)
            {
                app_port_ctx_t *app_port = &app_master.app_port[port_index];

                if ((event_value & (APP_OPERATE_EVENT << port_index)) != 0U)
                {
                    IOL_LOG_INF("Port %u: OPERATE — reading device parameters",
                                (unsigned)(port_index + 1U));

                    read_device_parameters(app_port);
                    device_ident_read[port_index] = true;

                    uint8_t        pd_out[1] = {0x00U};
                    iolink_error_t pd_err    = Master_PDout(
                        (uint8_t)(port_index + 1U),
                        true,          /* pd_valid = output is valid         */
                        pd_out,
                        sizeof(pd_out));
                    if (pd_err != IOLINK_ERROR_NONE)
                    {
                        IOL_LOG_ERR("Port %u: Master_PDout failed: %d",
                                    (unsigned)(port_index + 1U), (int)pd_err);
                    }
                }

                if ((event_value & (APP_COMLOST_EVENT << port_index)) != 0U)
                {
                    IOL_LOG_WRN("Port %u: COMLOST — device disconnected, "
                                "waiting for reconnect",
                                (unsigned)(port_index + 1U));
                }

                if ((event_value & (APP_PD_EVENT << port_index)) != 0U)
                {

                    static uint32_t pd_counter[IOLINK_NUM_PORTS];

                    pd_counter[port_index]++;
                    if ((pd_counter[port_index] % 250U) == 0U)
                    {
                        printk("Port %u PD Data (%u bytes): ",
                                    (unsigned)(port_index + 1U),
                                    (unsigned)app_port->pdin.data_len);

                        for (uint8_t i = 0U; i < app_port->pdin.data_len; i++)
                        {
                            printk("%d ", app_port->pdin.data[i]);
                        }
                        printk("\r\n");
                    }
                }
            } 
        }

    } 
}