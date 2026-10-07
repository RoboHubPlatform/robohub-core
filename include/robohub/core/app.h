/**
 * @file app.h
 * @brief The application lifecycle of the RoboHub application runtime.
 *
 * An application is the unit the runtime brings up and takes down: it is
 * initialised once, started when the loop begins, stepped every pass, and
 * stopped when the loop ends. A program is one or more of them, and the
 * runtime is what gives them an order — every `init` before any `start`,
 * every `stop` in reverse.
 *
 * It is deliberately not a task. A task is repeated work at a period; an
 * application is a part of the program with a beginning and an end, and it may
 * register tasks of its own in `init` or `start`. Keeping the two apart is what
 * lets a component be added to a program without a period being invented for
 * it.
 *
 * @project RoboHub Platform
 * @author Aurel Miron
 * @version 0.1
 * @date 2026
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_ROBOHUB_CORE_APP_H
#define INCLUDE_ROBOHUB_CORE_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include "types.h"
#include "error.h"

/**
 * @brief One application, as the runtime sees it.
 *
 * A null `init`, `start`, `step` or `stop` is simply a stage the application
 * does not have. Only `name` is required. The struct is read, not copied, so it
 * is expected to be a static the program owns for its lifetime.
 */
typedef struct robohub_core_app {
    /** A label for diagnostics. */
    const char* name;
    /** Called once, in registration order, before the loop. */
    RoboHubError (*init)(void);
    /** Called once when the loop begins. */
    RoboHubError (*start)(void);
    /** Called every pass of the loop with the current uptime in ms. */
    void (*step)(rh_u32 now_ms);
    /** Called once, in reverse order, when the loop ends. */
    void (*stop)(void);
} robohub_core_app;

/**
 * @brief Register an application.
 *
 * Called before `robohub_core_init`; registration after the runtime is up does
 * nothing, because the lifecycle it would join has already begun. Returns
 * `ROBOHUB_ERR_NO_MEM` when the table is full.
 */
RoboHubError robohub_core_app_register(const robohub_core_app* app);

/** How many applications are registered. */
rh_u32 robohub_core_app_count(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_CORE_APP_H */
