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
#include "robohub/hal/timer.h"

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

/* -------------------------------------------------------------------------- */
/*  Hardware smoke                                                            */
/* -------------------------------------------------------------------------- */

/**
 * The bit each probe occupies, matching the schema's ProbeId numbering.
 *
 * They are written here rather than shared, because the runtime does not depend
 * on a generated protocol header — a board's schema is its own. The two agree on
 * the numbering, and the smoke test checks it, so a divergence is a failing test
 * and not a wrong reading in the field.
 */
#define ROBOHUB_PROBE_CONSOLE (1u << 0)
#define ROBOHUB_PROBE_TIMER   (1u << 1)
#define ROBOHUB_PROBE_ONEWIRE (1u << 2)
#define ROBOHUB_PROBE_I2C     (1u << 3)
#define ROBOHUB_PROBE_SPI     (1u << 4)
#define ROBOHUB_PROBE_GPIO    (1u << 5)
#define ROBOHUB_PROBE_UART    (1u << 6)
#define ROBOHUB_PROBE_NET     (1u << 7)
#define ROBOHUB_PROBE_ALL     (0x000000FFu)

/**
 * @brief Run the hardware smoke the mask asks for.
 *
 * A probe is set in `probe_ok` when it passed, in `probe_fault` when it failed,
 * and in neither when it is absent — the platform brought no such device, which
 * is a fact rather than a failure, so absence is its own answer.
 *
 * `timer` is the instance to read twice for monotonicity, or null where the
 * caller has none. The other probes ask the registry whether the platform
 * registered the bus at all, which is why the runtime needs no name for them: a
 * board has one name per bus and no way to enumerate them from outside.
 *
 * A probe the runtime cannot answer — the network, whose interface belongs to a
 * platform that may not exist — is left absent rather than guessed.
 */
RoboHubError robohub_core_smoke(RoboHubTimer* timer, rh_u32 mask, rh_u32* probe_ok, rh_u32* probe_fault);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_CORE_DIAGNOSTICS_H */
