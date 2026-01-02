#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stdbool.h>

LOG_MODULE_REGISTER(filter, LOG_LEVEL_DBG);

/* Filter configuration */
static bool whitelist_mode;

/* Filter lists - configurable at runtime via shell */
#define MAX_FILTERS 32
static uint32_t filter_list[MAX_FILTERS];
static uint8_t filter_count = 0;

/* Initialize filter module */
void filter_init(bool is_whitelist)
{
    whitelist_mode = is_whitelist;
    filter_count = 0;

    LOG_INF("Filter initialized: %s mode", 
            whitelist_mode ? "Whitelist" : "Blacklist");
}

/* Add CAN ID to filter list */
bool filter_add_id(uint32_t can_id)
{
    if (filter_count >= MAX_FILTERS) {
        LOG_ERR("Filter list full");
        return false;
    }

    /* Check for duplicates */
    for (int i = 0; i < filter_count; i++) {
        if (filter_list[i] == can_id) {
            LOG_WRN("ID 0x%03X already in filter list", can_id);
            return false;
        }
    }

    filter_list[filter_count++] = can_id;
    LOG_INF("Added ID 0x%03X to %s", can_id, 
            whitelist_mode ? "whitelist" : "blacklist");
    return true;
}

/* Remove CAN ID from filter list */
bool filter_remove_id(uint32_t can_id)
{
    for (int i = 0; i < filter_count; i++) {
        if (filter_list[i] == can_id) {
            /* Shift remaining entries */
            for (int j = i; j < filter_count - 1; j++) {
                filter_list[j] = filter_list[j + 1];
            }
            filter_count--;
            LOG_INF("Removed ID 0x%03X from filter list", can_id);
            return true;
        }
    }
    LOG_WRN("ID 0x%03X not found in filter list", can_id);
    return false;
}

/* Clear all filters */
void filter_clear(void)
{
    filter_count = 0;
    LOG_INF("Filter list cleared");
}

/* Check if CAN ID should be accepted */
bool filter_check_id(uint32_t can_id)
{
    bool found = false;

    /* Search for ID in list */
    for (int i = 0; i < filter_count; i++) {
        if (filter_list[i] == can_id) {
            found = true;
            break;
        }
    }

    /* Whitelist: accept if found, reject if not found */
    /* Blacklist: reject if found, accept if not found */
    if (whitelist_mode) {
        /* Empty whitelist = accept nothing */
        return (filter_count == 0) ? false : found;
    } else {
        /* Empty blacklist = accept everything */
        return (filter_count == 0) ? true : !found;
    }
}

/* Get filter mode */
bool filter_is_whitelist(void)
{
    return whitelist_mode;
}

/* Set filter mode */
void filter_set_mode(bool is_whitelist)
{
    whitelist_mode = is_whitelist;
    LOG_INF("Filter mode changed to: %s", 
            whitelist_mode ? "Whitelist" : "Blacklist");
}

/* Get filter count */
uint8_t filter_get_count(void)
{
    return filter_count;
}

/* Get filter list */
void filter_get_list(uint32_t *list, uint8_t *count)
{
    if (list && count) {
        for (int i = 0; i < filter_count; i++) {
            list[i] = filter_list[i];
        }
        *count = filter_count;
    }
}