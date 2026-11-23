/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_LSC_H
#define TULPAR_LSC_H

#include "tulpar-agent-module.h"

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * M-LSC, local safety controller (RSD Section 4.8). Links no agent module and no record decoder
 * (MD19).
 *
 * Phase 0 skeleton: the module only reports that it started.
 */
class LscModule : public AgentModule
{
  public:
    using AgentModule::AgentModule;

    std::string Code() const override;
    void Start() override;
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_LSC_H
