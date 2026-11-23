/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-agent.h"

#include "tulpar-com.h"
#include "tulpar-cry.h"
#include "tulpar-del.h"
#include "tulpar-evs.h"
#include "tulpar-lsc.h"
#include "tulpar-mis.h"
#include "tulpar-obs.h"
#include "tulpar-sec.h"

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TulparAgent");

namespace tulpar
{

NS_OBJECT_ENSURE_REGISTERED(TulparAgent);

TypeId
TulparAgent::GetTypeId()
{
    static TypeId tid = TypeId("ns3::tulpar::TulparAgent")
                            .SetParent<Application>()
                            .SetGroupName("Tulpar")
                            .AddConstructor<TulparAgent>()
                            .AddAttribute("BeaconPeriod",
                                          "Period of the skeleton neighbor beacon.",
                                          TimeValue(Seconds(1)),
                                          MakeTimeAccessor(&TulparAgent::m_beaconPeriod),
                                          MakeTimeChecker(MilliSeconds(10)));
    return tid;
}

TulparAgent::TulparAgent()
{
    NS_LOG_FUNCTION(this);
}

TulparAgent::~TulparAgent()
{
    NS_LOG_FUNCTION(this);
}

void
TulparAgent::Configure(VehicleIdentity self, EventSink* sink, int64_t stream)
{
    NS_LOG_FUNCTION(this << self.name << stream);
    NS_ABORT_MSG_IF(sink == nullptr, "TulparAgent needs an event sink");
    NS_ABORT_MSG_IF(stream < 0, "TulparAgent needs an explicit stream number");
    m_self = std::move(self);
    m_sink = sink;
    m_stream = stream;
}

void
TulparAgent::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_modules.clear();
    Application::DoDispose();
}

void
TulparAgent::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_MSG_IF(m_sink == nullptr, "TulparAgent started without Configure()");

    auto jitter = CreateObject<UniformRandomVariable>();
    jitter->SetStream(m_stream);

    // M-CRY first: every other module uses its services (RSD Table 4.25, I6).
    m_modules.push_back(std::make_unique<CryModule>(m_self, *m_sink));
    m_modules.push_back(
        std::make_unique<ComModule>(m_self, *m_sink, GetNode(), jitter, m_beaconPeriod));
    m_modules.push_back(std::make_unique<DelModule>(m_self, *m_sink));
    m_modules.push_back(std::make_unique<EvsModule>(m_self, *m_sink));
    m_modules.push_back(std::make_unique<ObsModule>(m_self, *m_sink));
    m_modules.push_back(std::make_unique<SecModule>(m_self, *m_sink));
    m_modules.push_back(std::make_unique<MisModule>(m_self, *m_sink));
    m_modules.push_back(std::make_unique<LscModule>(m_self, *m_sink));
    for (auto& module : m_modules)
    {
        module->Start();
    }
}

void
TulparAgent::StopApplication()
{
    NS_LOG_FUNCTION(this);
    for (auto it = m_modules.rbegin(); it != m_modules.rend(); ++it)
    {
        (*it)->Stop();
    }
}

} // namespace tulpar
} // namespace ns3
