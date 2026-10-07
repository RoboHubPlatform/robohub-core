/*
 * test_event.c — the event bus: delivery, identity and the zero-copy contract.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/event.h"
#include "robohub/core/config.h"
#include "robohub/core/diagnostics.h"
#include "core_internal.h"

#include <assert.h>

static int calls;
static rh_f32 seen;

static RoboHubError on_temperature(robohub_event_id id, const void* payload, rh_size size, void* user)
{
    (void)user;
    assert(id == RH_EVENT_ID(RH_EVENT_DOMAIN_SENSOR, 1));
    assert(size == sizeof(rh_f32));
    calls++;
    seen = *(const rh_f32*)payload;
    return ROBOHUB_OK;
}

static RoboHubError on_other(robohub_event_id id, const void* payload, rh_size size, void* user)
{
    (void)id; (void)payload; (void)size; (void)user;
    assert(0 && "a handler was called for an event it did not subscribe to");
    return ROBOHUB_OK;
}

static RoboHubError always_fails(robohub_event_id id, const void* payload, rh_size size, void* user)
{
    (void)id; (void)payload; (void)size; (void)user;
    return ROBOHUB_ERR_FAIL;
}

int main(void)
{
    assert(robohub_core_event_init() == ROBOHUB_OK);

    const robohub_event_id temperature = RH_EVENT_ID(RH_EVENT_DOMAIN_SENSOR, 1);
    const robohub_event_id pressure = RH_EVENT_ID(RH_EVENT_DOMAIN_SENSOR, 2);
    rh_f32 payload = 21.5f;

    /* Nothing subscribed: publishing is not an error and nobody runs. */
    assert(robohub_core_event_publish(temperature, &payload, sizeof payload) == ROBOHUB_OK);
    assert(calls == 0);

    assert(robohub_core_event_subscribe(temperature, on_temperature, 0) == ROBOHUB_OK);
    assert(robohub_core_event_subscribers(temperature) == 1);

    assert(robohub_core_event_publish(temperature, &payload, sizeof payload) == ROBOHUB_OK);
    assert(calls == 1);
    assert(seen == payload);

    /* A different event does not reach it. */
    assert(robohub_core_event_subscribe(pressure, on_other, 0) == ROBOHUB_OK);
    assert(robohub_core_event_publish(temperature, &payload, sizeof payload) == ROBOHUB_OK);
    assert(calls == 2);

    /* Subscribing twice is one subscription. */
    assert(robohub_core_event_subscribe(temperature, on_temperature, 0) == ROBOHUB_OK);
    assert(robohub_core_event_subscribers(temperature) == 1);
    assert(robohub_core_event_publish(temperature, &payload, sizeof payload) == ROBOHUB_OK);
    assert(calls == 3);

    /* Unsubscribing stops it. */
    assert(robohub_core_event_unsubscribe(temperature, on_temperature, 0) == ROBOHUB_OK);
    assert(robohub_core_event_publish(temperature, &payload, sizeof payload) == ROBOHUB_OK);
    assert(calls == 3);

    /* The table is finite and says so. */
    robohub_core_event_reset();
    static int users[RH_CORE_MAX_SUBSCRIBERS];
    static int sentinel;
    for (rh_u32 i = 0; i < RH_CORE_MAX_SUBSCRIBERS; i++) {
        assert(robohub_core_event_subscribe(temperature, on_temperature, &users[i]) == ROBOHUB_OK);
    }
    assert(robohub_core_event_subscribe(temperature, on_temperature, &sentinel) == ROBOHUB_ERR_NO_MEM);

    /* A null handler is refused, and no event is none. */
    assert(robohub_core_event_subscribe(temperature, 0, 0) == ROBOHUB_ERR_INVALID_ARG);
    assert(robohub_core_event_publish(RH_EVENT_NONE, &payload, sizeof payload) == ROBOHUB_ERR_INVALID_ARG);

    /* A handler that fails is counted, and the delivery is recorded. */
    robohub_core_event_reset();
    robohub_core_diagnostics_reset();
    assert(robohub_core_event_subscribe(pressure, always_fails, 0) == ROBOHUB_OK);
    assert(robohub_core_event_publish(pressure, 0, 0) == ROBOHUB_OK);
    robohub_core_diagnostics snapshot;
    robohub_core_diagnostics_get(&snapshot);
    assert(snapshot.events_published == 1);
    assert(snapshot.event_deliveries == 1);
    assert(snapshot.handler_errors == 1);

    return 0;
}
