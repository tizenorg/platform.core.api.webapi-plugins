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

#include "contact/addressbook.h"

#include <wrt-common/native-context.h>
#include "common/converter.h"
#include "common/native-plugin.h"
#include "common/platform_exception.h"
#include "common/logger.h"
#include "common/task-queue.h"

#include <contacts.h>

namespace extension {
namespace contact {
namespace AddressBook {

using namespace common;
using namespace wrt::common;

typedef std::shared_ptr<JsonValue> shared_json_value;

namespace {
static const long kUnifiedAddressBookId = -1;
const char* kContactListenerId = "ContactChangeListener";

inline long AddressBookId(const JsonObject& obj) {
  return common::stol(FromJson<json::String>(obj, "addressBook", "id"));
}

inline bool IsUnified(const JsonObject& args) {
  return AddressBookId(args) == kUnifiedAddressBookId;
}

inline bool IsUnified(long id) { return id == kUnifiedAddressBookId; }

}  // anonymous namespace

void AddressBook_get(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactReadPrivileges);

  int contact_id = common::stol(FromJson<json::String>(args, "id"));

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h contacts_record = nullptr;
  err = contacts_db_get_record(_contacts_contact._uri, contact_id,
                               &contacts_record);
  if (CONTACTS_ERROR_NONE != err) {
    LOGW("Contacts record get error, error code: %d", err);
    throw common::NotFoundException("Contacts record get error");
  }
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  JsonValue result{json::Object{}};
  auto& result_obj = result.get<json::Object>();
  result_obj.insert(std::make_pair("id", std::to_string(contact_id)));
  ContactUtil::ImportContactFromContactsRecord(*contacts_record_ptr,
                                               &result_obj);
  ContactUtil::UpdateAdditionalInformation(contacts_record_ptr, &result_obj);

  NativePlugin::ReportSuccess(result, out);
}

void AddressBook_add(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactWritePrivileges);

  const json::Object& contact = FromJson<json::Object>(args, "contact");

  if (!IsNull(contact, "id")) {
    LOGW("Contact already exists");
    throw common::UnknownException("Contact already exists");
  }

  contacts_record_h contacts_record = nullptr;
  int err = 0;
  err = contacts_record_create(_contacts_contact._uri, &contacts_record);
  if (CONTACTS_ERROR_NONE != err) {
    LOGW("Contacts record create error, error code: %d", err);
    throw common::UnknownException("Contacts record create error");
  }

  // contacts_record starts to be protected by unique_ptr
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);
  ContactUtil::ExportContactToContactsRecord(*contacts_record_ptr, contact);

  int id = -1;
  err = contacts_db_insert_record(*contacts_record_ptr, &id);
  if (CONTACTS_ERROR_NONE != err) {
    LOGW("Contacts db insert error, error code: %d", err);
    throw common::UnknownException("Contacts db insert error");
  }

  contacts_record_h reset_record;
  err = contacts_db_get_record(_contacts_contact._uri, id, &reset_record);
  if (CONTACTS_ERROR_NONE != err) {
    LOGW("Contacts record get error, error code: %d", err);
    throw common::UnknownException("Contacts record get error");
  }
  if (nullptr != reset_record) {
    LOGE("reset");
    contacts_record_ptr.reset(&reset_record);
  }

  json::Value result{json::Object{}};
  json::Object& value_obj = result.get<json::Object>();
  value_obj.insert(std::make_pair("id", json::Value{std::to_string(id)}));

  ContactUtil::UpdateAdditionalInformation(contacts_record_ptr, &value_obj);

  NativePlugin::ReportSuccess(result, out);
}

