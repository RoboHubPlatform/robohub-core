/*
 * event.c — the inter-module event bus.
 *
 * A fixed table of subscriptions and a synchronous walk over it. There is no
 * queue and no copy: publish hands each handler the caller's pointer and lets
 * it read in place, which is what zero-copy means here and why the payload's
 * lifetime is the call.
 *
 * The table is guarded by a recursive OSAL mutex. It has to be recursive
 * because a handler runs inside publish, holding the lock, and a handler that
 * publishes or subscribes would otherwise deadlock against the bus it is
 * already in. Without an operating system the mutex is the OSAL's no-op, so the
 * same code costs nothing on bare metal.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/event.h"
#include "robohub/core/config.h"
#include "robohub/osal.h"
#include "core_internal.h"

typedef struct {
    robohub_event_id id;
    robohub_core_event_handler handler;
    void* user;
    rh_bool used;
} subscription;

static subscription subscriptions[RH_CORE_MAX_SUBSCRIBERS];
static robohub_mutex_t bus_mutex;
static rh_u16 current_tx;

RoboHubError robohub_core_event_init(void)
{
    bus_mutex = robohub_osal_mutex_create();
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

    robohub_osal_mutex_lock(bus_mutex);
    RoboHubError result = ROBOHUB_ERR_NO_MEM;

    /* Subscribing twice is one subscription: a module brought up twice is one
     * module, and being called twice per event would be a bug it did not ask
     * for. */
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS && result != ROBOHUB_OK; i++) {
        if (same(&subscriptions[i], id, handler, user)) {
            result = ROBOHUB_OK;
        }
    }
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS && result != ROBOHUB_OK; i++) {
        if (!subscriptions[i].used) {
            subscriptions[i].used = RH_TRUE;
            subscriptions[i].id = id;
            subscriptions[i].handler = handler;
            subscriptions[i].user = user;
            result = ROBOHUB_OK;
        }
    }

    robohub_osal_mutex_unlock(bus_mutex);
    return result;
}

RoboHubError robohub_core_event_unsubscribe(robohub_event_id id,
                                            robohub_core_event_handler handler,
                                            void* user)
{
    robohub_osal_mutex_lock(bus_mutex);
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        if (same(&subscriptions[i], id, handler, user)) {
            subscriptions[i].used = RH_FALSE;
            subscriptions[i].handler = 0;
            subscriptions[i].user = 0;
        }
    }
    robohub_osal_mutex_unlock(bus_mutex);
    return ROBOHUB_OK;
}

RoboHubError robohub_core_event_publish(robohub_event_id id,
                                        const void* payload,
                                        rh_size size)
{
    return robohub_core_event_publish_with_tx(id, 0u, payload, size);
}

RoboHubError robohub_core_event_publish_with_tx(robohub_event_id id,
                                                rh_u16 tx_id,
                                                const void* payload,
                                                rh_size size)
{
    if (id == RH_EVENT_NONE) {
        return ROBOHUB_ERR_INVALID_ARG;
    }

    robohub_osal_mutex_lock(bus_mutex);

    robohub_core_diagnostics* diag = robohub_core_diag_state();
    diag->events_published++;

    /* The transaction id belongs to the dispatch, not to the bus: a handler
     * that publishes in turn gets its own, and gets this one back when it
     * returns. */
    rh_u16 previous_tx = current_tx;
    current_tx = tx_id;

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

    current_tx = previous_tx;
    robohub_osal_mutex_unlock(bus_mutex);
    return ROBOHUB_OK;
}

rh_u16 robohub_core_event_current_tx(void)
{
    return current_tx;
}

rh_u32 robohub_core_event_subscribers(robohub_event_id id)
{
    robohub_osal_mutex_lock(bus_mutex);
    rh_u32 count = 0;
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        if (subscriptions[i].used && subscriptions[i].id == id) {
            count++;
        }
    }
    robohub_osal_mutex_unlock(bus_mutex);
    return count;
}
