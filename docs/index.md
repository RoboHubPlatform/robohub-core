# Core

The RoboHub application runtime: the layer a firmware program is built on when
it is more than one thing. Modules meet on an event bus, periodic work is
declared once, and an application is a unit with a beginning and an end — so a
program whose sensor calls are wired straight into its motor control gets a
place to add telemetry without touching either.

It is platform-agnostic: it reaches hardware only through the `robohub-common`
contracts (see {ref}`robohub-common/index`) and takes its clock from a
`RoboHubTimer` the platform registered, so the same runtime runs on a part or on
a host. `robohub-telemetry` and the device runtime are built on it.

## What it provides

| Piece | Header | What it is |
|---|---|---|
| Event bus | `robohub/core/event.h` | Publish and subscribe between modules, zero-copy and synchronous. A sensor publishes a reading; whoever cares handles it; neither names the other |
| Event catalog | `robohub/core/events.h` | An event is a number. The framework owns the low range, each layer of the ecosystem a block, and a module defines its events with `RH_EVENT_ID(domain, index)` |
| Scheduler | `robohub/core/scheduler.h` | Cooperative periodic tasks: a function and a period, run when due in one loop, with no stack per task and no locking |
| Application lifecycle | `robohub/core/app.h` | Applications registered with `robohub_core_app_register` before the runtime starts: every `init` before any `start`, `stop` in reverse |
| Diagnostics | `robohub/core/diagnostics.h` | Counters (events, deliveries, handler errors, tasks, steps, uptime) and the last error — what turns "it stopped" into "the third task has been failing since boot" |

## Example

```c
#include "robohub/core/core.h"
#include "robohub/core/event.h"

#define EVENT_TEMP_READING RH_EVENT_ID(RH_EVENT_DOMAIN_SENSOR, 1)

static RoboHubError on_temperature(robohub_event_id id, const void* payload,
                                   rh_size size, void* user)
{
    (void)id; (void)size; (void)user;
    float celsius = *(const float*)payload;   /* valid for this call only */
    return ROBOHUB_OK;
}

static RoboHubError read_temperature(robohub_core_task_id id, void* user)
{
    (void)id; (void)user;
    float celsius = 21.5f;                     /* a real driver would read it */
    return robohub_core_event_publish(EVENT_TEMP_READING, &celsius, sizeof celsius);
}

int main(void)
{
    robohub_platform_init();

    robohub_core_event_subscribe(EVENT_TEMP_READING, on_temperature, 0);
    robohub_core_task_add("temp", read_temperature, 0, 1000u, 0);

    robohub_core_init(0);
    robohub_core_run();
    return 0;
}
```

A payload is valid only for the duration of the handler call: the bus does not
copy it.

## Layout

- `include/robohub/core/` — the public headers.
- `src/` — the sources and the private header they share.
- `tests/` — host tests, run with `ctest`.

Generated headers live in the build tree, never in the source tree.
