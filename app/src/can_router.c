#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(can_router, LOG_LEVEL_DBG);

static const struct device *can_device;
static struct k_work_q can_work_q;
static K_THREAD_STACK_DEFINE(can_work_q_stack, 2048);

/* Statistics */
static struct {
    uint32_t rx_count;
    uint32_t tx_count;
    uint32_t filtered_count;
    uint32_t error_count;
} stats;

/* External filter function */
extern bool filter_check_id(uint32_t can_id);

/* Work item for CAN frame processing */
struct can_work_item {
    struct k_work work;
    struct can_frame frame;
    uint32_t timestamp;
};

#define CAN_WORK_POOL_SIZE 10
static struct can_work_item work_pool[CAN_WORK_POOL_SIZE];
static uint8_t work_pool_idx = 0;

/* Process received CAN frame */
static void can_frame_work_handler(struct k_work *work)
{
    struct can_work_item *item = CONTAINER_OF(work, struct can_work_item, work);
    int ret;

    /* Check filter */
    if (!filter_check_id(item->frame.id)) {
        stats.filtered_count++;
        LOG_DBG("Frame filtered: ID 0x%03X", item->frame.id);
        return;
    }

    /* Route/forward the frame */
    ret = can_send(can_device, &item->frame, K_MSEC(100), NULL, NULL);
    if (ret == 0) {
        stats.tx_count++;
        LOG_DBG("Routed frame: ID 0x%03X, DLC %d", item->frame.id, item->frame.dlc);
    } else {
        stats.error_count++;
        LOG_ERR("Failed to route frame: %d", ret);
    }
}

/* CAN RX callback */
static void can_rx_callback(const struct device *dev, struct can_frame *frame,
                           void *user_data)
{
    struct can_work_item *item;

    stats.rx_count++;

    /* Get work item from pool */
    item = &work_pool[work_pool_idx];
    work_pool_idx = (work_pool_idx + 1) % CAN_WORK_POOL_SIZE;

    /* Copy frame data */
    memcpy(&item->frame, frame, sizeof(struct can_frame));
    item->timestamp = k_cycle_get_32();

    /* Submit to work queue for processing */
    k_work_submit_to_queue(&can_work_q, &item->work);
}

/* Initialize CAN router */
void can_router_init(const struct device *can_dev)
{
    can_device = can_dev;

    /* Initialize work queue */
    k_work_queue_init(&can_work_q);
    k_work_queue_start(&can_work_q, can_work_q_stack,
                      K_THREAD_STACK_SIZEOF(can_work_q_stack),
                      K_PRIO_PREEMPT(5), NULL);

    /* Initialize work items */
    for (int i = 0; i < CAN_WORK_POOL_SIZE; i++) {
        k_work_init(&work_pool[i].work, can_frame_work_handler);
    }

    /* Reset statistics */
    memset(&stats, 0, sizeof(stats));

    LOG_INF("CAN router initialized");
}

/* Start CAN routing */
void can_router_start(void)
{
    struct can_filter filter;
    int ret;

    /* Accept all frames - filtering done in software */
    filter.id = 0;
    filter.mask = 0;
    filter.flags = 0;

    ret = can_add_rx_filter(can_device, can_rx_callback, NULL, &filter);
    if (ret < 0) {
        LOG_ERR("Failed to add RX filter: %d", ret);
        return;
    }

    LOG_INF("CAN router started, filter ID: %d", ret);
}

/* Get statistics */
void can_router_get_stats(uint32_t *rx, uint32_t *tx, uint32_t *filtered, uint32_t *errors)
{
    if (rx) *rx = stats.rx_count;
    if (tx) *tx = stats.tx_count;
    if (filtered) *filtered = stats.filtered_count;
    if (errors) *errors = stats.error_count;
}

/* Reset statistics */
void can_router_reset_stats(void)
{
    memset(&stats, 0, sizeof(stats));
    LOG_INF("Statistics reset");
}