/**
 * @file event.h
 * @brief The inter-module event bus of the RoboHub application runtime.
 *
 * A program's modules do not have to be wired to each other. A sensor publishes
 * the reading it took; whoever cares subscribes to it; neither names the other.
 * Adding the telemetry component is a subscription, and removing the sensor
 * leaves the components that did not depend on it untouched.
 *
 * Zero-copy, and what that means. Publish hands the handlers the caller's own
 * pointer — the reading is not copied, encoded or queued. The price is a
 * lifetime the contract states plainly: the payload is valid for the duration
 * of the publish call and not a moment after it, because the publisher owns the
 * buffer and will reuse it. A handler that needs the value later copies it; a
 * handler that reads it in place pays nothing.
 *
 * Dispatch is synchronous: a handler runs inside the publish call, on the
 * publisher's stack. That is what makes the lifetime rule possible, and it is
 * why a handler must be short — it is holding up the publisher.
 *
 * @project RoboHub Platform
 * @author Aurel Miron
 * @version 0.1
 * @date 2026
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_ROBOHUB_CORE_EVENT_H
#define INCLUDE_ROBOHUB_CORE_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include "types.h"
#include "error.h"
#include "robohub/core/events.h"

/* -------------------------------------------------------------------------- */
/*  Handler                                                                   */
/* -------------------------------------------------------------------------- */

/**
 * @brief What a subscriber is called with.
 *
 * `payload` is the publisher's buffer and is valid only until the handler
 * returns. `user` is whatever the subscriber gave at subscription time, so one
 * function can serve several subscriptions without a global to tell them apart.
 *
 * A handler returns an error when it could not take the event — a queue full, a
 * value refused. That does not stop the other subscribers and does not fail the
 * publish; it is counted, and kept as the last error, because a component that
 * always refuses its events is the thing worth seeing.
 */
typedef RoboHubError (*robohub_core_event_handler)(robohub_event_id id,
                                                   const void* payload,
                                                   rh_size size,
                                                   void* user);

/* -------------------------------------------------------------------------- */
/*  Bus                                                                       */
/* -------------------------------------------------------------------------- */

/**
 * @brief Subscribe to an event.
 *
 * Subscribing twice with the same handler and user is the same subscription and
 * is not an error: a module that brings itself up twice is one module, not two.
 * Returns `ROBOHUB_ERR_NO_MEM` when every slot is taken.
 */
RoboHubError robohub_core_event_subscribe(robohub_event_id id,
                                          robohub_core_event_handler handler,
                                          void* user);

/** Remove a subscription. A subscription that is not there is not an error. */
RoboHubError robohub_core_event_unsubscribe(robohub_event_id id,
                                            robohub_core_event_handler handler,
                                            void* user);

/**
 * @brief Publish an event to whoever subscribed.
 *
 * Handlers run in subscription order, inline. A handler that returns an error
 * does not stop the others: one bad subscriber does not silence the rest. The
 * call returns success even when a handler failed; the failure is counted, and
 * the last one is visible through the diagnostics.
 */
RoboHubError robohub_core_event_publish(robohub_event_id id,
                                        const void* payload,
                                        rh_size size);

/** How many handlers are subscribed to an event. */
rh_u32 robohub_core_event_subscribers(robohub_event_id id);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_CORE_EVENT_H */
