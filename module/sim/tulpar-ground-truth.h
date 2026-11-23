/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_GROUND_TRUTH_H
#define TULPAR_GROUND_TRUTH_H

#include <cstdint>
#include <map>
#include <sstream>
#include <string>

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * Ground truth of a run, held by the simulation harness (M-SIM) only.
 *
 * Holds what the agents must never see: the hidden attacker labels and the true
 * vehicle states. No agent target links or includes this header (RSD-SIM-02a, MD9).
 */
class GroundTruthStore
{
  public:
    /**
     * Register a vehicle.
     *
     * @param name roster name
     * @param vehicleClass vehicle class, "UAV" or "USV"
     * @param attacker hidden attacker label
     */
    void AddVehicle(const std::string& name, const std::string& vehicleClass, bool attacker);

    /**
     * Hidden attacker label of a vehicle.
     *
     * @param name roster name
     * @return true if the scenario labels the vehicle as an attacker
     */
    bool IsAttacker(const std::string& name) const;

    /**
     * Log the true position of a vehicle, in centimeters.
     *
     * @param timeNs simulation time in nanoseconds
     * @param name roster name
     * @param xCm x coordinate in cm
     * @param yCm y coordinate in cm
     * @param zCm z coordinate in cm
     */
    void LogPosition(int64_t timeNs,
                     const std::string& name,
                     int64_t xCm,
                     int64_t yCm,
                     int64_t zCm);

    /**
     * Text of the ground-truth log.
     *
     * @return the log, one line per entry
     */
    std::string Text() const;

  private:
    std::map<std::string, std::pair<std::string, bool>> m_vehicles; ///< name -> (class, attacker)
    std::ostringstream m_log;                                       ///< ground-truth log
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_GROUND_TRUTH_H
