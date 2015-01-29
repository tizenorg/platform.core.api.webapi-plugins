// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
        LOGD("ORIGINAL_NETWORK_ID not supported");
        return;
    }
    if ((m_mode == SERVICE_MODE_DVB || m_mode == SERVICE_MODE_DVBNT)
        && (columnName == MINOR || columnName == SOURCE_ID)) {
        LOGD("%d not supported", columnName);
        return;
    }
    if (m_mode == SERVICE_MODE_ISDB && columnName == SOURCE_ID) {
        LOGD("SOURCE_ID not supported");
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
