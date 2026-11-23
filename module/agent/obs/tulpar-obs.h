/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_OBS_H
#define TULPAR_OBS_H

#include "tulpar-agent-module.h"

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * M-OBS, observation fusion and statuses (RSD Section 4.2).
 *
 * Phase 0 skeleton: the module only reports that it started.
 */
class ObsModule : public AgentModule
{
  public:
    using AgentModule::AgentModule;

    std::string Code() const override;
    void Start() override;
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_OBS_H
