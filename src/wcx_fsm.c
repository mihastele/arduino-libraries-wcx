#include "wcx_fsm.h"

static const wcx_fsm_state_t *wcx_fsm_find_state(const wcx_fsm_t *fsm, wcx_fsm_state_id_t id)
{
    size_t index;

    if (fsm == NULL || fsm->states == NULL)
    {
        return NULL;
    }

    for (index = 0U; index < fsm->state_count; ++index)
    {
        if (fsm->states[index].id == id)
        {
            return &fsm->states[index];
        }
    }

    return NULL;
}

void wcx_fsm_init(wcx_fsm_t *fsm,
                  wcx_fsm_state_id_t initial_state,
                  const wcx_fsm_state_t *states,
                  size_t state_count,
                  const wcx_fsm_transition_t *transitions,
                  size_t transition_count,
                  void *context)
{
    if (fsm == NULL)
    {
        return;
    }

    fsm->current_state = initial_state;
    fsm->states = states;
    fsm->state_count = state_count;
    fsm->transitions = transitions;
    fsm->transition_count = transition_count;
    fsm->context = context;
}

void wcx_fsm_reset(wcx_fsm_t *fsm, wcx_fsm_state_id_t state)
{
    if (fsm == NULL)
    {
        return;
    }

    fsm->current_state = state;
}

wcx_fsm_state_id_t wcx_fsm_state(const wcx_fsm_t *fsm)
{
    if (fsm == NULL)
    {
        return 0U;
    }

    return fsm->current_state;
}

bool wcx_fsm_dispatch(wcx_fsm_t *fsm, wcx_fsm_event_id_t event)
{
    size_t index;
    const wcx_fsm_transition_t *transition;
    const wcx_fsm_state_t *from_state;
    const wcx_fsm_state_t *to_state;

    if (fsm == NULL || fsm->transitions == NULL)
    {
        return false;
    }

    for (index = 0U; index < fsm->transition_count; ++index)
    {
        transition = &fsm->transitions[index];

        if (transition->from_state != fsm->current_state || transition->event != event)
        {
            continue;
        }

        from_state = wcx_fsm_find_state(fsm, transition->from_state);
        to_state = wcx_fsm_find_state(fsm, transition->to_state);

        if (transition->to_state != transition->from_state &&
            from_state != NULL && from_state->on_exit != NULL)
        {
            from_state->on_exit(fsm->context, transition->from_state);
        }

        if (transition->action != NULL)
        {
            transition->action(fsm->context,
                               transition->from_state,
                               transition->event,
                               transition->to_state);
        }

        fsm->current_state = transition->to_state;

        if (transition->to_state != transition->from_state &&
            to_state != NULL && to_state->on_enter != NULL)
        {
            to_state->on_enter(fsm->context, transition->to_state);
        }

        return true;
    }

    return false;
}