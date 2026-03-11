#ifndef WCX_DEBOUNCE_H
#define WCX_DEBOUNCE_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        bool stable_state;
        bool candidate_state;
        uint32_t interval_ms;
        uint32_t candidate_since_ms;
        bool changed;
        bool rose;
        bool fell;
    } wcx_debounce_t;

    void wcx_debounce_init(wcx_debounce_t *debounce, bool initial_state, uint32_t interval_ms);
    bool wcx_debounce_update(wcx_debounce_t *debounce, bool raw_state, uint32_t now_ms);
    bool wcx_debounce_state(const wcx_debounce_t *debounce);
    bool wcx_debounce_changed(const wcx_debounce_t *debounce);
    bool wcx_debounce_rose(const wcx_debounce_t *debounce);
    bool wcx_debounce_fell(const wcx_debounce_t *debounce);

#ifdef __cplusplus
}
#endif

#endif
