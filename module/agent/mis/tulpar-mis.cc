/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-mis.h"

namespace ns3
{
namespace tulpar
{

std::string
MisModule::Code() const
{
    return "MIS";
}

void
MisModule::Start()
{
    Emit(MakeEvent("START"));
}

} // namespace tulpar
} // namespace ns3
