/*
 * diagnostics.c — the runtime's own counters.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/diagnostics.h"
#include "registry.h"
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

/* -------------------------------------------------------------------------- */
/*  Hardware smoke                                                            */
/* -------------------------------------------------------------------------- */

/** Record a probe's result: set in one mask, and not in the other. */
static void record(rh_u32 probe, rh_bool passed, rh_u32* ok, rh_u32* fault)
{
    if (passed) {
        *ok |= probe;
    } else {
        *fault |= probe;
    }
}

RoboHubError robohub_core_smoke(RoboHubTimer* timer, rh_u32 mask, rh_u32* probe_ok, rh_u32* probe_fault)
{
    if (probe_ok == 0 || probe_fault == 0) {
        return ROBOHUB_ERR_INVALID_ARG;
    }

    /* The runtime is running and this answer travels over the console, so the
     * console is up by construction. */
    if ((mask & ROBOHUB_PROBE_CONSOLE) != 0u) {
        record(ROBOHUB_PROBE_CONSOLE, RH_TRUE, probe_ok, probe_fault);
    }

    /* A timer that goes backwards is a timer that has lost its count — the one
     * failure a clock can have that a presence check would not see. */
    if ((mask & ROBOHUB_PROBE_TIMER) != 0u) {
        if (timer == 0 || timer->get_millis == 0) {
            /* Absent: neither mask. */
        } else {
            rh_millis first = timer->get_millis(timer);
            rh_millis second = timer->get_millis(timer);
            record(ROBOHUB_PROBE_TIMER, second >= first, probe_ok, probe_fault);
        }
    }

    /* The buses a platform registers: present is the answer, because a board
     * registered one to use it and a smoke cannot ask a bus what is on it. */
    if ((mask & ROBOHUB_PROBE_ONEWIRE) != 0u && robohub_count_onewire() > 0u) {
        record(ROBOHUB_PROBE_ONEWIRE, RH_TRUE, probe_ok, probe_fault);
    }
    if ((mask & ROBOHUB_PROBE_I2C) != 0u && robohub_count_i2c() > 0u) {
        record(ROBOHUB_PROBE_I2C, RH_TRUE, probe_ok, probe_fault);
    }
    if ((mask & ROBOHUB_PROBE_SPI) != 0u && robohub_count_spi() > 0u) {
        record(ROBOHUB_PROBE_SPI, RH_TRUE, probe_ok, probe_fault);
    }
    if ((mask & ROBOHUB_PROBE_GPIO) != 0u && robohub_count_gpio() > 0u) {
        record(ROBOHUB_PROBE_GPIO, RH_TRUE, probe_ok, probe_fault);
    }
    if ((mask & ROBOHUB_PROBE_UART) != 0u && robohub_count_uart() > 0u) {
        record(ROBOHUB_PROBE_UART, RH_TRUE, probe_ok, probe_fault);
    }

    /* The network is the platform's, and the runtime has no handle on it: a
     * probe it cannot answer is left absent rather than guessed. */

    return ROBOHUB_OK;
}
