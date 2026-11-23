/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-obs.h"

namespace ns3
{
namespace tulpar
{

std::string
ObsModule::Code() const
{
    return "OBS";
}

void
ObsModule::Start()
{
    Emit(MakeEvent("START"));
}

} // namespace tulpar
} // namespace ns3