void AddressBook_update(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactWritePrivileges);

  const json::Object& contact = FromJson<json::Object>(args, "contact");
  const json::Object& addressbook = FromJson<json::Object>(args, "addressBook");
  long contactId = common::stol(FromJson<json::String>(contact, "id"));

  if (IsNull(contact, "id")) {
    LOGW("Contact doesn't exist");
    throw common::UnknownException("Contact doesn't exist");
  }

  if (IsNull(addressbook, "id")) {
    LOGE("Contact is not saved in database");
    throw common::InvalidValuesException("Contact is not saved in database");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h to_update = nullptr;
  err = contacts_db_get_record(_contacts_contact._uri, contactId, &to_update);
  if (CONTACTS_ERROR_NONE != err) {
    LOGW("Problem with getting contact. Error: %d", err);
    throw common::NotFoundException("Problem with getting contact");
  }
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &to_update, ContactUtil::ContactsDeleter);
  ContactUtil::ExportContactToContactsRecord(*contacts_record_ptr, contact);
  err = contacts_db_update_record(*contacts_record_ptr);
  if (CONTACTS_ERROR_NONE != err) {
    if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
      LOGE("Error during executing contacts_db_update_record(). Error: %d",
           err);
      throw common::NotFoundException(
          "Error during executing contacts_db_update_record().");
    }
    if (CONTACTS_ERROR_DB == err) {
      LOGE("Error during executing contacts_db_update_record(). Error: %d",
           err);
      throw common::UnknownException(
          "Error during executing contacts_db_update_record().");
    }
  }

  json::Value result{json::Object{}};
  json::Object& value_obj = result.get<json::Object>();

  ContactUtil::UpdateAdditionalInformation(contacts_record_ptr, &value_obj);

  NativePlugin::ReportSuccess(result, out);
}

void AddressBook_remove(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactWritePrivileges);

  long contact_id = common::stol(FromJson<json::String>(args, "id"));

  if (contact_id < 0) {
    throw common::InvalidValuesException("Nagative contact id");
  }

  int err = CONTACTS_ERROR_NONE;
  err = contacts_db_delete_record(_contacts_contact._uri, contact_id);
  if (CONTACTS_ERROR_NO_DATA == err) {
    LOGW("Remove failed: contact not found, error code: %d", err);
    throw common::NotFoundException("Remove failed: contact not found");
  } else if (CONTACTS_ERROR_NONE != err) {
    LOGW("Contacts record delete error, error code: %d", err);
    throw common::UnknownException("Contacts record delete error");
  }
  NativePlugin::ReportSuccess(out);
}

void AddressBook_addBatch(const json::Object& args, json::Object& out) {
  LoggerD("Enter");
  long addressBookId = -1;
  const json::Array& batch_args = FromJson<json::Array>(args, "batchArgs");
  addressBookId = common::stol(FromJson<json::String>(args, "addressBookId"));
  addressBookId = addressBookId == -1 ? 0 : addressBookId;

  auto batch_func = [ addressBookId, batch_args ](const shared_json_value &
                                                  response)->void {
    NativePlugin::CheckAccess(ContactUtil::kContactWritePrivileges);
    json::Object& response_obj = response->get<json::Object>();
    json::Array& batch_result =
        response_obj.insert(
                         std::make_pair("result", json::Value{json::Array{}}))
            .first->second.get<json::Array>();

    try {
      unsigned length = batch_args.size();
      int error_code = 0;
      contacts_list_h contacts_list = NULL;
      error_code = contacts_list_create(&contacts_list);
      if (CONTACTS_ERROR_NONE != error_code) {
        LOGE("list creation failed, code: %d", error_code);
        throw new common::UnknownException("list creation failed");
      }
      ContactUtil::ContactsListHPtr contacts_list_ptr(
          &contacts_list, ContactUtil::ContactsListDeleter);

      for (auto& item : batch_args) {
        contacts_record_h contacts_record = nullptr;
        int err = 0;
        err = contacts_record_create(_contacts_contact._uri, &contacts_record);
        if (CONTACTS_ERROR_NONE != err) {
          LOGW("Contacts record create error, error code: %d", err);
          throw common::UnknownException("Contacts record create error");
        }
        ContactUtil::ContactsRecordHPtr x(&contacts_record,
                                          ContactUtil::ContactsDeleter);
        ContactUtil::ExportContactToContactsRecord(
            contacts_record, JsonCast<json::Object>(item));
        ContactUtil::SetIntInRecord(
            contacts_record, _contacts_contact.address_book_id, addressBookId);
        error_code = contacts_list_add(*contacts_list_ptr, *(x.release()));
        if (CONTACTS_ERROR_NONE != error_code) {
          LOGE("error during add record to list, code: %d", error_code);
          throw new common::UnknownException("error during add record to list");
        }
      }

      int* ids;
      unsigned int count;
      error_code = contacts_db_insert_records(*contacts_list_ptr, &ids, &count);
      if (CONTACTS_ERROR_NONE != error_code) {
        if (ids) {
          free(ids);
          ids = NULL;
        }
        LOGE("inserting contacts to db fails, code: %d", error_code);
        throw new common::UnknownException("inserting contacts to db fails");
      }
      if (length != count) {
        LOGW("Added different number of contacts");
      }

      for (unsigned int i = 0; i < count; i++) {
        json::Object out_object;
        contacts_record_h contact_record = nullptr;
        error_code = contacts_db_get_record(_contacts_contact._uri, ids[i],
                                            &contact_record);
        if (CONTACTS_ERROR_NONE != error_code) {
          if (ids) {
            free(ids);
            ids = NULL;
          }
          LOGW("Contacts record get error, error code: %d", error_code);
          throw common::UnknownException("Contacts record get error");
        }
        ContactUtil::ImportContactFromContactsRecord(contact_record,
                                                     &out_object);
        batch_result.push_back(json::Value{out_object});
      }
      if (ids) {
        free(ids);
        ids = NULL;
      }
      NativePlugin::ReportSuccess(response_obj);
    }
    catch (const BasePlatformException& error) {
      LOGE("Error %s, with msg: %s", error.name().c_str(),
           error.message().c_str());
      NativePlugin::ReportError(error, response_obj);
    }
  };
  int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);
  auto after_work_func = [=](const shared_json_value& response) {
    wrt::common::NativeContext::GetInstance()->InvokeCallback(
        callback_handle, response->serialize());
  };

  TaskQueue::GetInstance().Queue<json::Value>(
      batch_func, after_work_func,
      shared_json_value{new json::Value{json::Object{}}});

  NativePlugin::ReportSuccess(out);
}

