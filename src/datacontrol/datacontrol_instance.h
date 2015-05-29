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
