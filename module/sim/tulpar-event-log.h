/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_EVENT_LOG_H
#define TULPAR_EVENT_LOG_H

#include "tulpar-event-sink.h"

#include <cstdint>
#include <map>
#include <sstream>
#include <string>

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * Agent event log of a run (M-SIM side of the write-only event sink).
 */
class EventLog : public EventSink
{
  public:
    void Record(const Event& event) override;

    /**
     * Text of the log.
     * @return the log, one event per line
     */
    std::string Text() const;

    /**
     * Number of recorded events.
     * @return the event count
     */
    uint64_t Count() const;

    /**
     * Number of recorded events of one type.
     * @param type event type
     * @return the count
     */
    uint64_t Count(const std::string& type) const;

  private:
    std::ostringstream m_text;                ///< log text
    uint64_t m_count{0};                      ///< number of events
    std::map<std::string, uint64_t> m_byType; ///< events per type
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_EVENT_LOG_H
