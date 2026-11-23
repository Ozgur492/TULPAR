/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TULPAR_CRY_H
#define TULPAR_CRY_H

#include "tulpar-agent-module.h"

namespace ns3
{
namespace tulpar
{

/**
 * @ingroup tulpar
 * M-CRY, cryptographic identity, sessions and signatures (RSD Section 4.5). Botan 3.13.0 is linked
 * from Phase 1 (RSD-CRY-01e).
 *
 * Phase 0 skeleton: the module only reports that it started.
 */
class CryModule : public AgentModule
{
  public:
    using AgentModule::AgentModule;

    std::string Code() const override;
    void Start() override;
};

} // namespace tulpar
} // namespace ns3

#endif // TULPAR_CRY_H
