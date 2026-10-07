# robohub-core

The RoboHub application runtime: the layer a program is built on when it is more
than one thing.

A program whose sensor calls are wired straight into its motor control has no
place to add telemetry without touching both. `robohub-core` is where that
coupling goes instead: modules meet on an event bus, periodic work is declared
once, and an application is a unit with a beginning and an end.

## What it provides

- **Event bus** — publish and subscribe between modules, zero-copy and
  synchronous. A sensor publishes a reading; whoever cares handles it; neither
  names the other.
- **Scheduler** — cooperative periodic tasks. A task is a function and a period;
  the runtime runs it when it is due, in one loop, with no stack per task and no
  locking.
- **Application lifecycle** — every `init` before any `start`, `stop` in
  reverse.
- **Diagnostics** — counters (events, deliveries, handler errors, tasks, steps,
  uptime) and the last error, which is what turns "it stopped" into "the third
  task has been failing since boot".

It is platform-agnostic: it reaches hardware only through the `robohub-common`
contracts and takes its clock from a `RoboHubTimer` the platform registered.

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

An event is a number, and the ranges in `robohub/core/events.h` keep two
subsystems from colliding: the framework owns the low range, each layer of the
ecosystem owns a block, and a module defines its events with `RH_EVENT_ID`.

## Layout

- `include/robohub/core/` — the public headers.
- `src/` — the sources and the private header they share.
- `tests/` — host tests, run with `ctest`.

Generated headers live in the build tree, never here.

## License

MIT. See `LICENSE`.
