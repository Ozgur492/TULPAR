/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-mesh-core.h"

namespace ns3::tulpar
{

MeshCore::MeshCore(uint16_t self)
    : m_self(self)
{
}

MeshHello
MeshCore::MakeHello()
{
    return {m_self, m_nextSequence++};
}

bool
MeshCore::AcceptsHello(const MeshHello& hello) const
{
    return hello.sender != m_self;
}

uint32_t
MeshCore::GetSentCount() const
{
    return m_nextSequence;
}

} // namespace ns3::tulpar
