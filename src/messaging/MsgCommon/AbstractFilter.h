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

#ifndef __TIZEN_TIZEN_ABSTRACT_FILTER_H__
#define __TIZEN_TIZEN_ABSTRACT_FILTER_H__

#include <memory>
#include <vector>
#include <sstream>
//#include <JSArray.h>

#include "Any.h"

namespace extension {
namespace tizen {

class AttributeFilter;
typedef std::shared_ptr<AttributeFilter> AttributeFilterPtr;

class AttributeRangeFilter;
typedef std::shared_ptr<AttributeRangeFilter> AttributeRangeFilterPtr;

class CompositeFilter;
typedef std::shared_ptr<CompositeFilter> CompositeFilterPtr;

class AbstractFilter;
typedef std::shared_ptr<AbstractFilter> AbstractFilterPtr;
typedef std::vector<AbstractFilterPtr> AbstractFilterPtrVector;

struct AbstractFilterHolder {
    AbstractFilterPtr ptr;
};

enum FilterType {
    ABSTRACT_FILTER = 0,
    ATTRIBUTE_FILTER,
    ATTRIBUTE_RANGE_FILTER,
    COMPOSITE_FILTER
};

enum FilterMatchFlag {
    EXACTLY = 0,
    FULLSTRING,
    CONTAINS,
    STARTSWITH,
    ENDSWITH,
    EXISTS
};

class FilterableObject {
public:
    virtual ~FilterableObject(){}

    virtual bool isMatchingAttribute(const std::string& attribute_name,
            const FilterMatchFlag match_flag,
            AnyPtr match_value) const = 0;

    virtual bool isMatchingAttributeRange(const std::string& attribute_name,
            AnyPtr initial_value,
            AnyPtr end_value) const = 0;
};

class AbstractFilter {
public:
    AbstractFilter(FilterType type = ABSTRACT_FILTER);
    virtual ~AbstractFilter();

    FilterType getFilterType() const;

//    static JSValueRef makeJSValue(JSContextRef context, AbstractFilterPtr priv);
//    static AbstractFilterPtr getPrivateObject(JSContextRef context,
//            JSValueRef value);

    virtual bool isMatching(const FilterableObject* const filtered_object) const;

protected:
    FilterType m_filter_type;
};

/**
 * Returns NULL shared_ptr if cast is not possible (incorrect type)
 */
AttributeFilterPtr castToAttributeFilter(AbstractFilterPtr from);
AttributeRangeFilterPtr castToAttributeRangeFilter(AbstractFilterPtr from);
CompositeFilterPtr castToCompositeFilter(AbstractFilterPtr from);


//class JSFilterArray : public Common::JSArray<AbstractFilterPtr> {
//public:
//    JSFilterArray(JSContextRef ctx, JSObjectRef array):
//            JSArray<AbstractFilterPtr>(ctx, array, AbstractFilter::getPrivateObject,
//                    AbstractFilter::makeJSValue)
//    {
//    }
//    JSFilterArray(JSContextRef ctx):
//            JSArray<AbstractFilterPtr>(ctx, AbstractFilter::getPrivateObject,
//                    AbstractFilter::makeJSValue)
//    {
//    }
//    void operator=(const std::vector<AbstractFilterPtr>& list)
//    {
//        overwrite(list);
//    }
//};



class FilterUtils {
public:
    static bool isStringMatching(const std::string& key,
            const std::string& value,
            tizen::FilterMatchFlag flag);
    static bool isAnyStringMatching(const std::string& key,
            const std::vector<std::string>& values,
            tizen::FilterMatchFlag flag);
    static bool isTimeStampInRange(const time_t& time_stamp,
            tizen::AnyPtr& initial_value,
            tizen::AnyPtr& end_value);

    static inline bool isBetweenTimeRange(const time_t current,
            const time_t from,
            const time_t to)
    {
        return ((current - from) >= 0 ) && ((to - current) >= 0);
    }

    static inline std::string boolToString(const bool src)
    {
        if(src) {
            return "true";
        }
        else {
            return "false";
        }
    }
};

} // Tizen
} // DeviceAPI


#include "AttributeFilter.h"
#include "AttributeRangeFilter.h"
#include "CompositeFilter.h"

#endif // __TIZEN_TIZEN_ABSTRACT_FILTER_H__
