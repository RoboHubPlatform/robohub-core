/**
 * @file core.h
 * @brief The application runtime of the RoboHub Platform.
 *
 * This is what a program is built on when it is more than one thing. It brings
 * up the applications a program registers, gives them a clock, runs them in one
 * loop, and carries the events they send each other. It is the difference
 * between an application that calls a sensor and one whose sensor publishes a
 * reading that anything may subscribe to.
 *
 * It does not talk to hardware. The runtime is platform-agnostic and takes its
 * clock from a `RoboHubTimer` a platform registered — the same contract a
 * driver uses — so the same runtime runs on an RP2350, an STM32 or a host.
 *
 * A program does this, in this order:
 *
 *     robohub_platform_init();          // the platform brings its HALs up
 *     robohub_core_app_register(&telemetry);
 *     robohub_core_app_register(&control);
 *     robohub_core_init(NULL);          // apps' init runs
 *     robohub_core_run();               // apps start, loop, and stop
 *
 * @project RoboHub Platform
 * @author Aurel Miron
 * @version 0.1
 * @date 2026
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_ROBOHUB_CORE_CORE_H
#define INCLUDE_ROBOHUB_CORE_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include "types.h"
#include "error.h"
#include "robohub/hal/timer.h"

/**
 * @brief What the runtime is told when it is brought up.
 *
 * A field left zero takes its default, so a program with an ordinary board
 * passes NULL and the runtime finds the timer the board registered.
 */
typedef struct {
    /** The clock. NULL takes the registry's timer named by `RH_CORE_TIMER_NAME`. */
    RoboHubTimer* timer;
} robohub_core_config;

/**
 * @brief Bring the runtime up.
 *
 * Initialises the event bus, the scheduler and the diagnostics, and calls the
 * `init` of every registered application in registration order. The platform
 * must already be up: the runtime looks its clock up in the registry, and
 * returns `ROBOHUB_ERR_NOT_FOUND` if no timer is there.
 */
RoboHubError robohub_core_init(const robohub_core_config* config);

/**
 * @brief Run the applications until asked to stop.
 *
 * Calls every application's `start`, then loops: publish a tick, run the
 * scheduled work, step the applications, and sleep until the next task is due
 * or the idle tick, whichever comes first. Returns when `robohub_core_stop`
 * is called (from an application, a task or a handler) or when an application
 * refuses to start. The applications are stopped, in reverse, on the way out.
 */
RoboHubError robohub_core_run(void);

/** Ask the loop to return. Safe to call from an application, task or handler. */
void robohub_core_stop(void);

/** Whether the loop is running. */
rh_bool robohub_core_running(void);

/** Milliseconds since the runtime came up. */
rh_u32 robohub_core_uptime_ms(void);

/** A version string, for a diagnostic or a banner. */
const char* robohub_core_version(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_CORE_CORE_H */
