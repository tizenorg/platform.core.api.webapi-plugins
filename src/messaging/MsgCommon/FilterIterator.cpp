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
 
#include "FilterIterator.h"
#include "common/platform_exception.h"
#include "common/logger.h"

using namespace common;

namespace extension {
namespace tizen {

FilterIterator::FilterIterator(AbstractFilterPtr filter) :
        m_root_filter(filter),
        m_current_state(FIS_NOT_VALID)
{
    if (!m_root_filter) {
        LoggerE("Trying to create FilterIterator with NULL filter");
        m_root_filter = AbstractFilterPtr(new AbstractFilter());
    }

    goToNext(m_root_filter);
}

FilterIteratorState FilterIterator::getState() const
{
    return m_current_state;
}

AbstractFilterPtr FilterIterator::operator*() const
{
    return m_current_filter;
}

AbstractFilterPtr FilterIterator::getCurrentFilter() const
{
    return m_current_filter;
}

bool FilterIterator::isEnd() const
{
    return FIS_END == m_current_state;
}

bool FilterIterator::isInsideCompositeFilter() const
{
    return !m_composite_stack.empty();
}

CompositeFilterPtr FilterIterator::getCurrentCompositeFilter() const
{
    if(m_composite_stack.empty()) {
        return CompositeFilterPtr();
    }

    return m_composite_stack.top().filter;
}

int FilterIterator::getCurrentCompositeSubFilterIndex() const
{
    if(!isInsideCompositeFilter()) {
        return 0;
    }

    return m_composite_stack.top().cur_sub_filter_index;
}

bool FilterIterator::isLastCompositeSubFilter() const
{
    if(!isInsideCompositeFilter()) {
        return false;
    }

    CompositeIterState cfilter = m_composite_stack.top();
    return (int)(cfilter.filter->getFilters().size() - 1) == cfilter.cur_sub_filter_index;
}

void FilterIterator::operator++(int)
{
    this->operator++();
}

void FilterIterator::operator++()
{
    if(FIS_ATTRIBUTE_FILTER == m_current_state ||
            FIS_ATTRIBUTE_RANGE_FILTER == m_current_state) {

        if(m_composite_stack.empty()) {
            //We are not inside composite filter iteration -> reached THE END
            setReachedEnd();
        } else {
            //We are inside a composite filter -> try move to next sub filter
            goToNextInCurCompositeFilter();
        }
    }
    else if(FIS_COMPOSITE_START == m_current_state) {
        goToNextInCurCompositeFilter();
    }
    else if(FIS_COMPOSITE_END == m_current_state) {
        m_composite_stack.pop();
        if(m_composite_stack.empty()) {
            //There is no parent composite filter -> reached THE END
            setReachedEnd();
        }
        else {
            //There is parent composite filter -> go back and try move to next sub filter
            goToNextInCurCompositeFilter();
        }
    }
    else if(FIS_NOT_VALID == m_current_state) {
        //There is nothing to do -> reached THE END
        setReachedEnd();
    }
}

void FilterIterator::goToNextInCurCompositeFilter()
{
    CompositeIterState& cur_cs = m_composite_stack.top();
    AbstractFilterPtrVector sub_filters = cur_cs.filter->getFilters();
    const size_t next_filter_index = cur_cs.cur_sub_filter_index + 1;
    if(next_filter_index >= sub_filters.size()) {
        //Reached last item of composite filter
        m_current_filter = cur_cs.filter;
        m_current_state = FIS_COMPOSITE_END;
    }
    else {
        cur_cs.cur_sub_filter_index = next_filter_index;
        //There is next item inside this composite filter
        goToNext(sub_filters[next_filter_index]);
    }
}

void FilterIterator::setReachedEnd()
{
    m_current_state = FIS_END;
    m_current_filter = AbstractFilterPtr();
}

void FilterIterator::goToNext(AbstractFilterPtr next)
{
    switch(next->getFilterType()) {
        case ATTRIBUTE_FILTER: {
            m_current_state = FIS_ATTRIBUTE_FILTER;
        } break;
        case ATTRIBUTE_RANGE_FILTER: {
            m_current_state = FIS_ATTRIBUTE_RANGE_FILTER;
        } break;
        case COMPOSITE_FILTER: {
            m_current_state = FIS_COMPOSITE_START;
            CompositeIterState cf_state;
            cf_state.filter = castToCompositeFilter(next);
            cf_state.cur_sub_filter_index = -1;
            m_composite_stack.push(cf_state);
        } break;
        case ABSTRACT_FILTER: {
            LoggerE("Reached AbstractFilter!!");
            m_current_state = FIS_NOT_VALID;
        } break;
    }

    m_current_filter = next;
}

} // Tizen
} // DeviceAPI
