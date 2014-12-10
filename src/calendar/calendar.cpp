/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "calendar.h"

#include <native-context.h>

#include "logger.h"
#include "converter.h"
#include "native-plugin.h"
#include "task-queue.h"
#include "calendar-manager.h"
#include "calendar-privilege.h"
#include "calendar-item.h"


namespace webapi {
namespace calendar {

using namespace webapi::common;

int Calendar::current_db_version_ = 0;
std::map<std::string, std::string> Calendar::listeners_registered_;

Calendar& Calendar::GetInstance() {
    static Calendar instance;
    return instance;
}

Calendar::~Calendar() {
    int ret;

    if (listeners_registered_.find("EVENT") != listeners_registered_.end()) {
        ret = calendar_db_remove_changed_cb(_calendar_event._uri, ChangeCallback, nullptr);
        if (CALENDAR_ERROR_NONE != ret) {
            LoggerE("Remove calendar event change callback error");
        }
    }

    if (listeners_registered_.find("TASK") != listeners_registered_.end()) {
        ret = calendar_db_remove_changed_cb(_calendar_todo._uri, ChangeCallback, nullptr);
        if (CALENDAR_ERROR_NONE != ret) {
            LoggerE("Remove calendar todo change callback error");
        }
    }
}

void Calendar::Get(const json::Object& args, json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarRead);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    int calendar_id = common::stol(FromJson<std::string>(args, "calendarId"));
    CalendarRecordPtr calendar_ptr = CalendarRecord::GetById(calendar_id, _calendar_book._uri);
    int type = CalendarRecord::GetInt(calendar_ptr.get(), _calendar_book.store_type);

    int id;
    if (type == CALENDAR_BOOK_TYPE_EVENT) {
        id = common::stol(FromJson<std::string>(args, "id", "uid"));
    } else {
        id = common::stol(FromJson<std::string>(args, "id"));
    }

    CalendarRecordPtr record_ptr = CalendarRecord::GetById(id, CalendarRecord::TypeToUri(type));
    json::Value record_obj = json::Value(json::Object());
    CalendarItem::ToJson(type, record_ptr.get(), &record_obj.get<json::Object>());

    NativePlugin::ReportSuccess(record_obj, out);
}

void Calendar::Add(const json::Object& args, json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarWrite);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    const auto& item = FromJson<json::Object>(args, "item");
    int type = CalendarRecord::TypeToInt(FromJson<json::String>(args, "type"));

    CalendarRecordPtr item_ptr = CalendarItem::Create(type);
    CalendarItem::FromJson(type, item_ptr.get(), item);
    int record_id = CalendarRecord::Insert(item_ptr.get());

    json::Value result = json::Value(json::Object());
    json::Object& result_obj = result.get<json::Object>();
    result_obj.insert(std::make_pair("uid", std::to_string(record_id)));

    if (type == CALENDAR_BOOK_TYPE_EVENT) {
        std::string rid = CalendarRecord::GetString(item_ptr.get(), _calendar_event.recurrence_id);
        if (!rid.empty()) {
            result_obj["rid"] = json::Value(rid);
        } else {
            result_obj["rid"] = json::Value();
        }
    }

    NativePlugin::ReportSuccess(result, out);
}

