/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-agent-module.h"

#include "ns3/simulator.h"

namespace ns3
{
namespace tulpar
{

Event
AgentModule::MakeEvent(const std::string& type) const
{
    Event e;
    e.timeNs = Simulator::Now().GetNanoSeconds();
    e.vehicle = m_self.name;
    e.module = Code();
    e.type = type;
    return e;
}

} // namespace tulpar
} // namespace ns3
