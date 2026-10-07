/*
 * event.c — the inter-module event bus.
 *
 * A fixed table of subscriptions and a synchronous walk over it. There is no
 * queue and no copy: publish hands each handler the caller's pointer and lets
 * it read in place, which is what zero-copy means here and why the payload's
 * lifetime is the call.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/event.h"
#include "robohub/core/config.h"
#include "core_internal.h"

typedef struct {
    robohub_event_id id;
    robohub_core_event_handler handler;
    void* user;
    rh_bool used;
} subscription;

static subscription subscriptions[RH_CORE_MAX_SUBSCRIBERS];

RoboHubError robohub_core_event_init(void)
{
    robohub_core_event_reset();
    return ROBOHUB_OK;
}

void robohub_core_event_reset(void)
{
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        subscriptions[i].used = RH_FALSE;
        subscriptions[i].id = RH_EVENT_NONE;
        subscriptions[i].handler = 0;
        subscriptions[i].user = 0;
    }
}

static rh_bool same(const subscription* slot,
                    robohub_event_id id,
                    robohub_core_event_handler handler,
                    void* user)
{
    return slot->used &&
           slot->id == id &&
           slot->handler == handler &&
           slot->user == user;
}

RoboHubError robohub_core_event_subscribe(robohub_event_id id,
                                          robohub_core_event_handler handler,
                                          void* user)
{
    if (handler == 0) {
        return ROBOHUB_ERR_INVALID_ARG;
    }

    /* Subscribing twice is one subscription: a module brought up twice is one
     * module, and being called twice per event would be a bug it did not ask
     * for. */
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        if (same(&subscriptions[i], id, handler, user)) {
            return ROBOHUB_OK;
        }
    }
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        if (!subscriptions[i].used) {
            subscriptions[i].used = RH_TRUE;
            subscriptions[i].id = id;
            subscriptions[i].handler = handler;
            subscriptions[i].user = user;
            return ROBOHUB_OK;
        }
    }
    return ROBOHUB_ERR_NO_MEM;
}

RoboHubError robohub_core_event_unsubscribe(robohub_event_id id,
                                            robohub_core_event_handler handler,
                                            void* user)
{
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        if (same(&subscriptions[i], id, handler, user)) {
            subscriptions[i].used = RH_FALSE;
            subscriptions[i].handler = 0;
            subscriptions[i].user = 0;
        }
    }
    return ROBOHUB_OK;
}

RoboHubError robohub_core_event_publish(robohub_event_id id,
                                        const void* payload,
                                        rh_size size)
{
    if (id == RH_EVENT_NONE) {
        return ROBOHUB_ERR_INVALID_ARG;
    }

    robohub_core_diagnostics* diag = robohub_core_diag_state();
    diag->events_published++;

    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        subscription* slot = &subscriptions[i];
        if (!slot->used || slot->id != id) {
            continue;
        }
        diag->event_deliveries++;
        RoboHubError err = slot->handler(id, payload, size, slot->user);
        if (ROBOHUB_FAILED(err)) {
            /* One subscriber that fails does not silence the others, and the
             * publisher is not stopped for someone else's error. It is counted,
             * and kept as the last error, because a component that always
             * refuses its events is the thing worth seeing. */
            diag->handler_errors++;
            diag->last_error = err;
        }
    }
    return ROBOHUB_OK;
}

rh_u32 robohub_core_event_subscribers(robohub_event_id id)
{
    rh_u32 count = 0;
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        if (subscriptions[i].used && subscriptions[i].id == id) {
            count++;
        }
    }
    return count;
}
