/*
 * test_osal.c — the bare-metal OSAL defaults.
 *
 * With no middleware linked, the runtime is single-threaded: no threads, no
 * locking, and a delay that is a no-op when no clock has been registered.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/osal.h"

#include <assert.h>

int main(void)
{
    /* No operating system beneath. */
    assert(robohub_osal_rtos_present() == RH_FALSE);

    /* There are no threads to create. */
    robohub_thread_t thread = 0;
    robohub_osal_thread_config config = {
        .name = "t",
        .entry = 0,
        .arg = 0,
        .stack_size = 1024u,
        .priority = 1u,
    };
    assert(robohub_osal_thread_create(&thread, &config) == -1);
    assert(thread == 0);

    /* A mutex is a handle that locks nothing. */
    robohub_mutex_t mutex = robohub_osal_mutex_create();
    assert(mutex != 0);
    robohub_osal_mutex_lock(mutex);
    robohub_osal_mutex_unlock(mutex);
    robohub_osal_mutex_destroy(mutex);

    /* No clock is registered here, so time does nothing and reads zero. */
    robohub_osal_sleep_ms(10u);
    assert(robohub_osal_now_ms() == 0u);

    /* Starting a scheduler there is not one returns and lets the caller run. */
    robohub_osal_start();

    return 0;
}
