/*
 * core_internal.h — what the runtime's own sources share.
 *
 * None of this is a contract: it is the seam between the parts of one library,
 * and it is not installed. The public headers are the contract.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#ifndef ROBOHUB_CORE_INTERNAL_H
#define ROBOHUB_CORE_INTERNAL_H

#include "types.h"
#include "error.h"
#include "robohub/core/diagnostics.h"

/** No task is due within the foreseeable future. */
#define RH_CORE_NO_DUE 0xFFFFFFFFu

/* The counters, writable by the parts that count. */
robohub_core_diagnostics* robohub_core_diag_state(void);

/* Event bus. */
RoboHubError robohub_core_event_init(void);
void robohub_core_event_reset(void);

/* Scheduler. */
RoboHubError robohub_core_scheduler_init(void);
void robohub_core_scheduler_reset(void);
void robohub_core_scheduler_run(rh_u32 now_ms);
rh_u32 robohub_core_scheduler_next_due(rh_u32 now_ms);

/* Applications. */
void robohub_core_app_reset(void);
RoboHubError robohub_core_app_init(void);
RoboHubError robohub_core_app_start(void);
void robohub_core_app_step(rh_u32 now_ms);
void robohub_core_app_stop(void);

#endif /* ROBOHUB_CORE_INTERNAL_H */
