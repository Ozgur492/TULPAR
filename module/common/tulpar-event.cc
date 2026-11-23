/*
 * Copyright (c) 2026 TULPAR project team
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "tulpar-event.h"

#include <sstream>

namespace ns3
{
namespace tulpar
{

Event&
Event::With(const std::string& key, const std::string& value)
{
    fields.emplace_back(key, value);
    return *this;
}

Event&
Event::With(const std::string& key, int64_t value)
{
    fields.emplace_back(key, std::to_string(value));
    return *this;
}

std::string
Event::ToLine() const
{
    std::ostringstream os;
    os << "t=" << timeNs << " veh=" << vehicle << " mod=" << module << " ev=" << type;
    for (const auto& [key, value] : fields)
    {
        os << ' ' << key << '=' << value;
    }
    return os.str();
}

} // namespace tulpar
} // namespace ns3
