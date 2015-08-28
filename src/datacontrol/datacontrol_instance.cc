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

#include "datacontrol/datacontrol_instance.h"

#include <glib.h>

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

#include "common/scope_exit.h"

namespace extension {
namespace datacontrol {

using common::InvalidValuesException;
using common::TypeMismatchException;
using common::IOException;
using common::SecurityException;
using common::UnknownException;
using common::NotFoundException;

using common::ScopeExit;
using common::operator+;

struct DatacontrolInformation {
  int callbackId;
  int requestId;
  int userDefinedRequestId;
};

static std::map<int, DatacontrolInformation*> IdMap;

DatacontrolInstance::DatacontrolInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;
  #define REGISTER_SYNC(c, x) \
    RegisterSyncHandler(c, std::bind(&DatacontrolInstance::x, this, _1, _2));
  REGISTER_SYNC("SQLDataControlConsumer_update", SQLDataControlConsumerUpdate);
  REGISTER_SYNC("MappedDataControlConsumer_addValue",
                MappedDataControlConsumerAddvalue);
  REGISTER_SYNC("SQLDataControlConsumer_select",
                SQLDataControlConsumerSelect);
  REGISTER_SYNC("SQLDataControlConsumer_remove", SQLDataControlConsumerRemove);
  REGISTER_SYNC("MappedDataControlConsumer_removeValue",
                MappedDataControlConsumerRemovevalue);
  REGISTER_SYNC("MappedDataControlConsumer_updateValue",
                MappedDataControlConsumerUpdatevalue);
  REGISTER_SYNC("DataControlManager_getDataControlConsumer",
                DataControlManagerGetdatacontrolconsumer);
  REGISTER_SYNC("SQLDataControlConsumer_insert", SQLDataControlConsumerInsert);
  REGISTER_SYNC("MappedDataControlConsumer_getValue",
                MappedDataControlConsumerGetvalue);
  #undef REGISTER_SYNC
}

DatacontrolInstance::~DatacontrolInstance() {
}

static void ReplyAsync(DatacontrolInstance* instance, int callbackId,
                       bool isSuccess, picojson::object* param) {
  LoggerD("Enter");
  (*param)["callbackId"] = picojson::value(static_cast<double>(callbackId));
  (*param)["status"] = picojson::value(isSuccess ? "success" : "error");

  picojson::value result = picojson::value(*param);

  common::Instance::PostMessage(instance, result.serialize().c_str());
}

static bool SQLColumnName(result_set_cursor cursor, int columnIndex,
                          picojson::value& name) {
  LoggerD("Enter");
  char buffer[4096];
  int result = data_control_sql_get_column_name(cursor, columnIndex, buffer);
  if (result != DATA_CONTROL_ERROR_NONE) {
    LoggerE("Getting column item type is failed with error : %d", result);
    return false;
  }
  name = picojson::value(buffer);
  return true;
}

static bool SQLColumnValue(result_set_cursor cursor, int columnIndex,
                           picojson::value& val) {
  LoggerD("Enter");
  data_control_sql_column_type_e type = DATA_CONTROL_SQL_COLUMN_TYPE_UNDEFINED;
  int result =
      data_control_sql_get_column_item_type(cursor, columnIndex, &type);
  if (result != DATA_CONTROL_ERROR_NONE) {
    LoggerE("Getting column item type is failed with error : %d", result);
    return false;
  }
  switch (type) {
    case DATA_CONTROL_SQL_COLUMN_TYPE_INT64: {
      int64_t data = 0;
      result = data_control_sql_get_int64_data(cursor, columnIndex, &data);
      if (result != DATA_CONTROL_ERROR_NONE) break;
      val = picojson::value(static_cast<double>(data));
      break;
    }
    case DATA_CONTROL_SQL_COLUMN_TYPE_DOUBLE: {
      double data = 0;
      result = data_control_sql_get_double_data(cursor, columnIndex, &data);
      if (result != DATA_CONTROL_ERROR_NONE) break;
      val = picojson::value(data);
      break;
    }
    case DATA_CONTROL_SQL_COLUMN_TYPE_TEXT: {
      int size = data_control_sql_get_column_item_size(cursor, columnIndex);
      char *buffer = new char[size + 1];
      result = data_control_sql_get_text_data(cursor, columnIndex, buffer);
      if (result != DATA_CONTROL_ERROR_NONE) {
        LoggerE("Getting Text value failed : %s", get_error_message(result));
        delete[] buffer;
        break;
      }
      val = picojson::value(buffer);
      delete[] buffer;
      break;
    }
    case DATA_CONTROL_SQL_COLUMN_TYPE_BLOB: {
      int size = data_control_sql_get_column_item_size(cursor, columnIndex);
      char *buffer = new char[size + 1];
      result =
          data_control_sql_get_blob_data(cursor, columnIndex, buffer, size);
      if (result != DATA_CONTROL_ERROR_NONE) {
        delete[] buffer;
        break;
      }
      val = picojson::value(buffer);
      delete[] buffer;
      break;
    }
    case DATA_CONTROL_SQL_COLUMN_TYPE_NULL: {
      val = picojson::value();
      break;
    }
    default: {
      LoggerE("%th column is undefined column type", columnIndex);
      return false;
    }
  }
  if (result != DATA_CONTROL_ERROR_NONE) {
    LoggerE("Getting column item value is failed with error : %s",
            ::get_error_message(result));
    return false;
  } else {
    return true;
  }
}

static void MAPAddResponseCallback(int requestId, data_control_h handle,
                                   bool providerResult,
                                   const char *error, void *user_data) {
  LoggerD("Enter");
  DatacontrolInformation *info = IdMap[requestId];
  if (info == NULL) {
    LoggerE("Invalid context");
    return;
  }

  picojson::object obj;
  obj["requestId"] =
      picojson::value(static_cast<double>(info->userDefinedRequestId));
  if (!providerResult) {
    obj["result"] = InvalidValuesException(error).ToJSON();
  }

  ReplyAsync(static_cast<DatacontrolInstance*>(user_data), info->callbackId,
             providerResult, &obj);
  delete info;
  IdMap.erase(requestId);
}

static void MAPSetResponseCallback(int requestId, data_control_h handle,
                                   bool providerResult,
                                   const char *error, void *user_data) {
  LoggerD("Enter");
  DatacontrolInformation *info = IdMap[requestId];
  if (info == NULL) {
    LoggerE("Invalid context");
    return;
  }

  picojson::object obj;
  obj["requestId"] =
      picojson::value(static_cast<double>(info->userDefinedRequestId));
  if (!providerResult) {
    obj["result"] = NotFoundException(error).ToJSON();
  }

  ReplyAsync(static_cast<DatacontrolInstance*>(user_data), info->callbackId, providerResult, &obj);
  delete info;
  IdMap.erase(requestId);
}

static void MAPGetResponseCallback(int requestId, data_control_h handle,
                                   char **result_value_list,
                                   int result_value_count,
                                   bool providerResult,
                                   const char *error, void *user_data) {
  LoggerD("Enter");
  DatacontrolInformation *info = IdMap[requestId];
  if (info == NULL) {
    LoggerE("Invalid context");
    return;
  }

  picojson::object obj;
  obj["requestId"] =
      picojson::value(static_cast<double>(info->userDefinedRequestId));
  if (!providerResult) {
    obj["result"] = NotFoundException(error).ToJSON();
  } else {
    picojson::array result;
    for (int i=0; i < result_value_count; i++) {
      result.push_back(picojson::value(result_value_list[i]));
    }
    obj["result"] = picojson::value(result);
  }

  ReplyAsync(static_cast<DatacontrolInstance*>(user_data), info->callbackId,
             providerResult, &obj);
  delete info;
  IdMap.erase(requestId);
}

static void MAPRemoveReponseCallback(int requestId, data_control_h handle,
                                     bool providerResult,
                                     const char *error, void *user_data) {
  LoggerD("Enter");
  DatacontrolInformation *info = IdMap[requestId];
  if (info == NULL) {
    LoggerE("Invalid context");
    return;
  }

  picojson::object obj;
  obj["requestId"] =
      picojson::value(static_cast<double>(info->userDefinedRequestId));
  if (!providerResult) {
    obj["result"] = NotFoundException(error).ToJSON();
  }

  ReplyAsync(static_cast<DatacontrolInstance*>(user_data), info->callbackId,
             providerResult, &obj);
  delete info;
  IdMap.erase(requestId);
}


static void SQLSelectResponseCallback(int requestId, data_control_h handle,
                                      result_set_cursor cursor,
                                      bool providerResult,
                                      const char *error, void *user_data) {
  LoggerD("Enter");
  DatacontrolInformation *info = IdMap[requestId];
  if (info == NULL) {
    LoggerE("Invalid context");
    return;
  }

  picojson::object obj;
  obj["requestId"] =
      picojson::value(static_cast<double>(info->userDefinedRequestId));
  if (!providerResult) {
    obj["result"] = InvalidValuesException(error).ToJSON();
  } else {
    picojson::array result;

    while (data_control_sql_step_next(cursor) == DATA_CONTROL_ERROR_NONE) {
      picojson::object rowData;
      picojson::array columns;
      picojson::array values;
      int columnCount = data_control_sql_get_column_count(cursor);
      for (int i=0; i < columnCount; i++) {
        picojson::value column;
        picojson::value value;
        if (SQLColumnName(cursor, i, column) &&
            SQLColumnValue(cursor, i, value)) {
          columns.push_back(column);
          values.push_back(value);
        }
      }
      rowData["columns"] = picojson::value(columns);
      rowData["values"] = picojson::value(values);
      result.push_back(picojson::value(rowData));
    }
    obj["result"] = picojson::value(result);
  }
  ReplyAsync(static_cast<DatacontrolInstance*>(user_data), info->callbackId,
             providerResult, &obj);
  delete info;
  IdMap.erase(requestId);
}

static void SQLInsertResponseCallback(int requestId, data_control_h handle,
                                      int64_t inserted_row_id,
                                      bool providerResult,
                                      const char *error, void *user_data) {
  LoggerD("Enter");
  DatacontrolInformation *info = IdMap[requestId];
  if (info == NULL) {
    LoggerE("Invalid context");
    return;
  }

  picojson::object obj;
  obj["requestId"] =
      picojson::value(static_cast<double>(info->userDefinedRequestId));
  if (!providerResult) {
    obj["result"] = InvalidValuesException(error).ToJSON();
  } else {
    obj["result"] = picojson::value(static_cast<double>(inserted_row_id));
  }

  ReplyAsync(static_cast<DatacontrolInstance*>(user_data), info->callbackId,
             providerResult, &obj);
  delete info;
  IdMap.erase(requestId);
}

static void SQLUpdateResponseCallback(int requestId, data_control_h handle,
                                      bool providerResult,
                                      const char *error, void *user_data) {
  LoggerD("Enter");
  DatacontrolInformation *info = IdMap[requestId];
  if (info == NULL) {
    LoggerE("Invalid context");
    return;
  }

  picojson::object obj;
  obj["requestId"] =
      picojson::value(static_cast<double>(info->userDefinedRequestId));
  if (!providerResult) {
    obj["result"] = InvalidValuesException(error).ToJSON();
  }

  ReplyAsync(static_cast<DatacontrolInstance*>(user_data), info->callbackId,
             providerResult, &obj);
  delete info;
  IdMap.erase(requestId);
}

static void SQLDeleteResponseCallback(int requestId, data_control_h handle,
                                      bool providerResult,
                                      const char *error, void *user_data) {
  LoggerD("Enter");
  DatacontrolInformation *info = IdMap[requestId];
  if (info == NULL) {
    LoggerE("Invalid context");
    return;
  }

  picojson::object obj;
  obj["requestId"] =
      picojson::value(static_cast<double>(info->userDefinedRequestId));
  if (!providerResult) {
    obj["result"] = InvalidValuesException(error).ToJSON();
  }

  ReplyAsync(static_cast<DatacontrolInstance*>(user_data), info->callbackId,
             providerResult, &obj);
  delete info;
  IdMap.erase(requestId);
}

static data_control_sql_response_cb sqlResponseCallback = {
  SQLSelectResponseCallback,
  SQLInsertResponseCallback,
  SQLUpdateResponseCallback,
  SQLDeleteResponseCallback
};
static data_control_map_response_cb mapResponseCallback = {
  MAPGetResponseCallback,
  MAPSetResponseCallback,
  MAPAddResponseCallback,
  MAPRemoveReponseCallback
};

#define RETURN_IF_FAIL(result, msg) \
    do {\
      if (result != DATA_CONTROL_ERROR_NONE) {\
        LoggerE(msg" : %s", ::get_error_message(result));\
        return result;\
      }\
    } while (0)\

int DatacontrolInstance::RunMAPDataControlJob(const std::string& providerId,
                                              const std::string& dataId,
                                              int callbackId,
                                              int userRequestId,
                                              DataControlJob job) {
  LoggerD("Enter");
  int result = DATA_CONTROL_ERROR_NONE;
  std::unique_ptr<DatacontrolInformation> info {new DatacontrolInformation()};
  info->callbackId = callbackId;
  info->userDefinedRequestId = userRequestId;
  data_control_h handle;

  SCOPE_EXIT {
    ::data_control_map_destroy(handle);
  };

  result = ::data_control_map_create(&handle);
  RETURN_IF_FAIL(result,
                 "Creating map data control handle is failed with error");

  result = ::data_control_map_set_provider_id(handle, providerId.c_str());
  RETURN_IF_FAIL(result,
                 "Setting provider id is failed with error");

  result = ::data_control_map_set_data_id(handle, dataId.c_str());
  RETURN_IF_FAIL(result,
                 "Setting data id is failed th error");

  result =
      ::data_control_map_register_response_cb(handle, &mapResponseCallback,
                                              this);
  RETURN_IF_FAIL(result, "Setting result Callback failed with error");

  result = job(handle, &info->requestId);
  RETURN_IF_FAIL(result, "Doing job failed with error");

  IdMap[info->requestId] = info.get();

  info.release();

  return result;
}
int DatacontrolInstance::RunSQLDataControlJob(const std::string& providerId,
                                              const std::string& dataId,
                                              int callbackId,
                                              int userRequestId,
                                              DataControlJob job) {
  LoggerD("Enter");
  int result = DATA_CONTROL_ERROR_NONE;
  std::unique_ptr<DatacontrolInformation> info {new DatacontrolInformation()};
  info->callbackId = callbackId;
  info->userDefinedRequestId = userRequestId;
  data_control_h handle;

  SCOPE_EXIT {
    ::data_control_sql_destroy(handle);
  };

  result = ::data_control_sql_create(&handle);
  RETURN_IF_FAIL(result,
                 "Creating sql data control handle is failed with error");

  result = ::data_control_sql_set_provider_id(handle, providerId.c_str());
  RETURN_IF_FAIL(result, "Setting provider id is failed with error");

  result = ::data_control_sql_set_data_id(handle, dataId.c_str());
  RETURN_IF_FAIL(result, "Setting data id is failed th error");

  result =
      ::data_control_sql_register_response_cb(handle, &sqlResponseCallback,
                                              this);
  RETURN_IF_FAIL(result, "Setting result Callback failed with error");

  result = job(handle, &info->requestId);
  RETURN_IF_FAIL(result, "Doing job failed with error");

  IdMap[info->requestId] = info.get();

  info.release();

  return result;
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

void DatacontrolInstance::DataControlManagerGetdatacontrolconsumer(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)
}
void DatacontrolInstance::SQLDataControlConsumerInsert(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "reqId", out)
  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)
  CHECK_EXIST(args, "insertionData", out)

  const std::string& providerId = args.get("providerId").get<std::string>();
  const std::string& dataId = args.get("dataId").get<std::string>();
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  int reqId = static_cast<int>(args.get("reqId").get<double>());
  picojson::object insertionData =
      args.get("insertionData").get<picojson::object>();

  if (!insertionData.count("columns") || !insertionData.count("values")) {
    ReportError(TypeMismatchException(
            "columns and values is required insertionData argument"), out);
    return;
  }
  if (!insertionData["columns"].is<picojson::array>() ||
      !insertionData["values"].is<picojson::array>()) {
    ReportError(TypeMismatchException("columns and values type must be array"),
                out);
    return;
  }

  int result = RunSQLDataControlJob(providerId, dataId, callbackId, reqId,
                                    [&insertionData](data_control_h& handle,
                                        int *requestId) -> int {
    picojson::array columns = insertionData["columns"].get<picojson::array>();
    picojson::array values = insertionData["values"].get<picojson::array>();

    picojson::array::size_type columnsLength = columns.size();
    picojson::array::size_type valuesLength = values.size();

    picojson::array::size_type size = std::min(columnsLength, valuesLength);

    bundle * b = ::bundle_create();
    SCOPE_EXIT {
      bundle_free(b);
    };
    for (unsigned i=0; i < size; i++) {
      picojson::value& column = columns[i];
      picojson::value& value = values[i];

      if (!column.is<std::string>() || !value.is<std::string>()) {
        break;
      }

      std::string& columnName = column.get<std::string>();
      std::string valueString = value.get<std::string>();

      bundle_add_str(b, columnName.c_str(), valueString.c_str());
    }

    return ::data_control_sql_insert(handle, b, requestId);
  });

  if (result == DATA_CONTROL_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    if (result == DATA_CONTROL_ERROR_IO_ERROR) {
      ReportError(IOException(get_error_message(result)), out);
    } else if (result == DATA_CONTROL_ERROR_PERMISSION_DENIED) {
      ReportError(SecurityException(get_error_message(result)), out);
    } else {
      ReportError(UnknownException(get_error_message(result)), out);
    }
  }
}
void DatacontrolInstance::SQLDataControlConsumerUpdate(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "reqId", out)
  CHECK_EXIST(args, "where", out)
  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)
  CHECK_EXIST(args, "updateData", out)

  const std::string& providerId = args.get("providerId").get<std::string>();
  const std::string& dataId = args.get("dataId").get<std::string>();
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  int reqId = static_cast<int>(args.get("reqId").get<double>());
  const std::string& where = args.get("where").get<std::string>();
  picojson::object updateData = args.get("updateData").get<picojson::object>();

  if (!updateData.count("columns") || !updateData.count("values")) {
    ReportError(TypeMismatchException(
            "columns and values is required updateData argument"), out);
    return;
  }

  if (!updateData["columns"].is<picojson::array>() ||
      !updateData["values"].is<picojson::array>()) {
    ReportError(TypeMismatchException(
            "columns and values type must be array"), out);
    return;
  }

  int result = RunSQLDataControlJob(providerId, dataId, callbackId, reqId,
                                    [&updateData, &where](
                                        data_control_h& handle,
                                        int *requestId) -> int {
    LoggerD("Enter");
    picojson::array columns = updateData["columns"].get<picojson::array>();
    picojson::array values = updateData["values"].get<picojson::array>();

    picojson::array::size_type columnsLength = columns.size();
    picojson::array::size_type valuesLength = values.size();

    picojson::array::size_type size = std::min(columnsLength, valuesLength);

    bundle * b = ::bundle_create();
    SCOPE_EXIT {
      bundle_free(b);
    };
    for (unsigned i=0; i < size; i++) {
      picojson::value& column = columns[i];
      picojson::value& value = values[i];

      if (!column.is<std::string>() || !value.is<std::string>()) {
        break;
      }

      std::string& columnName = column.get<std::string>();
      std::string valueString = value.get<std::string>();

      bundle_add_str(b, columnName.c_str(), valueString.c_str());
    }

    return ::data_control_sql_update(handle, b, where.c_str(), requestId);
  });

  if (result == DATA_CONTROL_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    if (result == DATA_CONTROL_ERROR_IO_ERROR) {
      ReportError(IOException(get_error_message(result)), out);
    } else if (result == DATA_CONTROL_ERROR_PERMISSION_DENIED) {
      ReportError(SecurityException(get_error_message(result)), out);
    } else {
      ReportError(UnknownException(get_error_message(result)), out);
    }
  }
}

