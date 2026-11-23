/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_AGENT_H
#define TULPAR_AGENT_H

#include "tulpar-agent-module.h"

#include "ns3/application.h"
#include "ns3/nstime.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * The TULPAR software on one vehicle: the eight agent modules and their wiring.
 *
 * The agent receives only its roster identity, the run's write-only event sink and
 * the stream number of its random variables. It has no handle to the scenario
 * controller or to the ground-truth store (RSD-SIM-02a).
 */
class TulparAgent : public Application
{
  public:
    /**
     * Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    TulparAgent();
    ~TulparAgent() override;

    /**
     * Configure the agent before the simulation starts.
     *
     * @param self roster identity of the own vehicle
     * @param sink event sink of the run; must outlive the simulation
     * @param stream stream number of the agent's random variables, assigned by
     *        the harness from the run seed (RSD-SIM-01c)
     */
    void Configure(VehicleIdentity self, EventSink* sink, int64_t stream);

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    VehicleIdentity m_self;                              ///< own identity
    EventSink* m_sink{nullptr};                          ///< event sink of the run
    int64_t m_stream{-1};                                ///< stream number of the agent
    Time m_beaconPeriod;                                 ///< skeleton beacon period
    std::vector<std::unique_ptr<AgentModule>> m_modules; ///< modules in start order
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_AGENT_H
