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
 
#ifndef __TIZEN_TIZEN_SORT_MODE_H__
#define __TIZEN_TIZEN_SORT_MODE_H__

#include <memory>
#include <string>

namespace extension {
namespace tizen {

class SortMode;
typedef std::shared_ptr<SortMode> SortModePtr;

enum SortModeOrder {
    ASC,
    DESC
};

class SortMode {
public:
    SortMode(const std::string &attribute_name, SortModeOrder order);
    virtual ~SortMode();

    std::string getAttributeName() const;
    void setAttributeName(const std::string &attribute_name);
    SortModeOrder getOrder() const;
    void setOrder(SortModeOrder order);

private:
    std::string m_attribute_name;
    SortModeOrder m_order;
};

} // Tizen
} // DeviceAPI

#endif // __TIZEN_TIZEN_SORT_MODE_H__
