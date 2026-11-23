/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_EVENT_SINK_H
#define TULPAR_EVENT_SINK_H

#include "tulpar-event.h"

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * Write-only destination of agent events.
 *
 * Agents receive a reference to a sink from the simulation harness (M-SIM). The
 * interface has no read method, so an agent can report what it did but can never
 * read the log, the ground truth or another agent's events through it
 * (RSD-SIM-02a, MD9).
 */
class EventSink
{
  public:
    virtual ~EventSink() = default;

    /**
     * Record one event.
     *
     * @param event the event; its time must be the current simulation time
     */
    virtual void Record(const Event& event) = 0;
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_EVENT_SINK_H
