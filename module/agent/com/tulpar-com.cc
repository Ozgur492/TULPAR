/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-com.h"

#include "tulpar-hello-header.h"

#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TulparCom");

namespace tulpar
{

ComModule::ComModule(VehicleIdentity self,
                     EventSink& sink,
                     Ptr<Node> node,
                     Ptr<UniformRandomVariable> jitter,
                     Time period)
    : AgentModule(std::move(self), sink),
      m_node(node),
      m_jitter(jitter),
      m_period(period),
      m_mesh(m_self.index)
{
}

std::string
ComModule::Code() const
{
    return "COM";
}

void
ComModule::Start()
{
    NS_LOG_FUNCTION(this);
    m_socket = Socket::CreateSocket(m_node, UdpSocketFactory::GetTypeId());
    m_socket->SetAllowBroadcast(true);
    m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), kBeaconPort));
    m_socket->SetRecvCallback(MakeCallback(&ComModule::HandleRead, this));
    Emit(MakeEvent("START"));

    Time offset = Seconds(m_jitter->GetValue() * m_period.GetSeconds());
    m_next = Simulator::Schedule(offset, &ComModule::SendBeacon, this);
}

void
ComModule::Stop()
{
    NS_LOG_FUNCTION(this);
    m_next.Cancel();
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket = nullptr;
    }
    Emit(MakeEvent("STOP").With("sent", static_cast<int64_t>(m_mesh.GetSentCount())));
}

void
ComModule::SendBeacon()
{
    const MeshHello frame = m_mesh.MakeHello();
    HelloHeader hello;
    hello.SetSender(frame.sender);
    hello.SetSeq(frame.sequence);
    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(hello);
    m_socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::GetBroadcast(), kBeaconPort));
    Emit(MakeEvent("TX").With("seq", static_cast<int64_t>(frame.sequence)));

    // Up to 10 % jitter keeps neighboring beacons from synchronizing.
    Time next = m_period + Seconds(0.1 * m_period.GetSeconds() * m_jitter->GetValue());
    m_next = Simulator::Schedule(next, &ComModule::SendBeacon, this);
}

void
ComModule::HandleRead(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> packet;
    while ((packet = socket->RecvFrom(from)))
    {
        HelloHeader hello;
        if (packet->GetSize() < hello.GetSerializedSize())
        {
            Emit(MakeEvent("RX_DROP").With("reason", "short"));
            continue;
        }
        packet->RemoveHeader(hello);
        const MeshHello frame{hello.GetSender(), hello.GetSeq()};
        if (!m_mesh.AcceptsHello(frame))
        {
            continue;
        }
        Emit(MakeEvent("RX")
                 .With("from", static_cast<int64_t>(frame.sender))
                 .With("seq", static_cast<int64_t>(frame.sequence)));
    }
}

} // namespace tulpar
} // namespace ns3