void Calendar::AddBatch(const common::json::Object& args, common::json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarWrite);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    auto& items = FromJson<json::Array>(args, "items");
    if (items.empty()) {
        throw InvalidValuesException("No items");
    }

    int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
    const char* view_uri = CalendarRecord::TypeToUri(type);

    auto batch_func = [=](const JsonValuePtr& response)->void {
        json::Object& response_obj = response->get<json::Object>();

        try {
            calendar_list_h list = NULL;
            if (CALENDAR_ERROR_NONE != calendar_list_create(&list)) {
                LoggerE("Could not create list for batch operation");
                throw UnknownException("Could not create list for batch operation");
            }
            CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

            int ret;
            calendar_record_h record;

            for (auto& item : items) {
                ret = calendar_record_create(view_uri, &record);
                if (CALENDAR_ERROR_NONE != ret) {
                    LoggerW("Can't create platform record %d", ret);
                    throw UnknownException("Can't create platform record");
                }
                CalendarItem::FromJson(type, record, item.get<json::Object>());

                if (CALENDAR_ERROR_NONE != calendar_list_add(list_ptr.get(), record)) {
                    LoggerE("Could not add record to list events");
                    throw InvalidValuesException("Could not add record to list");
                }
            }

            int* ids;
            int count;
            ret = calendar_db_insert_records(list_ptr.get(), &ids, &count);
            if (CALENDAR_ERROR_NONE != ret) {
                LoggerE("calendar_db_insert_records failed.");
                if (CALENDAR_ERROR_INVALID_PARAMETER == ret) {
                    LoggerE("CALENDAR_ERROR_INVALID_PARAMETER.");
                    throw InvalidValuesException("Parameter is invalid");
                } else {
                    LoggerE("CALENDAR_ERROR_DB_FAILED");
                    throw UnknownException("CALENDAR_ERROR_DB_FAILED occurred");
                }
            }

            json::Value result = json::Value(json::Array());
            json::Array& array = result.get<json::Array>();

            for (int i = 0; i < count; i++) {
                json::Value id = json::Value(json::Object());
                json::Object& id_obj = id.get<json::Object>();

                id_obj.insert(std::make_pair("uid", std::to_string(ids[i])));

                if (type == CALENDAR_BOOK_TYPE_EVENT) {
                    id_obj.insert(std::make_pair("rid", json::Value()));
                }

                array.push_back(id);
            }
            free(ids);

            NativePlugin::ReportSuccess(result, response_obj);
        } catch (const BasePlatformException& e) {
            NativePlugin::ReportError(e, response_obj);
        }
    };

    int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);
    auto after_batch_func = [callback_handle](const JsonValuePtr& response) {
        wrt::common::NativeContext::GetInstance()
                ->InvokeCallback(callback_handle, response->serialize());
    };

    TaskQueue::GetInstance().Queue<json::Value>(batch_func, after_batch_func,
                                                JsonValuePtr(new json::Value(json::Object())));

    NativePlugin::ReportSuccess(out);
}

void Calendar::Update(const common::json::Object& args, common::json::Object& /*out*/) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarWrite);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    const auto& item = FromJson<json::Object>(args, "item");
    int type = CalendarRecord::TypeToInt(FromJson<json::String>(args, "type"));

    bool update_all = true;

    int id;
    if (type == CALENDAR_BOOK_TYPE_EVENT) {
        id = common::stol(FromJson<std::string>(item, "id", "uid"));
        if (!IsNull(args, "updateAllInstances")) {
            update_all = FromJson<bool>(args, "updateAllInstances");
        }
    } else {
        id = common::stol(FromJson<std::string>(item, "id"));
    }

    CalendarRecordPtr record_ptr = CalendarRecord::GetById(id, CalendarRecord::TypeToUri(type));
    CalendarItem::FromJson(type, record_ptr.get(), item);

    if (type == CALENDAR_BOOK_TYPE_TODO || update_all || common::IsNull(item, "recurrenceRule")) {
        if (CALENDAR_ERROR_NONE != calendar_db_update_record(record_ptr.get())) {
            LoggerE("Can't update calendar item");
            throw UnknownException("Can't update calendar item");
        }
    } else {
        // first update the parent event
        std::string exdate = CalendarItem::ExceptionsFromJson(
                common::FromJson<json::Array>(item, "recurrenceRule", "exceptions"));
        if (!common::IsNull(common::FromJson<json::Object>(item, "id"), "rid")) {
            exdate.append(common::FromJson<std::string>(item, "id", "rid"));
        }
        CalendarRecord::SetString(record_ptr.get(), _calendar_event.exdate, exdate);

        // don't set the recurrence id for the parent event
        CalendarRecord::SetString(record_ptr.get(), _calendar_event.recurrence_id, "");

        if (CALENDAR_ERROR_NONE != calendar_db_update_record(record_ptr.get())) {
            LoggerE("Can't update calendar item");
            throw UnknownException("Can't update calendar item");
        }

        // now add the detached child event
        CalendarRecordPtr item_ptr = CalendarItem::Create(type);
        CalendarItem::FromJson(type, item_ptr.get(), item);
        CalendarRecord::Insert(item_ptr.get());
    }
}

