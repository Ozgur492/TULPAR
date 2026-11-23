/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-lsc.h"

namespace ns3
{
namespace tulpar
{

std::string
LscModule::Code() const
{
    return "LSC";
}

void
LscModule::Start()
{
    Emit(MakeEvent("START"));
}

} // namespace tulpar
} // namespace ns3
