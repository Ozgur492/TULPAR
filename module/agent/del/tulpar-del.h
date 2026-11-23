/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_DEL_H
#define TULPAR_DEL_H

#include "tulpar-agent-module.h"

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * M-DEL, reliable and delayed delivery of bundles (RSD Section 4.3).
 *
 * Phase 0 skeleton: the module only reports that it started.
 */
class DelModule : public AgentModule
{
  public:
    using AgentModule::AgentModule;

    std::string Code() const override;
    void Start() override;
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_DEL_H