void AddressBook_batchFunc(NativeFunction impl, const char* single_arg_name,
                           const json::Object& args, json::Object& out) {
  const json::Array& batch_args = FromJson<json::Array>(args, "batchArgs");
  const json::Object& address_book =
      FromJson<json::Object>(args, "addressBook");

  auto work_func = [=](const shared_json_value & response)->void {
    json::Object& response_obj = response->get<json::Object>();
    json::Array& batch_result =
        response_obj.insert(
                         std::make_pair("result", json::Value{json::Array{}}))
            .first->second.get<json::Array>();
    try {
      for (auto& item : batch_args) {
        json::Object single_args{};

        single_args.insert(std::make_pair("addressBook", address_book));
        single_args.insert(std::make_pair(single_arg_name, item));

        json::Object single_out;
        impl(single_args, single_out);
        batch_result.push_back(json::Value{single_out});
      }
      NativePlugin::ReportSuccess(response_obj);
    }
    catch (const BasePlatformException& e) {
      NativePlugin::ReportError(e, response_obj);
    }
  };

  int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);
  auto after_work_func = [=](const shared_json_value& response) {
    wrt::common::NativeContext::GetInstance()->InvokeCallback(
        callback_handle, response->serialize());
  };

  TaskQueue::GetInstance().Queue<json::Value>(
      work_func, after_work_func,
      shared_json_value{new json::Value{json::Object{}}});

  NativePlugin::ReportSuccess(out);
}

