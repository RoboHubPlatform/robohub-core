/*
 * test_core.c — the lifecycle contract: apps register, and init needs a clock.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/core.h"
#include "robohub/core/app.h"

#include <assert.h>
#include <string.h>

static int inits;

static RoboHubError app_init(void)
{
    inits++;
    return ROBOHUB_OK;
}

static const robohub_core_app app = {
    .name = "test",
    .init = app_init,
};

int main(void)
{
    assert(strcmp(robohub_core_version(), "0.1.0") == 0);
    assert(robohub_core_running() == RH_FALSE);

    assert(robohub_core_app_register(&app) == ROBOHUB_OK);
    assert(robohub_core_app_count() == 1);
    assert(robohub_core_app_register(0) == ROBOHUB_ERR_INVALID_ARG);

    /* No platform registered a timer, so there is no clock and init must say
     * so rather than run a loop that cannot measure time. */
    assert(robohub_core_init(0) == ROBOHUB_ERR_NOT_FOUND);
    assert(inits == 0);

    return 0;
}
