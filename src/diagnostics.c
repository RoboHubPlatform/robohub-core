/*
 * diagnostics.c — the runtime's own counters.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/diagnostics.h"
#include "core_internal.h"

static robohub_core_diagnostics counters;

robohub_core_diagnostics* robohub_core_diag_state(void)
{
    return &counters;
}

void robohub_core_diagnostics_get(robohub_core_diagnostics* out)
{
    if (out == 0) {
        return;
    }
    *out = counters;
}

void robohub_core_diagnostics_reset(void)
{
    counters.events_published = 0u;
    counters.event_deliveries = 0u;
    counters.handler_errors = 0u;
    counters.tasks_run = 0u;
    counters.task_errors = 0u;
    counters.app_steps = 0u;
    counters.uptime_ms = 0u;
    counters.last_error = ROBOHUB_OK;
}