void AddressBook_find(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactReadPrivileges);

  // TODO implement contact filter and sorting.
  LoggerD("Entered");
  const json::Object& address_book =
      FromJson<json::Object>(args, "addressBook");

  int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);

  auto get = [address_book](const shared_json_value & response)->void {
    LoggerD("Entered");
    try {
      long addressbook_id =
          common::stol(FromJson<json::String>(address_book, "id"));
      // Read calendar by ID
      int error_code = 0;

      contacts_query_h query = nullptr;
      contacts_filter_h filter = nullptr;
      contacts_list_h list = nullptr;

      error_code = contacts_query_create(_contacts_contact._uri, &query);
      ContactUtil::ErrorChecker(error_code, "Failed contacts_query_create");
      ContactUtil::ContactsQueryHPtr query_ptr(
          &query, ContactUtil::ContactsQueryDeleter);
      error_code = contacts_filter_create(_contacts_contact._uri, &filter);
      ContactUtil::ErrorChecker(error_code, "Failed contacts_filter_create");
      ContactUtil::ContactsFilterPtr filter_ptr(
          filter, ContactUtil::ContactsFilterDeleter);
      error_code =
          contacts_filter_add_int(filter, _contacts_contact.address_book_id,
                                  CONTACTS_MATCH_EQUAL, addressbook_id);
      ContactUtil::ErrorChecker(error_code, "Failed contacts_filter_add_int");
      error_code = contacts_query_set_filter(query, filter);
      ContactUtil::ErrorChecker(error_code, "Failed contacts_query_set_filter");
      error_code = contacts_db_get_records_with_query(query, 0, 0, &list);
      ContactUtil::ErrorChecker(error_code,
                                "Failed contacts_db_get_records_with_query");
      ContactUtil::ContactsListHPtr list_ptr(&list,
                                             ContactUtil::ContactsListDeleter);

      unsigned int record_count = 0;
      error_code = contacts_list_get_count(list, &record_count);
      json::Value result{json::Array{}};
      json::Array& contacts = result.get<json::Array>();
      contacts_list_first(list);
      for (unsigned int i = 0; i < record_count; i++) {
        contacts_record_h record;
        error_code = contacts_list_get_current_record_p(list, &record);
        ContactUtil::ErrorChecker(error_code,
                                  "Failed contacts_list_get_current_record_p");

        int id_value = 0;
        error_code =
            contacts_record_get_int(record, _contacts_contact.id, &id_value);
        ContactUtil::ErrorChecker(error_code, "Failed contacts_record_get_int");

        contacts.push_back(json::Value(static_cast<double>(id_value)));
        contacts_list_next(list);
      }

      NativePlugin::ReportSuccess(result, response->get<json::Object>());
    }
    catch (const BasePlatformException& e) {
      LoggerE("error: %s: %s", e.name().c_str(), e.message().c_str());
      NativePlugin::ReportError(e, response->get<json::Object>());
    }
  };

  auto get_response = [callback_handle](const shared_json_value & response)
      ->void {
    wrt::common::NativeContext::GetInstance()->InvokeCallback(
        callback_handle, response->serialize());
  };

  TaskQueue::GetInstance().Queue<json::Value>(
      get, get_response, shared_json_value(new json::Value(json::Object())));

  NativePlugin::ReportSuccess(out);
}

void AddressBook_addGroup(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactWritePrivileges);
  const json::Object& group = FromJson<json::Object>(args, "group");
  if (!IsNull(group, "id") || !IsNull(group, "addressBookId")) {
    LOGE("Group object is previously added");
    throw common::InvalidValuesException("Group object is previously added");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h contacts_record = nullptr;
  err = contacts_record_create(_contacts_group._uri, &contacts_record);
  ContactUtil::ErrorChecker(err,
                            "Error during executing contacts_record_create()");

  ContactUtil::ContactsRecordHPtr record(&contacts_record,
                                         ContactUtil::ContactsDeleter);

  long addressbook_id =
      common::stol(FromJson<json::String>(args, "addressBookId"));
  addressbook_id = (IsUnified(addressbook_id)) ? 0 : addressbook_id;
  ContactUtil::SetIntInRecord(contacts_record, _contacts_group.address_book_id,
                              addressbook_id);

  ContactUtil::ExportContactGroupToContactsRecord(contacts_record, group);
  int groupId = 0;
  err = contacts_db_insert_record(contacts_record, &groupId);
  ContactUtil::ErrorChecker(err, "Error during insert group record");

  json::Value result{json::Object{}};
  json::Object& result_obj = result.get<json::Object>();
  result_obj.insert(std::make_pair("id", std::to_string(groupId)));
  result_obj.insert(
      std::make_pair("addressBookId", std::to_string(addressbook_id)));
  NativePlugin::ReportSuccess(result, out);
}

