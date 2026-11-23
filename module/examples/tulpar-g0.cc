/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

/**
 * @file
 * @ingroup tulpar
 * Phase 0 skeleton run: writes the agent event log and the ground-truth log of one
 * run and prints its trace hash. scripts/g0_checks.sh runs it twice with the same
 * seed (RSD-SIM-01d, gate I-07) and with two hidden attacker labels (RSD-SIM-02b).
 */

#include "ns3/command-line.h"
#include "ns3/tulpar-scenario.h"

#include <fstream>
#include <iomanip>
#include <iostream>

using namespace ns3;

int
main(int argc, char* argv[])
{
    tulpar::ScenarioConfig config;
    std::string eventLogPath = "tulpar-events.log";
    std::string groundTruthPath = "tulpar-ground-truth.log";

    CommandLine cmd(__FILE__);
    cmd.AddValue("seed", "Run seed", config.seed);
    cmd.AddValue("run", "Run number", config.run);
    cmd.AddValue("duration", "Mission duration in seconds", config.durationS);
    cmd.AddValue("attacker", "Hidden attacker label: a vehicle name or none", config.attacker);
    cmd.AddValue("eventLog", "Path of the agent event log", eventLogPath);
    cmd.AddValue("groundTruthLog", "Path of the ground-truth log", groundTruthPath);
    cmd.Parse(argc, argv);

    tulpar::RunResult result = tulpar::RunScenario(config);

    std::ofstream(eventLogPath, std::ios::binary) << result.eventLog;
    std::ofstream(groundTruthPath, std::ios::binary) << result.groundTruthLog;

    std::cout << "seed=" << config.seed << " run=" << config.run << " attacker=" << config.attacker
              << " events=" << result.events << " tx=" << result.tx << " rx=" << result.rx
              << " trace_hash=" << std::hex << std::setw(16) << std::setfill('0')
              << result.traceHash << std::dec << std::endl;
    return 0;
}
