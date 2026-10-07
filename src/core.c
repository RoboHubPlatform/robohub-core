/*
 * core.c — the application runtime.
 *
 * Bring the applications up, hand them a clock, loop, take them down. The loop
 * is the one place the runtime decides anything: it publishes the tick, runs
 * what is due, steps the applications, and then sleeps until the next of those
 * is due — no further, so the console the delay services is serviced, and no
 * less, so a due task is not late.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/core.h"
#include "robohub/core/config.h"
#include "robohub/core/event.h"
#include "robohub/osal.h"
#include "registry.h"
#include "core_internal.h"

static RoboHubTimer* clock_timer;
static rh_bool initialized;
static rh_bool running;

RoboHubError robohub_core_init(const robohub_core_config* config)
{
    if (initialized) {
        return ROBOHUB_OK;
    }

    clock_timer = (config != 0 && config->timer != 0)
        ? config->timer
        : robohub_get_timer(RH_CORE_TIMER_NAME);
    if (clock_timer == 0) {
        return ROBOHUB_ERR_NOT_FOUND;
    }

    robohub_core_diagnostics_reset();
    (void)robohub_core_event_init();
    (void)robohub_core_scheduler_init();

    RoboHubError err = robohub_core_app_init();
    if (ROBOHUB_FAILED(err)) {
        return err;
    }

    initialized = RH_TRUE;
    (void)robohub_core_event_publish(RH_EVENT_CORE_INIT, 0, 0);
    return ROBOHUB_OK;
}

rh_u32 robohub_core_uptime_ms(void)
{
    if (clock_timer == 0 || clock_timer->get_millis == 0) {
        return 0u;
    }
    return (rh_u32)clock_timer->get_millis(clock_timer);
}

RoboHubError robohub_core_run(void)
{
    if (!initialized) {
        return ROBOHUB_ERR_STATE;
    }

    RoboHubError err = robohub_core_app_start();
    if (ROBOHUB_FAILED(err)) {
        robohub_core_app_stop();
        return err;
    }

    running = RH_TRUE;

    if (robohub_osal_rtos_present()) {
        /* With a kernel, the work becomes threads: every periodic task its own,
         * and one for the loop itself. The kernel is started last, and it does
         * not return, so this thread is handed over to it. */
        (void)robohub_core_event_publish(RH_EVENT_CORE_START, 0, 0);

        err = robohub_core_scheduler_start_threads();
        if (ROBOHUB_FAILED(err)) {
            running = RH_FALSE;
            robohub_core_app_stop();
            return err;
        }
        err = robohub_core_app_start_thread();
        if (ROBOHUB_FAILED(err)) {
            running = RH_FALSE;
            return err;
        }

        robohub_osal_start();
        /* A kernel that came back from start did not start. Keep this thread
         * alive rather than falling out of main, which would leave a board
         * with no program on it. */
        for (;;) {
            robohub_osal_sleep_ms(RH_CORE_TICK_MS);
        }
    }

    (void)robohub_core_event_publish(RH_EVENT_CORE_START, 0, 0);

    while (running) {
        rh_u32 now = robohub_core_uptime_ms();
        robohub_core_diag_state()->uptime_ms = now;

        (void)robohub_core_event_publish(RH_EVENT_CORE_TICK, &now, (rh_size)sizeof(now));
        robohub_core_scheduler_run(now);
        robohub_core_app_step(now);

        rh_u32 wait = robohub_core_scheduler_next_due(now);
        if (wait == RH_CORE_NO_DUE || wait > RH_CORE_TICK_MS) {
            wait = RH_CORE_TICK_MS;
        }
        if (wait == 0u) {
            wait = 1u;
        }
        clock_timer->delay_ms(clock_timer, wait);
    }

    robohub_core_app_stop();
    (void)robohub_core_event_publish(RH_EVENT_CORE_STOP, 0, 0);
    return ROBOHUB_OK;
}

void robohub_core_stop(void)
{
    running = RH_FALSE;
}

rh_bool robohub_core_running(void)
{
    return running;
}

const char* robohub_core_version(void)
{
    return "0.1.0";
}
