/*
 * test_diagnostics.c — the runtime's counters and the hardware smoke.
 *
 * The smoke is checked where it can be checked without a board: a registered
 * bus is present, an unregistered one is absent, and a clock that goes backwards
 * is a fault rather than a pass. The probe bits are asserted against the numbers
 * the schema gives them, so a divergence between this header and the wire is a
 * failing test.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/diagnostics.h"
#include "registry.h"

#include <assert.h>
#include <string.h>

/* A timer that counts up, then one that goes backwards. */
static rh_millis forward_now = 100u;
static rh_millis forward_millis(RoboHubTimer* self)
{
    (void)self;
    forward_now += 10u;
    return forward_now;
}

static rh_millis backward_now = 100u;
static rh_millis backward_millis(RoboHubTimer* self)
{
    (void)self;
    backward_now -= 5u;
    return backward_now;
}

static void no_delay(RoboHubTimer* self, rh_u32 ms) { (void)self; (void)ms; }

int main(void)
{
    /* The probe bits are the schema's ProbeId numbering. */
    assert(ROBOHUB_PROBE_CONSOLE == (1u << 0));
    assert(ROBOHUB_PROBE_TIMER == (1u << 1));
    assert(ROBOHUB_PROBE_NET == (1u << 7));

    RoboHubTimer timer = {0};
    timer.name = "system_timer";
    timer.get_millis = forward_millis;
    timer.delay_ms = no_delay;

    RoboHubGPIO gpio = {0};
    RoboHubOneWire onewire = {0};

    robohub_registry_init();
    assert(robohub_register_timer("system_timer", &timer) == ROBOHUB_OK);
    assert(robohub_register_gpio("gpio0", &gpio) == ROBOHUB_OK);
    assert(robohub_register_onewire("onewire0", &onewire) == ROBOHUB_OK);

    rh_u32 ok = 0u;
    rh_u32 fault = 0u;
    assert(robohub_core_smoke(&timer, ROBOHUB_PROBE_ALL, &ok, &fault) == ROBOHUB_OK);

    assert((ok & ROBOHUB_PROBE_CONSOLE) != 0u);
    assert((ok & ROBOHUB_PROBE_TIMER) != 0u);
    assert((ok & ROBOHUB_PROBE_GPIO) != 0u);
    assert((ok & ROBOHUB_PROBE_ONEWIRE) != 0u);
    assert(fault == 0u);

    /* A bus nobody registered is absent: in neither mask. */
    assert((ok & ROBOHUB_PROBE_I2C) == 0u && (fault & ROBOHUB_PROBE_I2C) == 0u);
    assert((ok & ROBOHUB_PROBE_SPI) == 0u && (fault & ROBOHUB_PROBE_SPI) == 0u);
    assert((ok & ROBOHUB_PROBE_UART) == 0u && (fault & ROBOHUB_PROBE_UART) == 0u);
    /* And the network is not the runtime's to answer. */
    assert((ok & ROBOHUB_PROBE_NET) == 0u && (fault & ROBOHUB_PROBE_NET) == 0u);

    /* A clock that goes backwards is a fault, not a pass. */
    RoboHubTimer broken = {0};
    broken.name = "broken";
    broken.get_millis = backward_millis;
    broken.delay_ms = no_delay;

    ok = 0u;
    fault = 0u;
    assert(robohub_core_smoke(&broken, ROBOHUB_PROBE_TIMER, &ok, &fault) == ROBOHUB_OK);
    assert((fault & ROBOHUB_PROBE_TIMER) != 0u);
    assert((ok & ROBOHUB_PROBE_TIMER) == 0u);

    /* No timer at all is absent rather than a fault. */
    ok = 0u;
    fault = 0u;
    assert(robohub_core_smoke(0, ROBOHUB_PROBE_TIMER, &ok, &fault) == ROBOHUB_OK);
    assert((ok & ROBOHUB_PROBE_TIMER) == 0u && (fault & ROBOHUB_PROBE_TIMER) == 0u);

    /* The counters are the runtime's own. */
    robohub_core_diagnostics_reset();
    robohub_core_diagnostics before;
    robohub_core_diagnostics_get(&before);
    assert(before.events_published == 0u);
    assert(before.last_error == ROBOHUB_OK);

    robohub_registry_deinit();
    return 0;
}
