/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-del.h"

namespace ns3
{
namespace tulpar
{

std::string
DelModule::Code() const
{
    return "DEL";
}

void
DelModule::Start()
{
    Emit(MakeEvent("START"));
}

} // namespace tulpar
} // namespace ns3
