/**
 * @file scheduler.h
 * @brief Periodic work for the RoboHub application runtime.
 *
 * A program on a board is a set of things that happen at intervals — read the
 * sensor, publish telemetry, blink a heartbeat — and the scheduler is where
 * those intervals are said once. A task is a function and a period; the runtime
 * runs it when it is due, cooperatively, in one loop.
 *
 * Cooperative is the whole model and it has no preemption: a task runs until it
 * returns, so a task that blocks blocks everything. That is a constraint the
 * caller owns, and in exchange the runtime needs no stack per task and no
 * locking — a task may touch any state in the program, because nothing else is
 * running.
 *
 * @project RoboHub Platform
 * @author Aurel Miron
 * @version 0.1
 * @date 2026
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_ROBOHUB_CORE_SCHEDULER_H
#define INCLUDE_ROBOHUB_CORE_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include "types.h"
#include "error.h"

/** The handle of a registered task. */
typedef rh_u16 robohub_core_task_id;

/** No task: what add leaves behind when it fails and remove is given nothing. */
#define RH_CORE_TASK_INVALID ((robohub_core_task_id)0u)

/**
 * @brief A task: called when its period elapses.
 *
 * Returning an error does not unregister the task — a task that failed once is
 * a task that will run again — but the error is counted and kept as the last
 * error, which is what the diagnostics are for.
 */
typedef RoboHubError (*robohub_core_task_fn)(robohub_core_task_id id, void* user);

/**
 * @brief Register a task.
 *
 * The name is a label for a human reading a diagnostic, not an identity: two
 * tasks may share one. The task first runs on the next scheduler pass and every
 * `period_ms` after that. Returns `ROBOHUB_ERR_NO_MEM` when the table is full,
 * `ROBOHUB_ERR_INVALID_ARG` for a null function or a zero period.
 */
RoboHubError robohub_core_task_add(const char* name,
                                   robohub_core_task_fn fn,
                                   void* user,
                                   rh_u32 period_ms,
                                   robohub_core_task_id* out_id);

/** Remove a task. Removing one that is not there is not an error. */
RoboHubError robohub_core_task_remove(robohub_core_task_id id);

/** Change a task's period. */
RoboHubError robohub_core_task_set_period(robohub_core_task_id id, rh_u32 period_ms);

/** How many tasks are registered. */
rh_u32 robohub_core_task_count(void);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_CORE_SCHEDULER_H */
