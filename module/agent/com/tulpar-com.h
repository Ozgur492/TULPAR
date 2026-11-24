/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_COM_H
#define TULPAR_COM_H

#include "tulpar-agent-module.h"
#include "tulpar-mesh-core.h"

#include "ns3/event-id.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * M-COM, communication (owner: communication module).
 *
 * Phase 0 skeleton: a periodic neighbor beacon over UDP broadcast with a random
 * start offset and jitter, so that the event log depends on the run's random
 * streams. The mesh overlay of RSD Section 4.3 replaces it in Phase 1.
 */
class ComModule : public AgentModule
{
  public:
    /// UDP port of the skeleton beacon.
    static constexpr uint16_t kBeaconPort = 9100;

    /**
     * Constructor.
     *
     * @param self identity of the own vehicle
     * @param sink event sink of the run
     * @param node the ns-3 node of the own vehicle
     * @param jitter random variable for beacon offset and jitter; its stream is
     *        assigned by the harness from the run seed (RSD-SIM-01c)
     * @param period beacon period
     */
    ComModule(VehicleIdentity self,
              EventSink& sink,
              Ptr<Node> node,
              Ptr<UniformRandomVariable> jitter,
              Time period);

    std::string Code() const override;
    void Start() override;
    void Stop() override;

  private:
    /// Send one beacon and schedule the next one.
    void SendBeacon();

    /**
     * Receive beacons from the socket.
     * @param socket the receiving socket
     */
    void HandleRead(Ptr<Socket> socket);

    Ptr<Node> m_node;                    ///< own node
    Ptr<UniformRandomVariable> m_jitter; ///< offset and jitter in seconds, [0, 1)
    Time m_period;                       ///< beacon period
    Ptr<Socket> m_socket;                ///< broadcast socket
    EventId m_next;                      ///< next scheduled beacon
    MeshCore m_mesh;                     ///< socket-independent overlay state
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_COM_H
