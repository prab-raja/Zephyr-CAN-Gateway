#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

LOG_MODULE_REGISTER(uart_diag, LOG_LEVEL_DBG);

/* External functions */
extern void can_router_get_stats(uint32_t *rx, uint32_t *tx, uint32_t *filtered, uint32_t *errors);
extern void can_router_reset_stats(void);
extern bool filter_add_id(uint32_t can_id);
extern bool filter_remove_id(uint32_t can_id);
extern void filter_clear(void);
extern bool filter_is_whitelist(void);
extern void filter_set_mode(bool is_whitelist);
extern uint8_t filter_get_count(void);
extern void filter_get_list(uint32_t *list, uint8_t *count);

/* Shell command: show statistics */
static int cmd_stats(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t rx, tx, filtered, errors;
    
    can_router_get_stats(&rx, &tx, &filtered, &errors);
    
    shell_print(sh, "CAN Gateway Statistics:");
    shell_print(sh, "  RX Frames:      %u", rx);
    shell_print(sh, "  TX Frames:      %u", tx);
    shell_print(sh, "  Filtered:       %u", filtered);
    shell_print(sh, "  Errors:         %u", errors);
    
    return 0;
}

/* Shell command: reset statistics */
static int cmd_stats_reset(const struct shell *sh, size_t argc, char **argv)
{
    can_router_reset_stats();
    shell_print(sh, "Statistics reset");
    return 0;
}

/* Shell command: show filter configuration */
static int cmd_filter_show(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t list[32];
    uint8_t count;
    
    shell_print(sh, "Filter Mode: %s", 
                filter_is_whitelist() ? "Whitelist" : "Blacklist");
    
    filter_get_list(list, &count);
    shell_print(sh, "Filter Count: %u", count);
    
    if (count > 0) {
        shell_print(sh, "Filtered IDs:");
        for (int i = 0; i < count; i++) {
            shell_print(sh, "  0x%03X (%u)", list[i], list[i]);
        }
    }
    
    return 0;
}

/* Shell command: add filter ID */
static int cmd_filter_add(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t can_id;
    
    if (argc < 2) {
        shell_error(sh, "Usage: filter add <CAN_ID>");
        return -1;
    }
    
    can_id = strtoul(argv[1], NULL, 0);
    
    if (filter_add_id(can_id)) {
        shell_print(sh, "Added ID 0x%03X to filter", can_id);
    } else {
        shell_error(sh, "Failed to add ID");
    }
    
    return 0;
}

/* Shell command: remove filter ID */
static int cmd_filter_remove(const struct shell *sh, size_t argc, char **argv)
{
    uint32_t can_id;
    
    if (argc < 2) {
        shell_error(sh, "Usage: filter remove <CAN_ID>");
        return -1;
    }
    
    can_id = strtoul(argv[1], NULL, 0);
    
    if (filter_remove_id(can_id)) {
        shell_print(sh, "Removed ID 0x%03X from filter", can_id);
    } else {
        shell_error(sh, "Failed to remove ID");
    }
    
    return 0;
}

/* Shell command: clear all filters */
static int cmd_filter_clear(const struct shell *sh, size_t argc, char **argv)
{
    filter_clear();
    shell_print(sh, "All filters cleared");
    return 0;
}

/* Shell command: set filter mode */
static int cmd_filter_mode(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 2) {
        shell_error(sh, "Usage: filter mode <whitelist|blacklist>");
        return -1;
    }
    
    if (strcmp(argv[1], "whitelist") == 0) {
        filter_set_mode(true);
        shell_print(sh, "Filter mode set to: Whitelist");
    } else if (strcmp(argv[1], "blacklist") == 0) {
        filter_set_mode(false);
        shell_print(sh, "Filter mode set to: Blacklist");
    } else {
        shell_error(sh, "Invalid mode. Use 'whitelist' or 'blacklist'");
        return -1;
    }
    
    return 0;
}

/* Register shell commands */
SHELL_STATIC_SUBCMD_SET_CREATE(filter_cmds,
    SHELL_CMD(show, NULL, "Show filter configuration", cmd_filter_show),
    SHELL_CMD(add, NULL, "Add CAN ID to filter", cmd_filter_add),
    SHELL_CMD(remove, NULL, "Remove CAN ID from filter", cmd_filter_remove),
    SHELL_CMD(clear, NULL, "Clear all filters", cmd_filter_clear),
    SHELL_CMD(mode, NULL, "Set filter mode (whitelist/blacklist)", cmd_filter_mode),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(stats_cmds,
    SHELL_CMD(show, NULL, "Show statistics", cmd_stats),
    SHELL_CMD(reset, NULL, "Reset statistics", cmd_stats_reset),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(stats, &stats_cmds, "CAN gateway statistics", NULL);
SHELL_CMD_REGISTER(filter, &filter_cmds, "CAN filter management", NULL);

/* Initialize UART diagnostics */
void uart_diagnostics_init(void)
{
    LOG_INF("UART diagnostics initialized");
    LOG_INF("Available commands:");
    LOG_INF("  stats show          - Display statistics");
    LOG_INF("  stats reset         - Reset statistics");
    LOG_INF("  filter show         - Show filter config");
    LOG_INF("  filter add <ID>     - Add CAN ID");
    LOG_INF("  filter remove <ID>  - Remove CAN ID");
    LOG_INF("  filter clear        - Clear all filters");
    LOG_INF("  filter mode <mode>  - Set whitelist/blacklist");
}