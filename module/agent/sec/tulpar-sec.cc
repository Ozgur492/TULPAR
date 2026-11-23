/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-sec.h"

namespace ns3
{
namespace tulpar
{

std::string
SecModule::Code() const
{
    return "SEC";
}

void
SecModule::Start()
{
    Emit(MakeEvent("START"));
}

} // namespace tulpar
} // namespace ns3
