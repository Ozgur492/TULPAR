/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-hello-header.h"

namespace ns3
{
namespace tulpar
{

NS_OBJECT_ENSURE_REGISTERED(HelloHeader);

TypeId
HelloHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::tulpar::HelloHeader")
                            .SetParent<Header>()
                            .SetGroupName("Tulpar")
                            .AddConstructor<HelloHeader>();
    return tid;
}

TypeId
HelloHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
HelloHeader::GetSerializedSize() const
{
    return 6;
}

void
HelloHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU16(m_sender);
    start.WriteHtonU32(m_seq);
}

uint32_t
HelloHeader::Deserialize(Buffer::Iterator start)
{
    m_sender = start.ReadNtohU16();
    m_seq = start.ReadNtohU32();
    return GetSerializedSize();
}

void
HelloHeader::Print(std::ostream& os) const
{
    os << "sender=" << m_sender << " seq=" << m_seq;
}

void
HelloHeader::SetSender(uint16_t index)
{
    m_sender = index;
}

uint16_t
HelloHeader::GetSender() const
{
    return m_sender;
}

void
HelloHeader::SetSeq(uint32_t seq)
{
    m_seq = seq;
}

uint32_t
HelloHeader::GetSeq() const
{
    return m_seq;
}

} // namespace tulpar
} // namespace ns3
