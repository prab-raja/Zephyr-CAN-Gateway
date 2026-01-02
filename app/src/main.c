#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* Compile-time configuration */
#ifndef CAN_CONTROLLER
#define CAN_CONTROLLER 1
#endif

#ifndef USE_CANFD
#define USE_CANFD 0
#endif

#ifndef FILTER_MODE_WHITELIST
#define FILTER_MODE_WHITELIST 1
#endif

/* Device selection based on compiler flags */
#if CAN_CONTROLLER == 1
#define CAN_DEV_NODE DT_ALIAS(can0)
#elif CAN_CONTROLLER == 2
#define CAN_DEV_NODE DT_ALIAS(can1)
#else
#error "Invalid CAN_CONTROLLER value. Must be 1 or 2"
#endif

/* External function declarations */
extern void can_router_init(const struct device *can_dev);
extern void uart_diagnostics_init(void);
extern void filter_init(bool whitelist_mode);
extern void can_router_start(void);

/* Configuration info */
static void print_config(void)
{
    LOG_INF("========================================");
    LOG_INF("Zephyr CAN Gateway - Configuration");
    LOG_INF("========================================");
    LOG_INF("CAN Controller: %d", CAN_CONTROLLER);
    LOG_INF("CAN-FD Support: %s", USE_CANFD ? "Enabled" : "Disabled");
    LOG_INF("Filter Mode: %s", FILTER_MODE_WHITELIST ? "Whitelist" : "Blacklist");
    LOG_INF("Zephyr Version: %s", KERNEL_VERSION_STRING);
    LOG_INF("========================================");
}

int main(void)
{
    const struct device *can_dev;
    int ret;

    LOG_INF("Starting Zephyr CAN Gateway...");
    
    print_config();

    /* Get CAN device */
    can_dev = DEVICE_DT_GET(CAN_DEV_NODE);
    if (!device_is_ready(can_dev)) {
        LOG_ERR("CAN device not ready");
        return -1;
    }

    LOG_INF("CAN device: %s ready", can_dev->name);

#if USE_CANFD
    /* Set CAN-FD mode */
    ret = can_set_mode(can_dev, CAN_MODE_FD);
    if (ret != 0) {
        LOG_ERR("Failed to set CAN-FD mode: %d", ret);
        return -1;
    }
    LOG_INF("CAN-FD mode enabled");
#else
    /* Set normal CAN mode */
    ret = can_set_mode(can_dev, CAN_MODE_NORMAL);
    if (ret != 0) {
        LOG_ERR("Failed to set CAN mode: %d", ret);
        return -1;
    }
    LOG_INF("Classic CAN mode enabled");
#endif

    /* Start CAN interface */
    ret = can_start(can_dev);
    if (ret != 0) {
        LOG_ERR("Failed to start CAN: %d", ret);
        return -1;
    }

    /* Initialize filter with compile-time mode */
    filter_init(FILTER_MODE_WHITELIST);

    /* Initialize UART diagnostics */
    uart_diagnostics_init();

    /* Initialize and start CAN router */
    can_router_init(can_dev);
    can_router_start();

    LOG_INF("CAN Gateway initialized successfully");
    LOG_INF("Ready to route CAN messages");

    /* Main loop - keep alive */
    while (1) {
        k_sleep(K_SECONDS(10));
        LOG_DBG("Gateway running...");
    }

    return 0;
}