void Calendar::UpdateBatch(const common::json::Object& args, common::json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarWrite);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    auto& items = FromJson<json::Array>(args, "items");
    if (items.empty()) {
        throw InvalidValuesException("No items");
    }

    bool update_all = true;
    if (!IsNull(args, "updateAllInstances")) {
        update_all = FromJson<bool>(args, "updateAllInstances");
    }

    int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
    const char* view_uri = CalendarRecord::TypeToUri(type);

    auto batch_func = [=](const JsonValuePtr& response)->void {
        json::Object& response_obj = response->get<json::Object>();

        try {
            calendar_list_h list = NULL;
            if (CALENDAR_ERROR_NONE != calendar_list_create(&list)) {
                LoggerE("Could not create list for batch operation");
                throw UnknownException("Could not create list for batch operation");
            }
            CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

            int ret, id;
            calendar_record_h record;

            for (auto& item : items) {

                const json::Object& item_obj = item.get<json::Object>();
                if (type == CALENDAR_BOOK_TYPE_EVENT) {
                    id = common::stol(FromJson<std::string>(item_obj, "id", "uid"));
                } else {
                    id = common::stol(FromJson<std::string>(item_obj, "id"));
                }

                ret = calendar_db_get_record(view_uri, id, &record);
                if (CALENDAR_ERROR_NONE != ret) {
                    LoggerW("Can't get platform record %d", ret);
                    throw UnknownException("Can't get platform record");
                }
                CalendarItem::FromJson(type, record, item.get<json::Object>());

                if (CALENDAR_ERROR_NONE != calendar_list_add(list_ptr.get(), record)) {
                    LoggerE("Could not add record to list events");
                    throw InvalidValuesException("Could not add record to list");
                }
            }

            if (type == CALENDAR_BOOK_TYPE_TODO || update_all) {
                if (CALENDAR_ERROR_NONE != calendar_db_update_records(list_ptr.get())) {
                    LoggerE("Can't update calendar items");
                    throw UnknownException("Can't update calendar items");
                }
            } else {
                // @todo update the exdate for a recurring parent event and add a new child event
            }

            NativePlugin::ReportSuccess(response_obj);
        } catch (const BasePlatformException& e) {
            NativePlugin::ReportError(e, response_obj);
        }
    };

    int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);
    auto after_batch_func = [callback_handle](const JsonValuePtr& response) {
        wrt::common::NativeContext::GetInstance()
                ->InvokeCallback(callback_handle, response->serialize());
    };

    TaskQueue::GetInstance().Queue<json::Value>(batch_func, after_batch_func,
            JsonValuePtr(new json::Value(json::Object())));

    NativePlugin::ReportSuccess(out);
}

void Calendar::Remove(const common::json::Object& args, common::json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarWrite);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    int type = CalendarRecord::TypeToInt(FromJson<json::String>(args, "type"));

    int id;
    if (type == CALENDAR_BOOK_TYPE_EVENT) {
        id = common::stol(FromJson<std::string>(args, "id", "uid"));
    } else {
        id = common::stol(FromJson<std::string>(args, "id"));
    }

    CalendarItem::Remove(type, id);

    NativePlugin::ReportSuccess(out);
}

