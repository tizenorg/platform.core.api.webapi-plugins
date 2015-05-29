/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "SortMode.h"

#include "common/logger.h"

namespace extension {
namespace tizen {

SortMode::SortMode(const std::string &attribute_name, SortModeOrder order) :
        m_attribute_name(attribute_name),
        m_order(order)
{
    LoggerD("SortMode attributeName: %s, SortMode order: %s",
            attribute_name.c_str(), (order == SortModeOrder::DESC) ? "DESC" : "ASC");
}

SortMode::~SortMode()
{

}

std::string SortMode::getAttributeName() const
{
    return m_attribute_name;
}

void SortMode::setAttributeName(const std::string &attribute_name)
{
    m_attribute_name = attribute_name;
}

SortModeOrder SortMode::getOrder() const
{
    return m_order;
}

void SortMode::setOrder(SortModeOrder order)
{
    m_order = order;
}

} // Tizen
} // DeviceAPI
