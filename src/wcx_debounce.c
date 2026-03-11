#include "wcx_debounce.h"

void wcx_debounce_init(wcx_debounce_t *debounce, bool initial_state, uint32_t interval_ms)
{
    if (debounce == NULL)
    {
        return;
    }

    debounce->stable_state = initial_state;
    debounce->candidate_state = initial_state;
    debounce->interval_ms = interval_ms;
    debounce->candidate_since_ms = 0U;
    debounce->changed = false;
    debounce->rose = false;
    debounce->fell = false;
}

bool wcx_debounce_update(wcx_debounce_t *debounce, bool raw_state, uint32_t now_ms)
{
    if (debounce == NULL)
    {
        return false;
    }

    debounce->changed = false;
    debounce->rose = false;
    debounce->fell = false;

    if (raw_state != debounce->candidate_state)
    {
        debounce->candidate_state = raw_state;
        debounce->candidate_since_ms = now_ms;
        return debounce->stable_state;
    }

    if (debounce->stable_state != debounce->candidate_state &&
        wcx_time_reached(now_ms, debounce->candidate_since_ms + debounce->interval_ms))
    {
        debounce->stable_state = debounce->candidate_state;
        debounce->changed = true;
        debounce->rose = debounce->stable_state;
        debounce->fell = !debounce->stable_state;
    }

    return debounce->stable_state;
}

bool wcx_debounce_state(const wcx_debounce_t *debounce)
{
    return debounce != NULL && debounce->stable_state;
}

bool wcx_debounce_changed(const wcx_debounce_t *debounce)
{
    return debounce != NULL && debounce->changed;
}

bool wcx_debounce_rose(const wcx_debounce_t *debounce)
{
    return debounce != NULL && debounce->rose;
}

bool wcx_debounce_fell(const wcx_debounce_t *debounce)
{
    return debounce != NULL && debounce->fell;
}
