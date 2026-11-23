/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-event-log.h"

namespace ns3
{
namespace tulpar
{

void
EventLog::Record(const Event& event)
{
    m_text << event.ToLine() << '\n';
    ++m_count;
    ++m_byType[event.type];
}

std::string
EventLog::Text() const
{
    return m_text.str();
}

uint64_t
EventLog::Count() const
{
    return m_count;
}

uint64_t
EventLog::Count(const std::string& type) const
{
    auto it = m_byType.find(type);
    return it == m_byType.end() ? 0 : it->second;
}

} // namespace tulpar
} // namespace ns3
