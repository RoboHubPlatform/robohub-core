/**
 * @file diagnostics.h
 * @brief Counters a running runtime keeps about itself.
 *
 * A board has no debugger attached in the field, so the runtime has to be able
 * to say what it has been doing: how many events went out, how many handlers
 * refused one, how many tasks ran and failed, how long it has been up, and the
 * last error any of it produced. It is a snapshot of counters, not a log — the
 * smallest thing that turns "it stopped" into "the third task has been failing
 * since boot".
 *
 * @project RoboHub Platform
 * @author Aurel Miron
 * @version 0.1
 * @date 2026
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_ROBOHUB_CORE_DIAGNOSTICS_H
#define INCLUDE_ROBOHUB_CORE_DIAGNOSTICS_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include "types.h"
#include "error.h"

typedef struct {
    /** Events handed to the bus. */
    rh_u32 events_published;
    /** Handlers that were called for one. */
    rh_u32 event_deliveries;
    /** Handlers that returned an error. */
    rh_u32 handler_errors;
    /** Tasks the scheduler ran. */
    rh_u32 tasks_run;
    /** Tasks that returned an error. */
    rh_u32 task_errors;
    /** Application step calls. */
    rh_u32 app_steps;
    /** Milliseconds since the runtime started. */
    rh_u32 uptime_ms;
    /** The last error any handler, task or application produced. */
    RoboHubError last_error;
} robohub_core_diagnostics;

/** Copy the current counters out. */
void robohub_core_diagnostics_get(robohub_core_diagnostics* out);

/** Zero the counters. */
void robohub_core_diagnostics_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_CORE_DIAGNOSTICS_H */