void Calendar::Find(const json::Object& args, json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarWrite);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    // TODO implement calendar filter and sorting in native code.
    int calendar_id = common::stol(FromJson<std::string>(args, "calendarId"));
    int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);

    auto get = [calendar_id](const std::shared_ptr<json::Value> & response)->void {
        LoggerD("Entered");
        try {
            int error_code = 0;
            CalendarRecordPtr calendar_ptr =
                    CalendarRecord::GetById(calendar_id, _calendar_book._uri);
            int type = CalendarRecord::GetInt(calendar_ptr.get(), _calendar_book.store_type);
            calendar_query_h calendar_query = nullptr;
            if (type == CALENDAR_BOOK_TYPE_EVENT) {
                error_code = calendar_query_create(_calendar_event._uri, &calendar_query);
            } else {
                error_code = calendar_query_create(_calendar_todo._uri, &calendar_query);
            }
            if (CALENDAR_ERROR_NONE != error_code) {
                throw UnknownException("calendar_query_create failed");
            }

            CalendarQueryPtr calendar_query_ptr(calendar_query, CalendarRecord::QueryDeleter);

            calendar_list_h record_list = nullptr;
            error_code = calendar_db_get_records_with_query(calendar_query, 0, 0, &record_list);
            if (CALENDAR_ERROR_NONE != error_code) {
                throw UnknownException("calendar_db_get_records_with_query failed");
            }
            CalendarListPtr record_list_ptr(record_list, CalendarRecord::ListDeleter);

            int record_count = 0;
            error_code = calendar_list_get_count(record_list, &record_count);
            if (CALENDAR_ERROR_NONE != error_code) {
                throw UnknownException("calendar_list_get_count failed");
            }
            error_code = calendar_list_first(record_list);
            if (CALENDAR_ERROR_NONE != error_code) {
                throw UnknownException("calendar_list_first failed");
            }

            json::Value result{json::Array{}};
            json::Array& calendarItems = result.get<json::Array>();
            calendarItems.reserve(record_count);
            for (int i = 0; i < record_count; ++i) {
                calendar_record_h current_record = NULL;
                error_code = calendar_list_get_current_record_p(record_list, &current_record);
                if (CALENDAR_ERROR_NONE != error_code) {
                    throw UnknownException("calendar_list_get_current_record_p failed");
                }
                json::Value record_obj = json::Value(json::Object());
                CalendarItem::ToJson(type, current_record, &record_obj.get<json::Object>());
                calendarItems.push_back(record_obj);

                error_code = calendar_list_next(record_list);
                if (CALENDAR_ERROR_NONE != error_code) {
                    LoggerE("calendar_list_next failed (%i/%i)", i, record_count);
                    break;
                }
            }

            NativePlugin::ReportSuccess(result, response->get<json::Object>());
        }
        catch (const BasePlatformException& e) {
            LoggerE("error: %s: %s", e.name().c_str(), e.message().c_str());
            NativePlugin::ReportError(e, response->get<json::Object>());
        }
    };

    auto get_response = [callback_handle](const std::shared_ptr<json::Value> & response)->void {
        wrt::common::NativeContext::GetInstance()->InvokeCallback(callback_handle,
                                                                  response->serialize());
    };

    TaskQueue::GetInstance().Queue<json::Value>(
            get, get_response, std::shared_ptr<json::Value>(new json::Value(json::Object())));

    NativePlugin::ReportSuccess(out);
}

