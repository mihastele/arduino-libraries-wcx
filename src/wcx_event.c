#include "wcx_event.h"

bool wcx_event_bus_init(wcx_event_bus_t *bus,
                        wcx_event_t *queue_storage, size_t queue_capacity,
                        wcx_subscription_t *subs_storage, size_t subs_capacity)
{
    if (bus == NULL || queue_storage == NULL || queue_capacity == 0U ||
        subs_storage == NULL || subs_capacity == 0U)
    {
        return false;
    }
    bus->queue = queue_storage;
    bus->queue_capacity = queue_capacity;
    bus->queue_head = 0U;
    bus->queue_tail = 0U;
    bus->queue_count = 0U;
    bus->subs = subs_storage;
    bus->subs_capacity = subs_capacity;
    bus->subs_count = 0U;
    for (size_t i = 0U; i < subs_capacity; i++)
    {
        subs_storage[i].active = false;
    }
    return true;
}

bool wcx_event_subscribe(wcx_event_bus_t *bus, wcx_event_id_t event_id,
                         wcx_event_handler_t handler, void *context)
{
    if (bus == NULL || handler == NULL)
    {
        return false;
    }
    if (bus->subs_count >= bus->subs_capacity)
    {
        return false;
    }
    wcx_subscription_t *s = &bus->subs[bus->subs_count];
    s->event_id = event_id;
    s->handler = handler;
    s->context = context;
    s->active = true;
    bus->subs_count++;
    return true;
}

bool wcx_event_publish(wcx_event_bus_t *bus, wcx_event_id_t event_id,
                       uint8_t data)
{
    if (bus == NULL)
    {
        return false;
    }
    if (bus->queue_count >= bus->queue_capacity)
    {
        return false;
    }
    wcx_event_t *slot = &bus->queue[bus->queue_tail];
    slot->id = event_id;
    slot->data = data;
    bus->queue_tail = (bus->queue_tail + 1U) % bus->queue_capacity;
    bus->queue_count++;
    return true;
}

size_t wcx_event_dispatch(wcx_event_bus_t *bus)
{
    if (bus == NULL)
    {
        return 0U;
    }
    size_t dispatched = 0U;
    while (bus->queue_count > 0U)
    {
        wcx_event_t event = bus->queue[bus->queue_head];
        bus->queue_head = (bus->queue_head + 1U) % bus->queue_capacity;
        bus->queue_count--;
        for (size_t i = 0U; i < bus->subs_count; i++)
        {
            wcx_subscription_t *s = &bus->subs[i];
            if (s->active && s->event_id == event.id)
            {
                s->handler(event, s->context);
                dispatched++;
            }
        }
    }
    return dispatched;
}

size_t wcx_event_pending(const wcx_event_bus_t *bus)
{
    if (bus == NULL)
    {
        return 0U;
    }
    return bus->queue_count;
}
