/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_AGENT_MODULE_H
#define TULPAR_AGENT_MODULE_H

#include "tulpar-event-sink.h"

#include <cstdint>
#include <string>
#include <utility>

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * Identity of one vehicle as the agent on that vehicle knows it.
 */
struct VehicleIdentity
{
    uint16_t index{0}; ///< index in the mission roster
    std::string name;  ///< roster name, for example "UAV-1"
};

/**
 * @ingroup tulpar
 * Common base of the eight agent modules (M-COM, M-DEL, M-CRY, M-EVS, M-OBS,
 * M-SEC, M-MIS, M-LSC).
 *
 * Modules talk to each other only through the interfaces of RSD Table 4.25, which
 * the agent root wires together; a module never includes another module's
 * implementation headers (checked by tools/check_static.py).
 */
class AgentModule
{
  public:
    /**
     * Constructor.
     *
     * @param self identity of the vehicle that runs the module
     * @param sink event sink of the run
     */
    AgentModule(VehicleIdentity self, EventSink& sink)
        : m_self(std::move(self)),
          m_sink(sink)
    {
    }

    virtual ~AgentModule() = default;

    /**
     * Short module code used in the event log, for example "COM".
     *
     * @return the module code
     */
    virtual std::string Code() const = 0;

    /**
     * Start the module at the beginning of the mission.
     */
    virtual void Start() = 0;

    /**
     * Stop the module at the end of the mission.
     */
    virtual void Stop()
    {
    }

  protected:
    /**
     * Create an event stamped with the current simulation time, this vehicle and
     * this module.
     *
     * @param type event type
     * @return the event, ready for further fields
     */
    Event MakeEvent(const std::string& type) const;

    /**
     * Record an event in the run's event sink.
     *
     * @param event the event
     */
    void Emit(const Event& event) const
    {
        m_sink.Record(event);
    }

    VehicleIdentity m_self; ///< identity of the own vehicle
    EventSink& m_sink;      ///< event sink of the run
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_AGENT_MODULE_H
