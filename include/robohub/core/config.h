/*
 * config.h — build-time limits and names for the application runtime.
 *
 * A runtime on a microcontroller cannot grow: every table it holds is sized when
 * it is compiled. These are those sizes, and they come from Kconfig in a build
 * that has one and fall back to the defaults here in a build that does not —
 * which is what lets the same source run on a part and in a host test.
 *
 * @project RoboHub Platform
 * @license MIT
 */

#ifndef INCLUDE_ROBOHUB_CORE_CONFIG_H
#define INCLUDE_ROBOHUB_CORE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

/* The resolved configuration, when a build wrote one. A host test has none. */
#if defined(__has_include)
#  if __has_include("autoconf.h")
#    include "autoconf.h"
#  endif
#endif

/* How many event subscriptions the bus holds at once. */
#ifndef RH_CORE_MAX_SUBSCRIBERS
#  ifdef CONFIG_RH_CORE_MAX_SUBSCRIBERS
#    define RH_CORE_MAX_SUBSCRIBERS CONFIG_RH_CORE_MAX_SUBSCRIBERS
#  else
#    define RH_CORE_MAX_SUBSCRIBERS 16
#  endif
#endif

/* How many periodic tasks the scheduler holds at once. */
#ifndef RH_CORE_MAX_TASKS
#  ifdef CONFIG_RH_CORE_MAX_TASKS
#    define RH_CORE_MAX_TASKS CONFIG_RH_CORE_MAX_TASKS
#  else
#    define RH_CORE_MAX_TASKS 8
#  endif
#endif

/* How many applications the lifecycle holds at once. */
#ifndef RH_CORE_MAX_APPS
#  ifdef CONFIG_RH_CORE_MAX_APPS
#    define RH_CORE_MAX_APPS CONFIG_RH_CORE_MAX_APPS
#  else
#    define RH_CORE_MAX_APPS 4
#  endif
#endif

/* The longest a loop waits before looking for work again, in milliseconds. */
#ifndef RH_CORE_TICK_MS
#  ifdef CONFIG_RH_CORE_TICK_MS
#    define RH_CORE_TICK_MS CONFIG_RH_CORE_TICK_MS
#  else
#    define RH_CORE_TICK_MS 10u
#  endif
#endif

/* The timer the runtime takes its clock from, looked up in the registry. */
#ifndef RH_CORE_TIMER_NAME
#  ifdef CONFIG_RH_CORE_TIMER_NAME
#    define RH_CORE_TIMER_NAME CONFIG_RH_CORE_TIMER_NAME
#  else
#    define RH_CORE_TIMER_NAME "system_timer"
#  endif
#endif

/* What a periodic task is given when the runtime runs it as a thread. The
 * stack is in bytes; the priority is the kernel's own scale. */
#ifndef RH_CORE_TASK_STACK
#  ifdef CONFIG_RH_CORE_TASK_STACK
#    define RH_CORE_TASK_STACK CONFIG_RH_CORE_TASK_STACK
#  else
#    define RH_CORE_TASK_STACK 2048u
#  endif
#endif
#ifndef RH_CORE_TASK_PRIORITY
#  ifdef CONFIG_RH_CORE_TASK_PRIORITY
#    define RH_CORE_TASK_PRIORITY CONFIG_RH_CORE_TASK_PRIORITY
#  else
#    define RH_CORE_TASK_PRIORITY 1u
#  endif
#endif

/* What the runtime's own loop thread is given. It publishes the tick and steps
 * the applications, so it sits above the tasks it wakes. */
#ifndef RH_CORE_MAIN_STACK
#  ifdef CONFIG_RH_CORE_MAIN_STACK
#    define RH_CORE_MAIN_STACK CONFIG_RH_CORE_MAIN_STACK
#  else
#    define RH_CORE_MAIN_STACK 4096u
#  endif
#endif
#ifndef RH_CORE_MAIN_PRIORITY
#  ifdef CONFIG_RH_CORE_MAIN_PRIORITY
#    define RH_CORE_MAIN_PRIORITY CONFIG_RH_CORE_MAIN_PRIORITY
#  else
#    define RH_CORE_MAIN_PRIORITY 2u
#  endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_CORE_CONFIG_H */
