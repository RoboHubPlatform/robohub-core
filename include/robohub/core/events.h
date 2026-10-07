/**
 * @file events.h
 * @brief Event identities for the RoboHub application runtime.
 *
 * An event is a small number, and the number is the whole of the agreement
 * between a publisher and a subscriber: a sensor publishes one, a telemetry
 * component subscribes to it, and neither knows the other exists. That is the
 * point of a bus — the two are joined by the identity and not by a call.
 *
 * The numbers are laid out in ranges so that two subsystems cannot collide
 * without meaning to. The framework owns the low range; each layer of the
 * ecosystem owns a block above it; and a module that does not fit may take one
 * from the module range, or from the user range when it is an application's own.
 * A layer defines its events with RH_EVENT_ID and its own base, so the catalog
 * is the sum of what the layers declare rather than one file that knows every
 * device in the world.
 *
 * @project RoboHub Platform
 * @author Aurel Miron
 * @version 0.1
 * @date 2026
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_ROBOHUB_CORE_EVENTS_H
#define INCLUDE_ROBOHUB_CORE_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include "types.h"

/* -------------------------------------------------------------------------- */
/*  Type                                                                      */
/* -------------------------------------------------------------------------- */

/** The identity of an event: a number both sides agree on. */
typedef rh_u16 robohub_event_id;

/* -------------------------------------------------------------------------- */
/*  Framework events (0x0000 - 0x00FF)                                        */
/* -------------------------------------------------------------------------- */
#define RH_EVENT_NONE           ((robohub_event_id)0x0000u)
#define RH_EVENT_CORE_INIT      ((robohub_event_id)0x0001u)
#define RH_EVENT_CORE_START     ((robohub_event_id)0x0002u)
#define RH_EVENT_CORE_TICK      ((robohub_event_id)0x0003u)
#define RH_EVENT_CORE_STOP      ((robohub_event_id)0x0004u)
#define RH_EVENT_CORE_ERROR     ((robohub_event_id)0x0005u)

/* -------------------------------------------------------------------------- */
/*  Domain ranges                                                             */
/* -------------------------------------------------------------------------- */
/* A layer takes the block below its name and numbers within it. The numbering
 * is the layer's own; what the runtime fixes is only where each layer's numbers
 * begin, so that the ranges cannot overlap by accident. */
#define RH_EVENT_DOMAIN_FRAMEWORK ((robohub_event_id)0x0000u)
#define RH_EVENT_DOMAIN_HAL       ((robohub_event_id)0x0100u)
#define RH_EVENT_DOMAIN_SENSOR    ((robohub_event_id)0x0200u)
#define RH_EVENT_DOMAIN_ACTUATOR  ((robohub_event_id)0x0300u)
#define RH_EVENT_DOMAIN_NETWORK   ((robohub_event_id)0x0400u)
#define RH_EVENT_DOMAIN_APP       ((robohub_event_id)0x0500u)
#define RH_EVENT_DOMAIN_MODULE    ((robohub_event_id)0x0600u)
#define RH_EVENT_DOMAIN_USER      ((robohub_event_id)0x8000u)

/* An event an index below a domain. RH_EVENT_ID(RH_EVENT_DOMAIN_SENSOR, 1) is
 * the first sensor event; the layer that owns the domain says what it means. */
#define RH_EVENT_ID(domain, index) \
    ((robohub_event_id)((rh_u32)(domain) + (rh_u32)(index)))

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ROBOHUB_CORE_EVENTS_H */
