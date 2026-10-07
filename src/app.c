/*
 * app.c — the application lifecycle.
 *
 * The table is read in registration order for init and start, and in reverse
 * for stop: what was brought up last is taken down first, which is the order
 * that lets an application depend on one registered before it.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#include "robohub/core/app.h"
#include "robohub/core/config.h"
#include "core_internal.h"

static const robohub_core_app* apps[RH_CORE_MAX_APPS];
static rh_u32 app_count;

RoboHubError robohub_core_app_register(const robohub_core_app* app)
{
    if (app == 0 || app->name == 0) {
        return ROBOHUB_ERR_INVALID_ARG;
    }
    if (app_count >= RH_CORE_MAX_APPS) {
        return ROBOHUB_ERR_NO_MEM;
    }
    apps[app_count++] = app;
    return ROBOHUB_OK;
}

rh_u32 robohub_core_app_count(void)
{
    return app_count;
}

void robohub_core_app_reset(void)
{
    for (rh_u32 i = 0; i < RH_CORE_MAX_APPS; i++) {
        apps[i] = 0;
    }
    app_count = 0u;
}

RoboHubError robohub_core_app_init(void)
{
    for (rh_u32 i = 0; i < app_count; i++) {
        if (apps[i]->init != 0) {
            RoboHubError err = apps[i]->init();
            if (ROBOHUB_FAILED(err)) {
                robohub_core_diag_state()->last_error = err;
                return err;
            }
        }
    }
    return ROBOHUB_OK;
}

RoboHubError robohub_core_app_start(void)
{
    for (rh_u32 i = 0; i < app_count; i++) {
        if (apps[i]->start != 0) {
            RoboHubError err = apps[i]->start();
            if (ROBOHUB_FAILED(err)) {
                robohub_core_diag_state()->last_error = err;
                return err;
            }
        }
    }
    return ROBOHUB_OK;
}

void robohub_core_app_step(rh_u32 now_ms)
{
    robohub_core_diagnostics* diag = robohub_core_diag_state();
    for (rh_u32 i = 0; i < app_count; i++) {
        if (apps[i]->step != 0) {
            apps[i]->step(now_ms);
            diag->app_steps++;
        }
    }
}

void robohub_core_app_stop(void)
{
    for (rh_u32 i = app_count; i > 0u; i--) {
        if (apps[i - 1u]->stop != 0) {
            apps[i - 1u]->stop();
        }
    }
}
