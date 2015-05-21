// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_TVCHANNEL_CRITERIA_FILTER_H_
#define SRC_TVCHANNEL_CRITERIA_FILTER_H_

#include <ServiceNavigationDataType.h>
#include <CriteriaHelper.h>
#include <stdint.h>
#include <map>
#include <memory>

namespace extension {
namespace tvchannel {

class CriteriaFilter {
 public:
    explicit CriteriaFilter(EServiceMode mode);
    void filterWhere(int32_t columnName, const int32_t columnValue);
    void getFilteredCriteria(std::unique_ptr < TCCriteriaHelper > & _pCriteria);
    void resetFilter();
 private:
    EServiceMode m_mode;
    std::map<int32_t, int32_t> columnMap;
};

}  //  namespace tvchannel
}  //  namespace extension

#endif  //  SRC_TVCHANNEL_CRITERIA_FILTER_H_