void AddressBook_getGroup(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactReadPrivileges);

  long id = common::stol(FromJson<json::String>(args, "id"));

  if (id < 0) {
    throw common::InvalidValuesException("Incorrect group id");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h contacts_record = nullptr;
  err = contacts_db_get_record(_contacts_group._uri, id, &contacts_record);
  if (CONTACTS_ERROR_NONE != err || nullptr == contacts_record) {
    throw common::NotFoundException("Don't find group with this id");
  }

  ContactUtil::ContactsRecordHPtr record(&contacts_record,
                                         ContactUtil::ContactsDeleter);

  long addressbook_id =
      common::stol(FromJson<json::String>(args, "addressBook", "id"));
  if (IsUnified(addressbook_id)) {
    int address_book_id = 0;
    ContactUtil::GetIntFromRecord(
        contacts_record, _contacts_group.address_book_id, &address_book_id);
    if (address_book_id != addressbook_id) {
      throw common::NotFoundException("No group in this address book.");
    }
  }

  json::Value result{json::Object{}};
  ContactUtil::ImportContactGroupFromContactsRecord(
      contacts_record, &result.get<json::Object>());

  NativePlugin::ReportSuccess(result, out);
}

void AddressBook_updateGroup(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactWritePrivileges);

  const json::Object& group = FromJson<json::Object>(args, "group");

  if (IsNull(group, "id") || IsNull(group, "addressBookId")) {
    LOGE("Group object is not added");
    throw common::InvalidValuesException("Group object is not added");
  }

  long addressbook_id =
      common::stol(FromJson<json::String>(args, "addressBookId"));
  long group_addressbook_id =
      common::stol(FromJson<json::String>(group, "addressBookId"));
  if (IsUnified(addressbook_id) && (addressbook_id != group_addressbook_id)) {
    LOGE("Wrong address book");
    throw common::InvalidValuesException("Wrong address book");
  }

  if (FromJson<bool>(group, "readOnly")) {
    LOGW("Group is readonly - cancel update");
    // TODO should this be an error?
    NativePlugin::ReportSuccess(out);
    return;
  }

  long group_id = common::stol(FromJson<json::String>(group, "id"));
  if (group_id < 0) {
    throw common::InvalidValuesException("Incorrect group id");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h contacts_record = nullptr;
  err =
      contacts_db_get_record(_contacts_group._uri, group_id, &contacts_record);
  if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
    LOGE("Error during executing contacts_db_get_record()");
    throw common::NotFoundException(
        "Error during executing contacts_db_get_record()");
  }

  ContactUtil::ErrorChecker(err,
                            "Error during executing contacts_db_get_record()");

  ContactUtil::ContactsRecordHPtr record(&contacts_record,
                                         ContactUtil::ContactsDeleter);
  ContactUtil::ExportContactGroupToContactsRecord(contacts_record, group);

  err = contacts_db_update_record(contacts_record);
  if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
    LOGE("Problem during db_update_record");
    throw common::NotFoundException("Problem during db_update_record");
  }
  ContactUtil::ErrorChecker(err, "Problem during db_update_record");

  NativePlugin::ReportSuccess(out);
}

void AddressBook_removeGroup(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactWritePrivileges);

  long id = common::stol(FromJson<json::String>(args, "id"));
  if (id < 0) {
    throw common::InvalidValuesException("Incorrect group id");
  }

  int err = CONTACTS_ERROR_NONE;
  long addressbook_id = AddressBookId(args);
  if (!IsUnified(addressbook_id)) {
    contacts_record_h contacts_record = nullptr;
    err = contacts_db_get_record(_contacts_group._uri, id, &contacts_record);
    if (CONTACTS_ERROR_NONE != err || contacts_record == nullptr) {
      LOGE("No group");
      throw common::UnknownException("No group");
    }

    int group_addressbook_id = 0;
    ContactUtil::GetIntFromRecord(contacts_record,
                                  _contacts_group.address_book_id,
                                  &group_addressbook_id);

    if (group_addressbook_id != addressbook_id) {
      throw common::UnknownException(
          "Contact is not a member of this address book");
    }
  }

  err = contacts_db_delete_record(_contacts_group._uri, id);
  if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
    LOGE("Problem during db_update_record");
    throw common::NotFoundException("Problem during db_delete_record");
  }
  ContactUtil::ErrorChecker(err, "Problem during db_delete_record");
  NativePlugin::ReportSuccess(out);
}

