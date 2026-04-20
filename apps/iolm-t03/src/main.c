/**
 ******************************************************************************
 * @file    main.c
 * @author  Sanjana S, Calixto Firmware Team
 * @version V2.1.0
 * @date    17-Apr-2026
 * @brief   IO-Link Master - Zephyr entry point.
******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/version.h>

#include <iol_cce4511_pl.h>   
#include <iol_cce4511.h>      
#include <osal_spi.h>         
#include <iol_log.h>         
#include <cce4511_conf.h>   

#include "app_handler.h"      

LOG_MODULE_REGISTER(iol_main, CONFIG_IOL_MASTER_LOG_LEVEL);

#define CCE4511_NODE   DT_NODELABEL(cce4511_0)

static const struct spi_dt_spec cce4511_spi =
    SPI_DT_SPEC_GET(CCE4511_NODE, CCE4511_SPI_OPERATION, 0);

static const struct gpio_dt_spec cce4511_irq_gpio =
    GPIO_DT_SPEC_GET(CCE4511_NODE, irq_gpios);

#define APP_MASTER_THREAD_STACK_SIZE  (4 * 1024)
#define APP_MASTER_THREAD_PRIO        3
#define APP_DL_THREAD_STACK_SIZE      4096
#define APP_DL_THREAD_PRIO            (APP_MASTER_THREAD_PRIO + 1)

K_THREAD_STACK_DEFINE(iolink_task_stack, APP_MASTER_THREAD_STACK_SIZE);
static struct k_thread iolink_task_tcb;

static iolink_pl_mode_t mode_ch[IOLINK_NUM_PORTS] = {
    iolink_mode_SDCI,      /* port 0 - active IO-Link port   */
    iolink_mode_INACTIVE,  /* port 1 - unused                */
};

static iolink_port_cfg_t port_cfgs[IOLINK_NUM_PORTS];
static iolink_m_cfg_t    app_cfg;

static struct gpio_callback cce4511_gpio_cb;

static void gpio_isr_handler(const struct device *port,
                              struct gpio_callback *cb,
                              gpio_port_pins_t pins)
{
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    if (g_iolink_drv != NULL) {
        iolink_cce4511_isr((void *)g_iolink_drv, 0);
    } else {
        LOG_WRN("[GPIO ISR] g_iolink_drv is NULL - IRQ before driver init?");
    }
}

static int board_hw_init(void)
{
    if (!spi_is_ready_dt(&cce4511_spi)) {
        LOG_ERR("CCE4511 SPI bus not ready - check DTS overlay");
        return -ENODEV;
    }
                                              
    iol_spi_set_device(&cce4511_spi);

    if (!gpio_is_ready_dt(&cce4511_irq_gpio)) {
        LOG_ERR("CCE4511 IRQ GPIO not ready - check DTS irq-gpios property");
        return -ENODEV;
    }

    int rc = gpio_pin_configure_dt(&cce4511_irq_gpio, GPIO_INPUT);
    if (rc != 0) {
        LOG_ERR("gpio_pin_configure_dt(IRQ) failed: %d", rc);
        return rc;
    }

    rc = gpio_pin_interrupt_configure_dt(&cce4511_irq_gpio,
                                         GPIO_INT_EDGE_FALLING);
    if (rc != 0) {
        LOG_ERR("gpio_pin_interrupt_configure_dt failed: %d", rc);
        return rc;
    }

    gpio_init_callback(&cce4511_gpio_cb,
                       gpio_isr_handler,
                       BIT(cce4511_irq_gpio.pin));

    rc = gpio_add_callback(cce4511_irq_gpio.port, &cce4511_gpio_cb);
    if (rc != 0) {
        LOG_ERR("gpio_add_callback failed: %d", rc);
        return rc;
    }

    LOG_INF("CCE4511 hardware initialised - IRQ GPIO pin %d",
            cce4511_irq_gpio.pin);
    return 0;
}

/* ---------------------------------------------------------------------------
 * IO-Link application thread
 * ---------------------------------------------------------------------------*/
static void iolink_task(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    k_msleep(200);

    port_cfgs[0].mode     = &mode_ch[0];
    port_cfgs[0].arg      = (void *)0;
    port_cfgs[0].cyc_time = 0xBF;

    port_cfgs[1].mode     = &mode_ch[1];
    port_cfgs[1].arg      = (void *)1;
    port_cfgs[1].cyc_time = 0;

    app_cfg.port_cnt                 = 1;
    app_cfg.port_cfgs                = port_cfgs;
    app_cfg.master_thread_prio       = APP_MASTER_THREAD_PRIO;
    app_cfg.master_thread_stack_size = APP_MASTER_THREAD_STACK_SIZE;
    app_cfg.dl_thread_prio           = APP_DL_THREAD_PRIO;
    app_cfg.dl_thread_stack_size     = APP_DL_THREAD_STACK_SIZE;
    app_cfg.cb_arg                   = NULL;
    app_cfg.user_cb                  = NULL;

    app_handler(app_cfg);
}

/* ---------------------------------------------------------------------------
 * main()
 * ---------------------------------------------------------------------------*/
int main(void)
{
    LOG_INF("IO-Link Master starting (Zephyr %s)", KERNEL_VERSION_STRING);

    int rc = board_hw_init();
    if (rc != 0) {
        LOG_ERR("board_hw_init() failed (%d) - halting", rc);
        while (1) {
            k_sleep(K_SECONDS(1));
        }
    }

    k_thread_create(&iolink_task_tcb,
                    iolink_task_stack,
                    K_THREAD_STACK_SIZEOF(iolink_task_stack),
                    iolink_task,
                    NULL, NULL, NULL,
                    APP_MASTER_THREAD_PRIO,
                    0,
                    K_NO_WAIT);

    k_thread_name_set(&iolink_task_tcb, "iolink_app");

    return 0;
}