void DatacontrolInstance::SQLDataControlConsumerRemove(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "reqId", out)
  CHECK_EXIST(args, "where", out)
  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)

  const std::string& providerId = args.get("providerId").get<std::string>();
  const std::string& dataId = args.get("dataId").get<std::string>();
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  int reqId = static_cast<int>(args.get("reqId").get<double>());
  const std::string& where = args.get("where").get<std::string>();

  int result = RunSQLDataControlJob(providerId, dataId, callbackId, reqId,
                                    [&where](data_control_h& handle,
                                        int *requestId) -> int {
    return ::data_control_sql_delete(handle, where.c_str(), requestId);
  });
  if (result == DATA_CONTROL_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    if (result == DATA_CONTROL_ERROR_IO_ERROR) {
      ReportError(IOException(get_error_message(result)), out);
    } else if (result == DATA_CONTROL_ERROR_PERMISSION_DENIED) {
      ReportError(SecurityException(get_error_message(result)), out);
    } else {
      ReportError(UnknownException(get_error_message(result)), out);
    }
  }
}

void DatacontrolInstance::SQLDataControlConsumerSelect(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "reqId", out)
  CHECK_EXIST(args, "columns", out)
  CHECK_EXIST(args, "where", out)

  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)

  const std::string& providerId = args.get("providerId").get<std::string>();
  const std::string& dataId = args.get("dataId").get<std::string>();
  const picojson::array columns = args.get("columns").get<picojson::array>();
  const std::string& where = args.get("where").get<std::string>();

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  int reqId = static_cast<int>(args.get("reqId").get<double>());

  int page = 0, maxNumberPerPage = 0;
  if (args.contains("page")) {
    page = static_cast<int>(args.get("page").get<double>());
  }
  if (args.contains("maxNumberPerPage")) {
    maxNumberPerPage =
        static_cast<int>(args.get("maxNumberPerPage").get<double>());
  }

  int result = RunSQLDataControlJob(providerId, dataId, callbackId, reqId,
                                    [&columns, &where, page, maxNumberPerPage](
                                        data_control_h& handle,
                                        int *requestId) -> int {
    LoggerD("Enter");
    std::vector<const char*> temp;
    for (auto& s : columns) temp.push_back(s.get<std::string>().c_str());
    int columnCount = static_cast<int>(temp.size());
    char** cColumns = const_cast<char**>(&*temp.begin());

    if (page > 0 && maxNumberPerPage > 0) {
      return ::data_control_sql_select_with_page(handle, cColumns,
                                                 columnCount, where.c_str(),
                                                 "1 ASC", page,
                                                 maxNumberPerPage, requestId);
    } else {
      return ::data_control_sql_select(handle, cColumns, columnCount,
                                       where.c_str(), "1 ASC", requestId);
    }
  });

  if (result == DATA_CONTROL_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    if (result == DATA_CONTROL_ERROR_IO_ERROR) {
      ReportError(IOException(get_error_message(result)), out);
    } else if (result == DATA_CONTROL_ERROR_PERMISSION_DENIED) {
      ReportError(SecurityException(get_error_message(result)), out);
    } else {
      ReportError(UnknownException(get_error_message(result)), out);
    }
  }
}
void DatacontrolInstance::MappedDataControlConsumerAddvalue(
    const picojson::value& args,
    picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "reqId", out)
  CHECK_EXIST(args, "key", out)
  CHECK_EXIST(args, "value", out)
  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)

  const std::string& providerId = args.get("providerId").get<std::string>();
  const std::string& dataId = args.get("dataId").get<std::string>();
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  int reqId = static_cast<int>(args.get("reqId").get<double>());
  const std::string& key = args.get("key").get<std::string>();
  const std::string& value = args.get("value").get<std::string>();

  int result = RunMAPDataControlJob(providerId, dataId, callbackId, reqId,
                                    [&key, &value](data_control_h& handle,
                                                   int *requestId) -> int {
    return ::data_control_map_add(handle, key.c_str(), value.c_str(),
                                  requestId);
  });

  if (result == DATA_CONTROL_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    if (result == DATA_CONTROL_ERROR_IO_ERROR) {
      ReportError(IOException(get_error_message(result)), out);
    } else if (result == DATA_CONTROL_ERROR_PERMISSION_DENIED) {
      ReportError(SecurityException(get_error_message(result)), out);
    } else {
      ReportError(UnknownException(get_error_message(result)), out);
    }
  }
}
void DatacontrolInstance::MappedDataControlConsumerRemovevalue(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "reqId", out)
  CHECK_EXIST(args, "key", out)
  CHECK_EXIST(args, "value", out)
  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)

  const std::string& providerId = args.get("providerId").get<std::string>();
  const std::string& dataId = args.get("dataId").get<std::string>();
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  int reqId = static_cast<int>(args.get("reqId").get<double>());
  const std::string& key = args.get("key").get<std::string>();
  const std::string& value = args.get("value").get<std::string>();

  int result = RunMAPDataControlJob(providerId, dataId, callbackId, reqId,
                                    [&key, &value](data_control_h& handle,
                                                   int *requestId) -> int {
    return ::data_control_map_remove(handle, key.c_str(), value.c_str(),
                                     requestId);
  });

  if (result == DATA_CONTROL_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    if (result == DATA_CONTROL_ERROR_IO_ERROR) {
      ReportError(IOException(get_error_message(result)), out);
    } else if (result == DATA_CONTROL_ERROR_PERMISSION_DENIED) {
      ReportError(SecurityException(get_error_message(result)), out);
    } else {
      ReportError(UnknownException(get_error_message(result)), out);
    }
  }
}
void DatacontrolInstance::MappedDataControlConsumerGetvalue(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "reqId", out)
  CHECK_EXIST(args, "key", out)
  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)

  const std::string& providerId = args.get("providerId").get<std::string>();
  const std::string& dataId = args.get("dataId").get<std::string>();
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  int reqId = static_cast<int>(args.get("reqId").get<double>());
  const std::string& key = args.get("key").get<std::string>();

  int result = RunMAPDataControlJob(providerId, dataId, callbackId, reqId,
                                    [&key](data_control_h& handle,
                                           int *requestId) -> int {
    return ::data_control_map_get(handle, key.c_str(), requestId);
  });

  if (result == DATA_CONTROL_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    if (result == DATA_CONTROL_ERROR_IO_ERROR) {
      ReportError(IOException(get_error_message(result)), out);
    } else if (result == DATA_CONTROL_ERROR_PERMISSION_DENIED) {
      ReportError(SecurityException(get_error_message(result)), out);
    } else {
      ReportError(UnknownException(get_error_message(result)), out);
    }
  }
}
void DatacontrolInstance::MappedDataControlConsumerUpdatevalue(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Enter");
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "reqId", out)
  CHECK_EXIST(args, "key", out)
  CHECK_EXIST(args, "oldValue", out)
  CHECK_EXIST(args, "newValue", out)
  CHECK_EXIST(args, "providerId", out)
  CHECK_EXIST(args, "dataId", out)

  const std::string& providerId = args.get("providerId").get<std::string>();
  const std::string& dataId = args.get("dataId").get<std::string>();
  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  int reqId = static_cast<int>(args.get("reqId").get<double>());
  const std::string& key = args.get("key").get<std::string>();
  const std::string& oldValue = args.get("oldValue").get<std::string>();
  const std::string& newValue = args.get("newValue").get<std::string>();

  int result = RunMAPDataControlJob(providerId, dataId, callbackId, reqId,
                                    [&key, &oldValue, &newValue](
                                        data_control_h& handle,
                                        int *requestId) -> int {
    return ::data_control_map_set(handle, key.c_str(),
                                  oldValue.c_str(), newValue.c_str(),
                                  requestId);
  });

  if (result == DATA_CONTROL_ERROR_NONE) {
    ReportSuccess(out);
  } else {
    if (result == DATA_CONTROL_ERROR_IO_ERROR) {
      ReportError(IOException(get_error_message(result)), out);
    } else if (result == DATA_CONTROL_ERROR_PERMISSION_DENIED) {
      ReportError(SecurityException(get_error_message(result)), out);
    } else {
      ReportError(UnknownException(get_error_message(result)), out);
    }
  }
}


#undef CHECK_EXIST

}  // namespace datacontrol
}  // namespace extension
