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

#ifndef __TIZEN_TIZEN_ATTRIBUTE_FILTER_H__
#define __TIZEN_TIZEN_ATTRIBUTE_FILTER_H__

#include "AbstractFilter.h"
#include "Any.h"
#include <string>

namespace extension {
namespace tizen {

class AttributeFilter;
typedef std::shared_ptr<AttributeFilter> AttributeFilterPtr;

class AttributeFilter: public AbstractFilter {
public:
    AttributeFilter(const std::string &attribute_name);
    virtual ~AttributeFilter();

    std::string getAttributeName() const;
    void setAttributeName(const std::string &attribute_name);
    FilterMatchFlag getMatchFlag() const;
    void setMatchFlag(FilterMatchFlag match_flag);
    AnyPtr getMatchValue() const;
    void setMatchValue(AnyPtr match_value);

    virtual bool isMatching(const FilterableObject* const filtered_object) const;
private:
    std::string m_attribute_name;
    FilterMatchFlag m_match_flag;
    AnyPtr m_match_value;
};

} // Tizen
} // DeviceAPI

#endif // __TIZEN_TIZEN_ABSTRACT_FILTER_H__