void Calendar::RemoveBatch(const common::json::Object& args, common::json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarWrite);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    auto& ids = FromJson<json::Array>(args, "ids");
    if (ids.empty()) {
        throw InvalidValuesException("No items");
    }

    int type = CalendarRecord::TypeToInt(FromJson<std::string>(args, "type"));
    const char* view_uri = CalendarRecord::TypeToUri(type);

    auto batch_func = [=](const JsonValuePtr& response)->void {
        json::Object& response_obj = response->get<json::Object>();

        try {

            std::vector<int> ids_to_remove;
            int id;
            for (int i = 0, size = ids.size(); i < size; i++) {

                if (type == CALENDAR_BOOK_TYPE_EVENT) {
                    id = common::stol(FromJson<std::string>(ids.at(i).get<json::Object>(), "uid"));
                } else {
                    id = common::stol(ids.at(i).get<std::string>());
                }

                CalendarRecordPtr record_ptr = CalendarItem::GetById(id, view_uri);

                if (type == CALENDAR_BOOK_TYPE_EVENT) {
                    const std::string& rid = CalendarRecord::GetString(record_ptr.get(),
                            _calendar_event.recurrence_id);
                    if (rid.empty()) {
                        ids_to_remove.push_back(id);
                    } else {
                        // @todo handle recurrence_id
                    }
                } else {
                    ids_to_remove.push_back(id);
                }
            }

            int ret;
            if (ids_to_remove.size() > 0) {
                ret = calendar_db_delete_records(view_uri, &ids_to_remove[0], ids_to_remove.size());

                if (CALENDAR_ERROR_NONE != ret) {
                    LoggerE("calendar_db_delete_records failed.");
                    if (CALENDAR_ERROR_INVALID_PARAMETER == ret) {
                        LoggerE("CALENDAR_ERROR_INVALID_PARAMETER");
                        throw InvalidValuesException("Parameter is invalid");
                    } else {
                        LoggerE("CALENDAR_ERROR_DB_FAILED");
                        throw UnknownException("UnknownError");
                    }
                }
            }

            NativePlugin::ReportSuccess(response_obj);
        } catch (const BasePlatformException& e) {
            NativePlugin::ReportError(e, response_obj);
        }
    };

    int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);
    auto after_batch_func = [callback_handle](const JsonValuePtr& response) {
        wrt::common::NativeContext::GetInstance()
                ->InvokeCallback(callback_handle, response->serialize());
    };

    TaskQueue::GetInstance().Queue<json::Value>(batch_func, after_batch_func,
            JsonValuePtr(new json::Value(json::Object())));

    NativePlugin::ReportSuccess(out);
}

void Calendar::AddChangeListener(const json::Object& args, json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarRead);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    const std::string& type = FromJson<std::string>(args, "type");
    const std::string& listener_id = FromJson<std::string>(args, "listenerId");

    int ret;
    if (listeners_registered_.find(type) == listeners_registered_.end()) {
        const char* view_uri = CalendarRecord::TypeToUri(type);
        ret = calendar_db_add_changed_cb(view_uri, ChangeCallback, nullptr);
        if (CALENDAR_ERROR_NONE != ret) {
            LoggerE("Add calendar change callback error for type %s", type.c_str());
            throw UnknownException("Add calendar change callback error");
        }

        ret = calendar_db_get_current_version(&current_db_version_);
        if (CALENDAR_ERROR_NONE != ret) {
            current_db_version_ = 0;
            LoggerE("Can't get calendar db version");
            throw UnknownException("Can't get calendar db version");
        }

        listeners_registered_[type] = listener_id;
    }

    NativePlugin::ReportSuccess(out);
}

void Calendar::RemoveChangeListener(const json::Object& args, json::Object& out) {
    LoggerD("enter");

    NativePlugin::CheckAccess(Privilege::kCalendarRead);

    if (!CalendarManager::GetInstance().IsConnected()) {
        throw UnknownException("DB Connection failed.");
    }

    const std::string& type = FromJson<std::string>(args, "type");

    int ret;
    if (listeners_registered_.find(type) != listeners_registered_.end()) {
        const char* view_uri = CalendarRecord::TypeToUri(type);
        ret = calendar_db_remove_changed_cb(view_uri, ChangeCallback, nullptr);
        if (CALENDAR_ERROR_NONE != ret) {
            LoggerE("Remove calendar change callback error for type %s", type.c_str());
            throw UnknownException("Remove calendar change callback error");
        }
        listeners_registered_.erase(type);
    }

    NativePlugin::ReportSuccess(out);
}

