/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-scenario.h"

#include "tulpar-agent.h"
#include "tulpar-event-log.h"
#include "tulpar-ground-truth.h"

#include "ns3/abort.h"
#include "ns3/boolean.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/hash.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/names.h"
#include "ns3/node-container.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/wifi-helper.h"
#include "ns3/wifi-mac-helper.h"
#include "ns3/yans-wifi-helper.h"

#include <cmath>
#include <vector>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TulparScenario");

namespace tulpar
{

namespace
{

/// One vehicle of the skeleton scenario.
struct VehicleSpec
{
    std::string name; ///< roster name
    std::string cls;  ///< vehicle class
    Vector position;  ///< start position in m
    Vector velocity;  ///< constant velocity in m/s
};

/**
 * Round a coordinate to integer centimeters for the ground-truth log.
 * @param meters coordinate in m
 * @return coordinate in cm
 */
int64_t
ToCm(double meters)
{
    return static_cast<int64_t>(std::llround(meters * 100.0));
}

} // namespace

RunResult
RunScenario(const ScenarioConfig& config)
{
    NS_LOG_FUNCTION(config.seed << config.run << config.durationS << config.attacker);
    NS_ABORT_MSG_IF(config.durationS <= 0, "duration must be positive");

    // Every run starts from the same generator state (RSD-SIM-01c, RSD-SIM-01d).
    RngSeedManager::SetSeed(config.seed);
    RngSeedManager::SetRun(config.run);
    RngSeedManager::ResetNextStreamIndex();
    Names::Clear();

    const std::vector<VehicleSpec> fleet = {
        {"UAV-1", "UAV", Vector(0.0, 0.0, 30.0), Vector(5.0, 0.0, 0.0)},
        {"USV-1", "USV", Vector(0.0, 20.0, 0.0), Vector(0.0, 0.0, 0.0)},
    };

    GroundTruthStore truth;
    bool attackerFound = (config.attacker == "none");
    for (const auto& v : fleet)
    {
        bool isAttacker = (v.name == config.attacker);
        attackerFound = attackerFound || isAttacker;
        truth.AddVehicle(v.name, v.cls, isAttacker);
    }
    NS_ABORT_MSG_UNLESS(attackerFound, "unknown attacker label " << config.attacker);

    NodeContainer nodes;
    nodes.Create(fleet.size());
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes);
    for (uint32_t i = 0; i < fleet.size(); ++i)
    {
        Names::Add(fleet[i].name, nodes.Get(i));
        auto model = nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        model->SetPosition(fleet[i].position);
        model->SetVelocity(fleet[i].velocity);
    }

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",
                                 StringValue("HtMcs0"),
                                 "ControlMode",
                                 StringValue("HtMcs0"));
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // Routing between vehicles belongs to the TULPAR overlay only (RSD-NET-01a). The IPv4 stack
    // gets static routing with no route added, which delivers to the node's own UDP sockets and
    // one-hop broadcast and selects no path to another node; no ns-3 routing protocol is installed.
    InternetStackHelper internet;
    internet.SetRoutingHelper(Ipv4StaticRoutingHelper());
    internet.Install(nodes);
    Ipv4AddressHelper addresses;
    addresses.SetBase("10.1.0.0", "255.255.0.0");
    addresses.Assign(devices);
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        nodes.Get(i)->GetObject<Ipv4>()->SetAttribute("IpForward", BooleanValue(false));
    }

    // Fixed stream numbers for every random variable of the run (RSD-SIM-01c).
    int64_t stream = 0;
    stream += wifi.AssignStreams(devices, stream);
    stream += internet.AssignStreams(nodes, stream);

    EventLog log;
    const Time stop = Seconds(config.durationS);
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        auto agent = CreateObject<TulparAgent>();
        agent->Configure({static_cast<uint16_t>(i), fleet[i].name}, &log, stream + i);
        nodes.Get(i)->AddApplication(agent);
        agent->SetStartTime(Seconds(0));
        agent->SetStopTime(stop);
    }

    Simulator::Stop(stop + MilliSeconds(1));
    Simulator::Run();

    const int64_t endNs = Simulator::Now().GetNanoSeconds();
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Vector p = nodes.Get(i)->GetObject<MobilityModel>()->GetPosition();
        truth.LogPosition(endNs, fleet[i].name, ToCm(p.x), ToCm(p.y), ToCm(p.z));
    }
    Simulator::Destroy();
    Names::Clear();

    RunResult result;
    result.eventLog = log.Text();
    result.groundTruthLog = truth.Text();
    result.events = log.Count();
    result.tx = log.Count("TX");
    result.rx = log.Count("RX");
    result.traceHash = Hash64(result.eventLog);
    return result;
}

} // namespace tulpar
} // namespace ns3
