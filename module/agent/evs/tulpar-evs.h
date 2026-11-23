/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_EVS_H
#define TULPAR_EVS_H

#include "tulpar-agent-module.h"

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * M-EVS, evidence store and Evidence Graph (RSD Section 4.4).
 *
 * Phase 0 skeleton: the module only reports that it started.
 */
class EvsModule : public AgentModule
{
  public:
    using AgentModule::AgentModule;

    std::string Code() const override;
    void Start() override;
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_EVS_H
