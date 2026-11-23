/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-cry.h"

namespace ns3
{
namespace tulpar
{

std::string
CryModule::Code() const
{
    return "CRY";
}

void
CryModule::Start()
{
    Emit(MakeEvent("START"));
}

} // namespace tulpar
} // namespace ns3
