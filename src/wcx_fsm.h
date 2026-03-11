#ifndef WCX_FSM_H
#define WCX_FSM_H

#include "wcx_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef uint8_t wcx_fsm_state_id_t;
    typedef uint8_t wcx_fsm_event_id_t;

    typedef void (*wcx_fsm_state_callback_t)(void *context, wcx_fsm_state_id_t state);
    typedef void (*wcx_fsm_transition_action_t)(void *context,
                                                wcx_fsm_state_id_t from_state,
                                                wcx_fsm_event_id_t event,
                                                wcx_fsm_state_id_t to_state);

    typedef struct
    {
        wcx_fsm_state_id_t id;
        wcx_fsm_state_callback_t on_enter;
        wcx_fsm_state_callback_t on_exit;
    } wcx_fsm_state_t;

    typedef struct
    {
        wcx_fsm_state_id_t from_state;
        wcx_fsm_event_id_t event;
        wcx_fsm_state_id_t to_state;
        wcx_fsm_transition_action_t action;
    } wcx_fsm_transition_t;

    typedef struct
    {
        wcx_fsm_state_id_t current_state;
        const wcx_fsm_state_t *states;
        size_t state_count;
        const wcx_fsm_transition_t *transitions;
        size_t transition_count;
        void *context;
    } wcx_fsm_t;

    void wcx_fsm_init(wcx_fsm_t *fsm,
                      wcx_fsm_state_id_t initial_state,
                      const wcx_fsm_state_t *states,
                      size_t state_count,
                      const wcx_fsm_transition_t *transitions,
                      size_t transition_count,
                      void *context);
    void wcx_fsm_reset(wcx_fsm_t *fsm, wcx_fsm_state_id_t state);
    wcx_fsm_state_id_t wcx_fsm_state(const wcx_fsm_t *fsm);
    bool wcx_fsm_dispatch(wcx_fsm_t *fsm, wcx_fsm_event_id_t event);

#ifdef __cplusplus
}
#endif

#endif