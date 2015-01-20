// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATACONTROL_DATACONTROL_INSTANCE_H_
#define DATACONTROL_DATACONTROL_INSTANCE_H_

#include <data_control.h>
#include <string>

#include "common/extension.h"

namespace extension {
namespace datacontrol {

class DatacontrolInstance : public common::ParsedInstance {
 public:
  DatacontrolInstance();
  virtual ~DatacontrolInstance();

  typedef std::function< int (data_control_h& handle, int *requestId) >
      DataControlJob;
  int RunMAPDataControlJob(const std::string& providerId,
                           const std::string& dataId,
                           int callbackId, int userRequestId,
                           DataControlJob job);
  int RunSQLDataControlJob(const std::string& providerId,
                           const std::string& dataId,
                           int callbackId, int userRequestId,
                           DataControlJob job);

 private:
  void DataControlManagerGetdatacontrolconsumer(const picojson::value& args,
                                                picojson::object& out);
  void SQLDataControlConsumerInsert(const picojson::value& args,
                                    picojson::object& out);
  void SQLDataControlConsumerUpdate(const picojson::value& args,
                                    picojson::object& out);
  void SQLDataControlConsumerRemove(const picojson::value& args,
                                    picojson::object& out);
  void SQLDataControlConsumerSelect(const picojson::value& args,
                                    picojson::object& out);
  void MappedDataControlConsumerAddvalue(const picojson::value& args,
                                         picojson::object& out);
  void MappedDataControlConsumerRemovevalue(const picojson::value& args,
                                            picojson::object& out);
  void MappedDataControlConsumerGetvalue(const picojson::value& args,
                                         picojson::object& out);
  void MappedDataControlConsumerUpdatevalue(const picojson::value& args,
                                            picojson::object& out);
};

}  // namespace datacontrol
}  // namespace extension

#endif  // DATACONTROL_DATACONTROL_INSTANCE_H_