void Calendar::ChangeCallback(const char* view_uri, void*) {
    LoggerD("enter");

    calendar_list_h list = nullptr;
    int ret, updated_version;
    ret = calendar_db_get_changes_by_version(view_uri, CALENDAR_BOOK_FILTER_ALL,
                                             current_db_version_, &list, &updated_version);
    if (CALENDAR_ERROR_NONE != ret) {
        LoggerE("Can't get the changed item list");
        throw UnknownException("Can't get the changed item list");
    }
    CalendarListPtr list_ptr = CalendarListPtr(list, CalendarRecord::ListDeleter);

    int count;
    ret = calendar_list_get_count(list_ptr.get(), &count);
    if (CALENDAR_ERROR_NONE != ret) {
        LoggerE("Can't get the changed item count");
        throw UnknownException("Can't get the changed item count");
    }
    LoggerD("Item count: %d", count);

    calendar_list_first(list_ptr.get());

    int id, calendar_id, status;
    calendar_record_h update_info = nullptr;

    int type = CalendarRecord::TypeToInt(view_uri);

    // prepare response object
    json::Value response = json::Value(json::Object());
    json::Object& response_obj = response.get<json::Object>();

    json::Array& added = response_obj.insert(std::make_pair("added", json::Array()))
            .first->second.get<json::Array>();
    json::Array& updated = response_obj.insert(std::make_pair("updated", json::Array()))
            .first->second.get<json::Array>();
    json::Array& removed = response_obj.insert(std::make_pair("removed", json::Array()))
            .first->second.get<json::Array>();

    while(count-- > 0) {
        ret = calendar_list_get_current_record_p(list, &update_info);
        if (CALENDAR_ERROR_NONE != ret) {
            LoggerE("Can't get current record");
            throw UnknownException("Can't get current record");
        }

        id = CalendarRecord::GetInt(update_info, _calendar_updated_info.id);
        status = CalendarRecord::GetInt(update_info, _calendar_updated_info.modified_status);

        if (status == CALENDAR_RECORD_MODIFIED_STATUS_DELETED) {
            calendar_id = CalendarRecord::GetInt(update_info,
                    _calendar_updated_info.calendar_book_id);

            json::Value removed_row = json::Value(json::Object());
            json::Object& removed_obj = removed_row.get<json::Object>();
            removed_obj.insert(std::make_pair("id", json::Value(std::to_string(id))));
            removed_obj.insert(std::make_pair("calendarId",
                    json::Value(std::to_string(calendar_id))));

            removed.push_back(removed_row);

            calendar_list_next(list);
            continue;
        }

        try {
            CalendarRecordPtr record_ptr = CalendarRecord::GetById(id, view_uri);
            json::Value record_obj = json::Value(json::Object());
            CalendarItem::ToJson(type, record_ptr.get(), &record_obj.get<json::Object>());
            if (status == CALENDAR_RECORD_MODIFIED_STATUS_INSERTED) {
                added.push_back(record_obj);
            } else {
                updated.push_back(record_obj);
            }
        } catch (BasePlatformException &ex) {
            LoggerD("Can't get changed calendar item: %s", ex.message().c_str());
        }

        calendar_list_next(list);
    }

    if (added.empty()) {
        response_obj.erase("added");
    }
    if (updated.empty()) {
        response_obj.erase("updated");
    }
    if (removed.empty()) {
        response_obj.erase("removed");
    }

    ret = calendar_db_get_current_version(&updated_version);
    if (CALENDAR_ERROR_NONE != ret) {
        LoggerE("Can't get new version");
        throw UnknownException("Can't get new version");
    }
    current_db_version_ = updated_version;

    wrt::common::NativeContext::GetInstance()->FireEvent(
        listeners_registered_[CalendarRecord::TypeToString(type)], response.serialize());
}

} // namespace calendar
} // namespace webapi