void AddressBook_getGroups(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactReadPrivileges);

  int err = CONTACTS_ERROR_NONE;
  contacts_list_h groups_list = nullptr;
  long addressbook_id = AddressBookId(args);

  if (IsUnified(addressbook_id)) {
    err = contacts_db_get_all_records(_contacts_group._uri, 0, 0, &groups_list);
    ContactUtil::ErrorChecker(err, "Fail to get group list");
  } else {
    contacts_query_h query = nullptr;
    contacts_filter_h filter = nullptr;

    err = contacts_query_create(_contacts_group._uri, &query);
    ContactUtil::ErrorChecker(err, "Fail to get contacts_query_create ");

    err = contacts_filter_create(_contacts_group._uri, &filter);
    ContactUtil::ErrorChecker(err, "Fail to get contacts_filter_create ");

    err = contacts_filter_add_int(filter, _contacts_group.address_book_id,
                                  CONTACTS_MATCH_EQUAL, addressbook_id);
    ContactUtil::ErrorChecker(err, "Fail to get contacts_filter_add_int ");

    err = contacts_query_set_filter(query, filter);
    ContactUtil::ErrorChecker(err, "Fail to get contacts_query_set_filter ");

    err = contacts_db_get_records_with_query(query, 0, 0, &groups_list);
    ContactUtil::ErrorChecker(
        err, "Fail to get contacts_db_get_records_with_query ");

    err = contacts_filter_destroy(filter);
    ContactUtil::ErrorChecker(err, "Fail to get contacts_filter_destroy ");

    err = contacts_query_destroy(query);
    ContactUtil::ErrorChecker(err, "Fail to get contacts_query_destroy ");
  }
  unsigned int record_count = 0;
  err = contacts_list_get_count(groups_list, &record_count);
  ContactUtil::ErrorChecker(err, "Fail to get contacts_list_get_count ");

  contacts_list_first(groups_list);

  json::Value result{json::Array{}};
  json::Array& array = result.get<json::Array>();

  for (unsigned int i = 0; i < record_count; i++) {
    contacts_record_h contacts_record;
    err = contacts_list_get_current_record_p(groups_list, &contacts_record);
    if (CONTACTS_ERROR_NONE != err || nullptr == contacts_record) {
      LOGE("Fail to get group record ");
      throw common::UnknownException("Fail to get group record");
    }

    JsonValue group{json::Object{}};
    ContactUtil::ImportContactGroupFromContactsRecord(contacts_record,
                                                      &group.get<JsonObject>());
    array.push_back(group);

    err = contacts_list_next(groups_list);
    if (CONTACTS_ERROR_NONE != err) {
      LoggerE("Fail to get next address book ");
      break;
    }
  }

  NativePlugin::ReportSuccess(result, out);
}

