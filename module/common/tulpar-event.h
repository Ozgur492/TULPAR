/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_EVENT_H
#define TULPAR_EVENT_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ns3
{
namespace tulpar
{

/**
 * @defgroup tulpar TULPAR
 * Distributed UAV-USV collective (TULPAR graduation project).
 */

/**
 * @ingroup tulpar
 * One line of the agent event log.
 *
 * The log is the evidence base of the verification gates (RSD Chapter 5), so the
 * text form must be identical for identical runs: simulation time in integer
 * nanoseconds, fields in insertion order, no pointers and no wall-clock values.
 */
struct Event
{
    int64_t timeNs{0};   ///< simulation time in nanoseconds
    std::string vehicle; ///< vehicle name, for example "UAV-1"
    std::string module;  ///< agent module that produced the event, for example "COM"
    std::string type;    ///< event type, for example "TX"
    std::vector<std::pair<std::string, std::string>> fields; ///< further key=value fields

    /**
     * Add a field.
     *
     * @param key field name (no spaces)
     * @param value field value (no spaces)
     * @return this event, for chaining
     */
    Event& With(const std::string& key, const std::string& value);

    /**
     * Add an integer field.
     *
     * @param key field name (no spaces)
     * @param value field value
     * @return this event, for chaining
     */
    Event& With(const std::string& key, int64_t value);

    /**
     * Text form of the event, one line without the line break.
     *
     * @return the formatted line
     */
    std::string ToLine() const;
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_EVENT_H
