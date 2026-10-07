/*
 * osal_baremetal.c — the OSAL with no operating system beneath it.
 *
 * Every function here is weak, so a middleware that implements the contract
 * strongly replaces it and the linker never looks back. With no middleware the
 * program is single-threaded, which is what these say: a thread cannot be
 * created because there is no scheduler, a mutex locks nothing because there is
 * nothing to lock against, and the delay is the hardware timer the platform
 * registered.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/osal.h"
#include "robohub/core/config.h"
#include "registry.h"
#include "robohub/hal/timer.h"

#if defined(__GNUC__) || defined(__clang__)
#define RH_OSAL_WEAK __attribute__((weak))
#else
#define RH_OSAL_WEAK
#endif

/* The delay needs a clock, and the platform's is the one the runtime uses. It
 * is looked up rather than kept, because this file may run before the runtime
 * has taken it. */
static RoboHubTimer* osal_timer(void)
{
    return robohub_get_timer(RH_CORE_TIMER_NAME);
}

RH_OSAL_WEAK rh_bool robohub_osal_rtos_present(void)
{
    return RH_FALSE;
}

RH_OSAL_WEAK void robohub_osal_start(void)
{
    /* There is no scheduler to start; the caller keeps the thread it already
     * has. The runtime relies on this returning, and then runs its own loop. */
}

RH_OSAL_WEAK int robohub_osal_thread_create(robohub_thread_t* thread,
                                            const robohub_osal_thread_config* config)
{
    (void)thread;
    (void)config;
    return -1; /* no threads without a kernel */
}

RH_OSAL_WEAK void robohub_osal_sleep_ms(rh_u32 ms)
{
    RoboHubTimer* timer = osal_timer();
    if (timer != 0 && timer->delay_ms != 0) {
        timer->delay_ms(timer, ms);
    }
}

RH_OSAL_WEAK rh_u32 robohub_osal_now_ms(void)
{
    RoboHubTimer* timer = osal_timer();
    if (timer == 0 || timer->get_millis == 0) {
        return 0u;
    }
    return (rh_u32)timer->get_millis(timer);
}

RH_OSAL_WEAK robohub_mutex_t robohub_osal_mutex_create(void)
{
    /* A handle that is not null, so a caller that checks gets a usable one; it
     * locks nothing, which is correct with one thread. */
    return (robohub_mutex_t)1;
}

RH_OSAL_WEAK void robohub_osal_mutex_destroy(robohub_mutex_t mutex)
{
    (void)mutex;
}

RH_OSAL_WEAK void robohub_osal_mutex_lock(robohub_mutex_t mutex)
{
    (void)mutex;
}

RH_OSAL_WEAK void robohub_osal_mutex_unlock(robohub_mutex_t mutex)
{
    (void)mutex;
}