namespace {
void AddressBook_listenerCallback(const char* view_uri, void* user_data) {
  (void)view_uri;
  (void)user_data;

  int* last_database_version = static_cast<int*>(user_data);

  contacts_list_h contacts_list = nullptr;

  int error_code = contacts_db_get_changes_by_version(
      _contacts_contact_updated_info._uri, kUnifiedAddressBookId,
      *last_database_version, &contacts_list, last_database_version);

  ContactUtil::ContactsListHPtr contacts_list_ptr(
      &contacts_list, ContactUtil::ContactsListDeleter);

  if (CONTACTS_ERROR_NONE != error_code) {
    LOGE("cannot get changes by version, code: %d", error_code);
  } else {
    unsigned int count = 0;

    error_code = contacts_list_get_count(contacts_list, &count);
    if (CONTACTS_ERROR_NONE != error_code) {
      LOGW("Cannot get updated contact count, code: %d", error_code);
      return;
    }

    if (!count) {
      LOGW("No updated contacts");
      return;
    }

    contacts_list_first(contacts_list);

    json::Value result{json::Object{}};
    json::Object& result_obj = result.get<json::Object>();
    json::Array& added =
        result_obj.insert(std::make_pair("added", json::Array{}))
            .first->second.get<json::Array>();
    json::Array& updated =
        result_obj.insert(std::make_pair("updated", json::Array{}))
            .first->second.get<json::Array>();
    json::Array& removed =
        result_obj.insert(std::make_pair("removed", json::Array{}))
            .first->second.get<json::Array>();

    for (unsigned int i = 0; i < count; i++) {
      contacts_record_h contact_updated_record = nullptr;

      error_code = contacts_list_get_current_record_p(contacts_list,
                                                      &contact_updated_record);
      if (CONTACTS_ERROR_NONE != error_code) {
        LOGW("fail to get contact from list, code: %d", error_code);
        return;
      }

      int changed_id = 0;
      int changed_ab_id = 0;
      int changed_type = 0;

      try {
        ContactUtil::GetIntFromRecord(contact_updated_record,
                                      _contacts_contact_updated_info.contact_id,
                                      &changed_id);

        ContactUtil::GetIntFromRecord(
            contact_updated_record,
            _contacts_contact_updated_info.address_book_id, &changed_ab_id);

        ContactUtil::GetIntFromRecord(contact_updated_record,
                                      _contacts_contact_updated_info.type,
                                      &changed_type);
      }
      catch (const BasePlatformException&) {
        LOGE("failt to get int from record");
        return;
      }

      if (CONTACTS_CHANGE_INSERTED == changed_type ||
          CONTACTS_CHANGE_UPDATED == changed_type) {
        contacts_record_h contacts_record = nullptr;

        error_code = contacts_db_get_record(_contacts_contact._uri, changed_id,
                                            &contacts_record);

        if (CONTACTS_ERROR_NONE != error_code) {
          LOGW("fail to get contact from record");
          return;
        }

        ContactUtil::ContactsRecordHPtr contact_record_ptr(
            &contacts_record, ContactUtil::ContactsDeleter);

        json::Value contact{json::Object{}};
        ContactUtil::ImportContactFromContactsRecord(
            *contact_record_ptr, &contact.get<json::Object>());

        if (CONTACTS_CHANGE_INSERTED == changed_type) {
          added.push_back(std::move(contact));
        } else {
          updated.push_back(std::move(contact));
        }
      } else if (CONTACTS_CHANGE_DELETED == changed_type) {
        // Need to send the addressbook id with the removed id
        json::Value removed_data{json::Object{}};
        json::Object& removed_data_obj = removed_data.get<json::Object>();

        removed_data_obj.insert(
            std::make_pair("id", std::to_string(changed_id)));
        removed_data_obj.insert(
            std::make_pair("addressBookId", std::to_string(changed_ab_id)));
        removed.push_back(std::move(removed_data));
      }
    }
    NativeContext::GetInstance()->FireEvent(kContactListenerId,
                                            result.serialize());
  }
}
}

void AddressBook_startListening(int* current_state, const JsonObject&,
                                JsonObject& out) {
  // Set the initial latest version before registering the callback.
  // The callback should only be registered once so no race can occur.
  int error_code = contacts_db_get_current_version(current_state);
  if (CONTACTS_ERROR_NONE != error_code) {
    LOGW("get current version returns error, code: %d", error_code);
  }

  error_code = contacts_db_add_changed_cb(
      _contacts_contact._uri, AddressBook_listenerCallback, current_state);

  if (CONTACTS_ERROR_NONE != error_code) {
    LOGE("Error while registering listener to contacts db, code: %d",
         error_code);
    throw UnknownException("Error while registering listener to contacts db");
  }

  NativePlugin::ReportSuccess(out);
}

void AddressBook_stopListening(int* current_state, const JsonObject&,
                               JsonObject& out) {
  int error_code = contacts_db_remove_changed_cb(
      _contacts_contact._uri, AddressBook_listenerCallback, current_state);

  if (CONTACTS_ERROR_NONE != error_code) {
    LOGE("Error while removing listener");
    throw UnknownException("Error while removing listener");
  }

  NativePlugin::ReportSuccess(out);
}

}  // AddressBook
}  // contact
}  // extension
