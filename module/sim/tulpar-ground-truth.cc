/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-ground-truth.h"

namespace ns3
{
namespace tulpar
{

void
GroundTruthStore::AddVehicle(const std::string& name,
                             const std::string& vehicleClass,
                             bool attacker)
{
    m_vehicles[name] = {vehicleClass, attacker};
    m_log << "t=0 gt=vehicle name=" << name << " class=" << vehicleClass
          << " attacker=" << (attacker ? 1 : 0) << '\n';
}

bool
GroundTruthStore::IsAttacker(const std::string& name) const
{
    auto it = m_vehicles.find(name);
    return it != m_vehicles.end() && it->second.second;
}

void
GroundTruthStore::LogPosition(int64_t timeNs,
                              const std::string& name,
                              int64_t xCm,
                              int64_t yCm,
                              int64_t zCm)
{
    m_log << "t=" << timeNs << " gt=position name=" << name << " x_cm=" << xCm << " y_cm=" << yCm
          << " z_cm=" << zCm << '\n';
}

std::string
GroundTruthStore::Text() const
{
    return m_log.str();
}

} // namespace tulpar
} // namespace ns3
