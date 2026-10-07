/*
 * test_scheduler.c — periods, first run, next due, and removal.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/scheduler.h"
#include "core_internal.h"

#include <assert.h>

static int runs;

static RoboHubError tick(robohub_core_task_id id, void* user)
{
    (void)id; (void)user;
    runs++;
    return ROBOHUB_OK;
}

int main(void)
{
    assert(robohub_core_scheduler_init() == ROBOHUB_OK);
    assert(robohub_core_task_count() == 0);

    robohub_core_task_id id = RH_CORE_TASK_INVALID;
    assert(robohub_core_task_add("tick", tick, 0, 100u, &id) == ROBOHUB_OK);
    assert(id != RH_CORE_TASK_INVALID);
    assert(robohub_core_task_count() == 1);

    /* First pass runs it; the period is measured from there. */
    robohub_core_scheduler_run(0u);
    assert(runs == 1);
    robohub_core_scheduler_run(0u);
    assert(runs == 1);
    robohub_core_scheduler_run(99u);
    assert(runs == 1);
    robohub_core_scheduler_run(100u);
    assert(runs == 2);

    assert(robohub_core_scheduler_next_due(150u) == 50u);

    assert(robohub_core_task_set_period(id, 200u) == ROBOHUB_OK);
    assert(robohub_core_task_set_period(id, 0u) == ROBOHUB_ERR_INVALID_ARG);
    assert(robohub_core_task_set_period(RH_CORE_TASK_INVALID, 10u) == ROBOHUB_ERR_NOT_FOUND);

    /* Removing stops it. */
    assert(robohub_core_task_remove(id) == ROBOHUB_OK);
    assert(robohub_core_task_count() == 0);
    robohub_core_scheduler_run(1000u);
    assert(runs == 2);

    /* A null function or a zero period is refused. */
    assert(robohub_core_task_add("bad", 0, 0, 10u, 0) == ROBOHUB_ERR_INVALID_ARG);
    assert(robohub_core_task_add("bad", tick, 0, 0u, 0) == ROBOHUB_ERR_INVALID_ARG);

    /* No tasks leaves nothing due. */
    assert(robohub_core_scheduler_next_due(0u) == RH_CORE_NO_DUE);

    return 0;
}
