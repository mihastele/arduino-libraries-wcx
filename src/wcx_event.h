#ifndef WCX_EVENT_H
#define WCX_EVENT_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Event identifier type — small integer.
     */
    typedef uint8_t wcx_event_id_t;

    /**
     * A queued event: id plus a small payload.
     */
    typedef struct
    {
        wcx_event_id_t id;
        uint8_t data;
    } wcx_event_t;

    /**
     * Subscriber callback.  Receives the event and an opaque context pointer.
     */
    typedef void (*wcx_event_handler_t)(wcx_event_t event, void *context);

    /**
     * A single subscription record.
     */
    typedef struct
    {
        wcx_event_id_t event_id;
        wcx_event_handler_t handler;
        void *context;
        bool active;
    } wcx_subscription_t;

    /**
     * Event queue with pub/sub dispatch.  All storage is caller-owned.
     */
    typedef struct
    {
        wcx_event_t *queue;
        size_t queue_capacity;
        size_t queue_head;
        size_t queue_tail;
        size_t queue_count;
        wcx_subscription_t *subs;
        size_t subs_capacity;
        size_t subs_count;
    } wcx_event_bus_t;

    bool wcx_event_bus_init(wcx_event_bus_t *bus,
                            wcx_event_t *queue_storage, size_t queue_capacity,
                            wcx_subscription_t *subs_storage, size_t subs_capacity);

    bool wcx_event_subscribe(wcx_event_bus_t *bus, wcx_event_id_t event_id,
                             wcx_event_handler_t handler, void *context);

    bool wcx_event_publish(wcx_event_bus_t *bus, wcx_event_id_t event_id,
                           uint8_t data);

    size_t wcx_event_dispatch(wcx_event_bus_t *bus);

    size_t wcx_event_pending(const wcx_event_bus_t *bus);

#ifdef __cplusplus
}
#endif

#endif /* WCX_EVENT_H */
