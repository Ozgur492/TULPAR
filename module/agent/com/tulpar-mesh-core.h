/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_MESH_CORE_H
#define TULPAR_MESH_CORE_H

#include <cstdint>

namespace ns3::tulpar
{

/** HELLO fields shared by the mesh core and its transport adapter. */
struct MeshHello
{
    uint16_t sender;
    uint32_t sequence;
};

/** Transport-independent state for the existing overlay HELLO flow. */
class MeshCore
{
  public:
    explicit MeshCore(uint16_t self);
    MeshHello MakeHello();
    bool AcceptsHello(const MeshHello& hello) const;
    uint32_t GetSentCount() const;

  private:
    uint16_t m_self;
    uint32_t m_nextSequence{0};
};

} // namespace ns3::tulpar

#endif // TULPAR_MESH_CORE_H
