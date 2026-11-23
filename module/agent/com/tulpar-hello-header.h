/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_HELLO_HEADER_H
#define TULPAR_HELLO_HEADER_H

#include "ns3/header.h"

#include <cstdint>

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * Header of the skeleton neighbor beacon.
 *
 * Placeholder for the Phase 1 HELLO of the mesh overlay (RSD-NET-01); it carries
 * only the sender's roster index and a sequence number. The byte-level encodings
 * of records and messages are frozen in Phase 1.
 */
class HelloHeader : public Header
{
  public:
    /**
     * Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;

    /**
     * Set the sender's roster index.
     * @param index roster index
     */
    void SetSender(uint16_t index);

    /**
     * Get the sender's roster index.
     * @return roster index
     */
    uint16_t GetSender() const;

    /**
     * Set the sequence number.
     * @param seq sequence number
     */
    void SetSeq(uint32_t seq);

    /**
     * Get the sequence number.
     * @return sequence number
     */
    uint32_t GetSeq() const;

  private:
    uint16_t m_sender{0}; ///< sender roster index
    uint32_t m_seq{0};    ///< sequence number
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_HELLO_HEADER_H
