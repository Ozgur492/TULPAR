/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_SCENARIO_H
#define TULPAR_SCENARIO_H

#include <cstdint>
#include <string>

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * Configuration of one run of the Phase 0 skeleton scenario.
 *
 * This header belongs to the simulation harness (M-SIM). Agent code must not
 * include it (tools/check_static.py, RSD-SIM-02a).
 */
struct ScenarioConfig
{
    uint32_t seed{1};             ///< run seed (RSD-SIM-01c)
    uint64_t run{1};              ///< run number; separates the MRG32k3a substreams
    double durationS{60.0};       ///< mission duration in seconds
    std::string attacker{"none"}; ///< hidden attacker label: a vehicle name or "none"
};

/**
 * @ingroup tulpar
 * Outputs of one run.
 */
struct RunResult
{
    std::string eventLog;       ///< agent event log, one event per line
    std::string groundTruthLog; ///< ground-truth log written by the harness only
    uint64_t events{0};         ///< number of agent events
    uint64_t tx{0};             ///< number of beacons sent
    uint64_t rx{0};             ///< number of beacons received
    uint64_t traceHash{0};      ///< 64-bit hash of the agent event log
};

/**
 * @ingroup tulpar
 * Run the Phase 0 skeleton scenario: UAV-1 and USV-1 with all eight agent modules,
 * an 802.11n ad hoc channel and the skeleton neighbor beacon.
 *
 * The function resets the random number generator, the stream counter and the
 * name registry, runs the simulation and destroys it, so that two calls with the
 * same configuration in one process produce identical results.
 *
 * @param config run configuration
 * @return the run outputs
 */
RunResult RunScenario(const ScenarioConfig& config);

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_SCENARIO_H
