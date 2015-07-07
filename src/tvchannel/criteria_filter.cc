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

#include "tvchannel/criteria_filter.h"
#include <common/logger.h>

namespace extension {
namespace tvchannel {

CriteriaFilter::CriteriaFilter(EServiceMode _mode) :
    m_mode(_mode) {
}

void CriteriaFilter::filterWhere(int32_t columnName,
    const int32_t columnValue) {
    if (m_mode == SERVICE_MODE_ATSC && columnName == ORIGINAL_NETWORK_ID) {
        LoggerD("ORIGINAL_NETWORK_ID not supported");
        return;
    }
    if ((m_mode == SERVICE_MODE_DVB || m_mode == SERVICE_MODE_DVBNT)
        && (columnName == MINOR || columnName == SOURCE_ID)) {
        LoggerD("%d not supported", columnName);
        return;
    }
    if (m_mode == SERVICE_MODE_ISDB && columnName == SOURCE_ID) {
        LoggerD("SOURCE_ID not supported");
        return;
    }

    columnMap[columnName] = columnValue;
}

void CriteriaFilter::getFilteredCriteria(std::unique_ptr < TCCriteriaHelper > & _pCriteria) {
    for (auto it = columnMap.begin(); it != columnMap.end(); ++it) {
        _pCriteria->Where(it->first, it->second);
    }
}

void CriteriaFilter::resetFilter() {
    columnMap.clear();
}

}  //  namespace tvchannel
}  //  namespace extension
