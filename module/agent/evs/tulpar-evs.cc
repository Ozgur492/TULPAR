/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-evs.h"

namespace ns3
{
namespace tulpar
{

std::string
EvsModule::Code() const
{
    return "EVS";
}

void
EvsModule::Start()
{
    Emit(MakeEvent("START"));
}

} // namespace tulpar
} // namespace ns3
