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
 
#ifndef __TIZEN_TIZEN_FILTER_ITERATOR_H__
#define __TIZEN_TIZEN_FILTER_ITERATOR_H__

#include "AbstractFilter.h"
#include "AttributeFilter.h"
#include "AttributeRangeFilter.h"
#include "CompositeFilter.h"

#include <stack>

namespace extension {
namespace tizen {

enum FilterIteratorState {
    FIS_NOT_VALID = 0,
    FIS_ATTRIBUTE_FILTER,
    FIS_ATTRIBUTE_RANGE_FILTER,
    FIS_COMPOSITE_START,
    FIS_COMPOSITE_END,
    FIS_END
};

class FilterIterator
{
public:
    FilterIterator(AbstractFilterPtr filter);

    FilterIteratorState getState() const;
    AbstractFilterPtr operator*() const;
    AbstractFilterPtr getCurrentFilter() const;
    bool isEnd() const;

    bool isInsideCompositeFilter() const;

    /**
     * Returns null shared pointer if we are not inside composite filter
     */
    CompositeFilterPtr getCurrentCompositeFilter() const;

    /**
     * Get index of current sub filter (inside composite filter)
     * Returns 0 if we are not inside composite filter.
     */
    int getCurrentCompositeSubFilterIndex() const;

    /**
     * Return true if current sub filter is the last one in current composite filter
     * Returns false if we are not inside composite filter.
     */
    bool isLastCompositeSubFilter() const;

    void operator++();
    void operator++(int);

private:
    void setReachedEnd();
    void goToNext(AbstractFilterPtr next);
    void goToNextInCurCompositeFilter();

    AbstractFilterPtr m_root_filter;
    FilterIteratorState m_current_state;
    AbstractFilterPtr m_current_filter;

    struct CompositeIterState {
        CompositeIterState() :
                cur_sub_filter_index(0)
        {
        }

        CompositeFilterPtr filter;
        int cur_sub_filter_index;
    };

    std::stack<CompositeIterState> m_composite_stack;
};

} // Tizen
} // DeviceAPI

#endif // __TIZEN_TIZEN_FILTER_ITERATOR_H__
