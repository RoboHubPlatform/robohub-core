/*
 * scheduler.c — cooperative periodic tasks.
 *
 * A table of tasks and one comparison per pass. Due-ness is a signed
 * subtraction, so the millisecond counter wrapping is not a special case: a
 * task with a deadline in the past is due, and one with a deadline in the
 * future is not, whichever side of the wrap the two fall on.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/scheduler.h"
#include "robohub/core/config.h"
#include "core_internal.h"

typedef struct {
    robohub_core_task_id id;
    const char* name;
    robohub_core_task_fn fn;
    void* user;
    rh_u32 period_ms;
    rh_u32 next_ms;   /* zero means "due on the next pass" */
    rh_bool used;
} task_slot;

static task_slot tasks[RH_CORE_MAX_TASKS];
static robohub_core_task_id next_id = 1u;

static task_slot* find(robohub_core_task_id id)
{
    for (rh_u32 i = 0; i < RH_CORE_MAX_TASKS; i++) {
        if (tasks[i].used && tasks[i].id == id) {
            return &tasks[i];
        }
    }
    return 0;
}

RoboHubError robohub_core_scheduler_init(void)
{
    robohub_core_scheduler_reset();
    return ROBOHUB_OK;
}

void robohub_core_scheduler_reset(void)
{
    for (rh_u32 i = 0; i < RH_CORE_MAX_TASKS; i++) {
        tasks[i].used = RH_FALSE;
        tasks[i].id = RH_CORE_TASK_INVALID;
        tasks[i].name = 0;
        tasks[i].fn = 0;
        tasks[i].user = 0;
        tasks[i].period_ms = 0u;
        tasks[i].next_ms = 0u;
    }
    next_id = 1u;
}

RoboHubError robohub_core_task_add(const char* name,
                                   robohub_core_task_fn fn,
                                   void* user,
                                   rh_u32 period_ms,
                                   robohub_core_task_id* out_id)
{
    if (fn == 0 || period_ms == 0u) {
        return ROBOHUB_ERR_INVALID_ARG;
    }

    for (rh_u32 i = 0; i < RH_CORE_MAX_TASKS; i++) {
        if (tasks[i].used) {
            continue;
        }
        robohub_core_task_id id = next_id++;
        if (next_id == 0u) {
            next_id = 1u; /* RH_CORE_TASK_INVALID is not an id */
        }
        tasks[i].used = RH_TRUE;
        tasks[i].id = id;
        tasks[i].name = name;
        tasks[i].fn = fn;
        tasks[i].user = user;
        tasks[i].period_ms = period_ms;
        tasks[i].next_ms = 0u;
        if (out_id != 0) {
            *out_id = id;
        }
        return ROBOHUB_OK;
    }
    return ROBOHUB_ERR_NO_MEM;
}

RoboHubError robohub_core_task_remove(robohub_core_task_id id)
{
    task_slot* slot = find(id);
    if (slot == 0) {
        return ROBOHUB_OK;
    }
    slot->used = RH_FALSE;
    slot->fn = 0;
    slot->user = 0;
    return ROBOHUB_OK;
}

RoboHubError robohub_core_task_set_period(robohub_core_task_id id, rh_u32 period_ms)
{
    if (period_ms == 0u) {
        return ROBOHUB_ERR_INVALID_ARG;
    }
    task_slot* slot = find(id);
    if (slot == 0) {
        return ROBOHUB_ERR_NOT_FOUND;
    }
    slot->period_ms = period_ms;
    return ROBOHUB_OK;
}

rh_u32 robohub_core_task_count(void)
{
    rh_u32 count = 0;
    for (rh_u32 i = 0; i < RH_CORE_MAX_TASKS; i++) {
        if (tasks[i].used) {
            count++;
        }
    }
    return count;
}

void robohub_core_scheduler_run(rh_u32 now_ms)
{
    robohub_core_diagnostics* diag = robohub_core_diag_state();
    for (rh_u32 i = 0; i < RH_CORE_MAX_TASKS; i++) {
        task_slot* slot = &tasks[i];
        if (!slot->used) {
            continue;
        }
        rh_bool due = slot->next_ms == 0u || (rh_i32)(now_ms - slot->next_ms) >= 0;
        if (!due) {
            continue;
        }

        diag->tasks_run++;
        RoboHubError err = slot->fn(slot->id, slot->user);
        if (ROBOHUB_FAILED(err)) {
            diag->task_errors++;
            diag->last_error = err;
        }

        slot->next_ms = now_ms + slot->period_ms;
        if (slot->next_ms == 0u) {
            slot->next_ms = 1u; /* keep zero meaning "first pass" */
        }
    }
}

rh_u32 robohub_core_scheduler_next_due(rh_u32 now_ms)
{
    rh_u32 best = RH_CORE_NO_DUE;
    for (rh_u32 i = 0; i < RH_CORE_MAX_TASKS; i++) {
        task_slot* slot = &tasks[i];
        if (!slot->used) {
            continue;
        }
        rh_u32 delta;
        if (slot->next_ms == 0u) {
            delta = 0u;
        } else {
            rh_i32 remaining = (rh_i32)(slot->next_ms - now_ms);
            delta = remaining > 0 ? (rh_u32)remaining : 0u;
        }
        if (delta < best) {
            best = delta;
        }
    }
    return best;
}
