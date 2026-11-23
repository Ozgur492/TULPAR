/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_SEC_H
#define TULPAR_SEC_H

#include "tulpar-agent-module.h"

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * M-SEC, distributed security evaluation and responses (RSD Section 4.7). Never commands motion
 * (MD5).
 *
 * Phase 0 skeleton: the module only reports that it started.
 */
class SecModule : public AgentModule
{
  public:
    using AgentModule::AgentModule;

    std::string Code() const override;
    void Start() override;
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_SEC_H
