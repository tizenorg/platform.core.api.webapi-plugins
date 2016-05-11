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

#include "common/converter.h"
#include "common/extension.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/scope_exit.h"

#include <contacts.h>
#include "contact/contact_instance.h"
#include "contact/contact_search_engine.h"

namespace extension {
namespace contact {
namespace AddressBook {

using namespace common;

typedef std::shared_ptr<JsonValue> shared_json_value;

namespace {
static const long kUnifiedAddressBookId = -1;
const char* kContactListenerId = "ContactChangeListener";

inline long AddressBookId(const JsonObject& obj) {
  return common::stol(FromJson<JsonString>(obj, "addressBook", "id"));
}

inline bool IsUnified(const JsonObject& args) {
  return AddressBookId(args) == kUnifiedAddressBookId;
}

inline bool IsUnified(long id) { return id == kUnifiedAddressBookId; }

}  // anonymous namespace

PlatformResult AddressBookGet(const JsonObject& args, JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  int contact_id = common::stol(FromJson<JsonString>(args, "id"));

  contacts_record_h contacts_record = nullptr;
  int err = contacts_db_get_record(_contacts_contact._uri, contact_id,
                                   &contacts_record);
  if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR,
                          "Contacts record get error",
                          ("Contacts record get error, error code: %d", err));
  }
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  out["id"] = picojson::value(std::to_string(contact_id));
  status =
      ContactUtil::ImportContactFromContactsRecord(*contacts_record_ptr, &out);
  if (status.IsError()) return status;

  status = ContactUtil::UpdateAdditionalInformation(*contacts_record_ptr, &out);
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookAdd(const JsonObject& args, JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const JsonObject& contact = FromJson<JsonObject>(args, "contact");
  long addressBookId = common::stol(FromJson<JsonString>(args, "addressBookId"));

  if (!IsNull(contact, "id")) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Contact already exists");
  }

  contacts_record_h contacts_record = nullptr;
  int err = 0;
  err = contacts_record_create(_contacts_contact._uri, &contacts_record);
  if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Contacts record create error",
                          ("Contacts record create error, error code: %d", err));
  }

  // contacts_record starts to be protected by unique_ptr
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);
  status =
      ContactUtil::ExportContactToContactsRecord(*contacts_record_ptr, contact);
  if (status.IsError()) return status;

  status = ContactUtil::SetIntInRecord(
      contacts_record, _contacts_contact.address_book_id, addressBookId);
  if (status.IsError()) return status;

  int id = -1;
  err = contacts_db_insert_record(*contacts_record_ptr, &id);
  if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Contacts db insert error",
                              ("Contacts db insert error, error code: %d", err));
  }

  contacts_record_h reset_record;
  err = contacts_db_get_record(_contacts_contact._uri, id, &reset_record);
  if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Contacts record get error",
                              ("Contacts record get error, error code: %d", err));
  }
  if (nullptr != reset_record) {
    LoggerE("reset");
    contacts_record_ptr.reset(&reset_record);
  }

  out.insert(std::make_pair("id", JsonValue{std::to_string(id)}));

  status = ContactUtil::UpdateAdditionalInformation(*contacts_record_ptr, &out);
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookUpdate(const JsonObject& args, JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const JsonObject& contact = FromJson<JsonObject>(args, "contact");
  long contactId = common::stol(FromJson<JsonString>(contact, "id"));

  if (IsNull(contact, "id")) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Contact doesn't exist");
  }

  contacts_record_h to_update = nullptr;
  int err = contacts_db_get_record(_contacts_contact._uri, contactId, &to_update);
  if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR,
                          "Problem with getting contact",
                          ("Problem with getting contact. Error: %d", err));
  }
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &to_update, ContactUtil::ContactsDeleter);
  status =
      ContactUtil::ExportContactToContactsRecord(*contacts_record_ptr, contact);
  if (status.IsError()) return status;

  err = contacts_db_update_record(*contacts_record_ptr);
  if (CONTACTS_ERROR_NONE != err) {
    if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
      return LogAndCreateResult(
          ErrorCode::NOT_FOUND_ERR,
          "Error during executing contacts_db_update_record().",
          ("Error during executing contacts_db_update_record(). Error: %d", err));
    }
    if (CONTACTS_ERROR_DB == err) {
      return LogAndCreateResult(
          ErrorCode::UNKNOWN_ERR,
          "Error during executing contacts_db_update_record().",
          ("Error during executing contacts_db_update_record(). Error: %d", err));
    }
  }

  status = ContactUtil::UpdateAdditionalInformation(*contacts_record_ptr, &out);
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookRemove(const JsonObject& args, JsonObject&) {
  LoggerE("entered");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  int contact_id = common::stol(FromJson<JsonString>(args, "id"));

  if (contact_id < 0) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Nagative contact id");
  }

  int err = contacts_db_delete_record(_contacts_contact._uri, contact_id);
  if (CONTACTS_ERROR_NO_DATA == err) {
    return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR,
                          "Remove failed: contact not found");
  } else if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Contacts record delete error");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookAddBatch(const JsonObject& args, JsonArray& out) {
  LoggerD("Enter");

  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const JsonArray& batch_args = FromJson<JsonArray>(args, "batchArgs");
  long addressBookId =
      common::stol(FromJson<JsonString>(args, "addressBookId"));
  addressBookId = addressBookId == -1 ? 0 : addressBookId;

  int length = static_cast<int>(batch_args.size());
  contacts_list_h contacts_list = NULL;
  int error_code = contacts_list_create(&contacts_list);
  if (CONTACTS_ERROR_NONE != error_code) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "list creation failed",
                          ("list creation failed, code: %d", error_code));
  }
  ContactUtil::ContactsListHPtr contacts_list_ptr(
      &contacts_list, ContactUtil::ContactsListDeleter);

  for (auto& item : batch_args) {
    contacts_record_h contacts_record = nullptr;
    int err = 0;
    err = contacts_record_create(_contacts_contact._uri, &contacts_record);
    if (CONTACTS_ERROR_NONE != err) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Contacts record create error",
                            ("Contacts record create error, error code: %d", err));
    }
    ContactUtil::ContactsRecordHPtr x(&contacts_record,
                                      ContactUtil::ContactsDeleter);
    status = ContactUtil::ExportContactToContactsRecord(
        contacts_record, JsonCast<JsonObject>(item));
    if (status.IsError()) return status;

    status = ContactUtil::SetIntInRecord(
        contacts_record, _contacts_contact.address_book_id, addressBookId);
    if (status.IsError()) return status;

    error_code = contacts_list_add(*contacts_list_ptr, *(x.release()));
    if (CONTACTS_ERROR_NONE != error_code) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "error during add record to list",
                            ("error during add record to list, code: %d", error_code));
    }
  }

  int* ids = nullptr;
  int count = 0;

  SCOPE_EXIT {
    free(ids);
  };

  error_code = contacts_db_insert_records(*contacts_list_ptr, &ids, &count);
  if (CONTACTS_ERROR_NONE != error_code || nullptr == ids) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "inserting contacts to db fails",
                          ("inserting contacts to db fails, code: %d", error_code));
  }

  if (length != count) {
    LoggerW("Added different number of contacts");
  }

  for (int i = 0; i < count; i++) {
    JsonObject out_object;
    contacts_record_h contact_record = nullptr;
    error_code =
        contacts_db_get_record(_contacts_contact._uri, ids[i], &contact_record);
    if (CONTACTS_ERROR_NONE != error_code) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Contacts record get error",
                            ("Contacts record get error, error code: %d", error_code));
    }
    status = ContactUtil::ImportContactFromContactsRecord(
        contact_record, &out_object);
    if (status.IsError()) return status;

    out.push_back(JsonValue{out_object});
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookUpdateBatch(const JsonObject& args, JsonArray& out) {
  LoggerD("Enter");

  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const JsonArray& batch_args = FromJson<JsonArray>(args, "batchArgs");
  contacts_list_h contacts_list = NULL;
  int err = 0;
  err = contacts_list_create(&contacts_list);
  if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "list creation failed",
                          ("list creation failed, code: %d", err));
  }
  ContactUtil::ContactsListHPtr contacts_list_ptr(
      &contacts_list, ContactUtil::ContactsListDeleter);
  for (auto& item : batch_args) {
    const JsonObject& contact = JsonCast<JsonObject>(item);
    long contactId = common::stol(FromJson<JsonString>(contact, "id"));
    if (IsNull(contact, "id")) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Contact doesn't exist",
                                ("Contact doesn't exist"));
    }
    contacts_record_h to_update = nullptr;
    err = contacts_db_get_record(_contacts_contact._uri, contactId, &to_update);
    if (CONTACTS_ERROR_NONE != err) {
      return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR,
                            "Problem with getting contact",
                            ("Problem with getting contact. Error: %d", err));
    }
    ContactUtil::ContactsRecordHPtr x(&to_update,
                                      ContactUtil::ContactsDeleter);
    status = ContactUtil::ExportContactToContactsRecord(
        to_update, JsonCast<JsonObject>(item));
    if (status.IsError()) return status;
    JsonObject out_object;
    status = ContactUtil::UpdateAdditionalInformation(*x, &out_object);
    if (status.IsError()) return status;
    out.push_back(JsonValue{out_object});

    err = contacts_list_add(*contacts_list_ptr, *(x.release()));
    if (CONTACTS_ERROR_NONE != err) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "error during add record to list",
                            ("error during add record to list, code: %d", err));
    }
  }
  err = contacts_db_update_records(*contacts_list_ptr);
  if (CONTACTS_ERROR_NONE != err) {
    if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
      return LogAndCreateResult(
          ErrorCode::NOT_FOUND_ERR,
          "Error during executing contacts_db_update_record().",
          ("Error during executing contacts_db_update_record(). Error: %d",
                        err));
    }
    if (CONTACTS_ERROR_DB == err) {
      return LogAndCreateResult(
          ErrorCode::UNKNOWN_ERR,
          "Error during executing contacts_db_update_record().",
          ("Error during executing contacts_db_update_record(). Error: %d",
                        err));
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookRemoveBatch(const JsonObject& args) {
  LoggerD("Enter");

  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;
  const JsonArray& batch_args = FromJson<JsonArray>(args, "batchArgs");
  int length = static_cast<int>(batch_args.size());
  int ids[length], i=0;
  for (auto& item : batch_args) {
    long contact_id = common::stol(item.get<std::string>());
    if (contact_id < 0) {
      return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Nagative contact id");
    }
    ids[i] = contact_id;
    i++;
  }
  int err = contacts_db_delete_records(_contacts_contact._uri, ids, length);
  if (CONTACTS_ERROR_NO_DATA == err) {
    return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR,
                          "Remove failed: contact not found");
  } else if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Contacts record delete error");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookFind(const JsonObject& args, JsonArray& array) {
  LoggerD("Entered");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (!status) return status;

  long address_book_id = common::stol(FromJson<std::string>(args, "addressBookId"));

  LoggerD("Searching in address book: %d", address_book_id);

  ContactSearchEngine engine;
  if (!IsUnified(address_book_id)) {
    engine.SetAddressBookId(address_book_id);
  }

  const auto filter_it = args.find("filter");
  if (args.end() != filter_it && filter_it->second.is<picojson::object>()) {
    status = engine.ApplyFilter(filter_it->second);
    if (!status) return status;
  }

  const auto sort_mode_it = args.find("sortMode");
  if (args.end() != sort_mode_it && sort_mode_it->second.is<picojson::object>()) {
    status = engine.SetSortMode(sort_mode_it->second);
    if (!status) return status;
  }

  return engine.Find(&array);
}

PlatformResult AddressBookAddGroup(const JsonObject& args, JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const JsonObject& group = FromJson<JsonObject>(args, "group");
  if (!IsNull(group, "id") || !IsNull(group, "addressBookId")) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR,
                          "Group object is previously added");
  }

  contacts_record_h contacts_record = nullptr;
  int err = contacts_record_create(_contacts_group._uri, &contacts_record);
  status = ContactUtil::ErrorChecker(
      err, "Error during executing contacts_record_create()");
  if (status.IsError()) return status;

  ContactUtil::ContactsRecordHPtr record(&contacts_record,
                                         ContactUtil::ContactsDeleter);

  long addressbook_id =
      common::stol(FromJson<JsonString>(args, "addressBookId"));
  addressbook_id = (IsUnified(addressbook_id)) ? 0 : addressbook_id;
  status = ContactUtil::SetIntInRecord(
      contacts_record, _contacts_group.address_book_id, addressbook_id);
  if (status.IsError()) return status;

  status =
      ContactUtil::ExportContactGroupToContactsRecord(contacts_record, group);
  if (status.IsError()) return status;

  int groupId = 0;
  err = contacts_db_insert_record(contacts_record, &groupId);
  status = ContactUtil::ErrorChecker(err, "Error during insert group record");
  if (status.IsError()) return status;

  out["id"] = picojson::value(std::to_string(groupId));
  out["addressBookId"] = picojson::value(std::to_string(addressbook_id));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookGetGroup(const JsonObject& args, JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long id = common::stol(FromJson<JsonString>(args, "id"));

  if (id < 0) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Incorrect group id");
  }

  contacts_record_h contacts_record = nullptr;
  int err = contacts_db_get_record(_contacts_group._uri, id, &contacts_record);
  if (CONTACTS_ERROR_NONE != err || nullptr == contacts_record) {
    return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR, "Group not exist");
  }

  ContactUtil::ContactsRecordHPtr record(&contacts_record,
                                         ContactUtil::ContactsDeleter);
  long addressbook_id = common::stol(FromJson<std::string>(args, "addressBookId"));
  if (IsUnified(addressbook_id)) {
    int address_book_id = 0;
    status = ContactUtil::GetIntFromRecord(
        contacts_record, _contacts_group.address_book_id, &address_book_id);
    if (status.IsError()) return status;

    if (address_book_id != addressbook_id) {
      return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR,
                            "No group in this address book.");
    }
  }

  status =
      ContactUtil::ImportContactGroupFromContactsRecord(contacts_record, &out);
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookUpdateGroup(const JsonObject& args, JsonObject&) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const JsonObject& group = FromJson<JsonObject>(args, "group");

  if (IsNull(group, "id") || IsNull(group, "addressBookId")) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR,
                          "Group object is not added");
  }

  long addressbook_id =
      common::stol(FromJson<JsonString>(args, "addressBookId"));
  long group_addressbook_id =
      common::stol(FromJson<JsonString>(group, "addressBookId"));
  if (IsUnified(addressbook_id) && (addressbook_id != group_addressbook_id)) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Wrong address book");
  }

  if (FromJson<bool>(group, "readOnly")) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Group is readonly - cancel update");
  }

  long group_id = common::stol(FromJson<JsonString>(group, "id"));
  if (group_id < 0) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Incorrect group id");
  }

  contacts_record_h contacts_record = nullptr;
  int err =
      contacts_db_get_record(_contacts_group._uri, group_id, &contacts_record);
  if (CONTACTS_ERROR_NONE != err || nullptr == contacts_record) {
    return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR, "Group not exist");
  }

  status = ContactUtil::ErrorChecker(
      err, "Error during executing contacts_db_get_record()");
  if (status.IsError()) return status;

  ContactUtil::ContactsRecordHPtr record(&contacts_record,
                                         ContactUtil::ContactsDeleter);
  status =
      ContactUtil::ExportContactGroupToContactsRecord(contacts_record, group);
  if (status.IsError()) return status;

  err = contacts_db_update_record(contacts_record);
  if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
    return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR,
                          "Problem during db_update_record");
  }
  status = ContactUtil::ErrorChecker(err, "Problem during db_update_record");
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookRemoveGroup(const JsonObject& args, JsonObject&) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long id = common::stol(FromJson<JsonString>(args, "id"));
  if (id < 0) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Incorrect group id");
  }

  int err;
  long addressbook_id = common::stol(FromJson<std::string>(args, "addressBookId"));
  if (!IsUnified(addressbook_id)) {
    contacts_record_h contacts_record = nullptr;
    err = contacts_db_get_record(_contacts_group._uri, id, &contacts_record);
    if (CONTACTS_ERROR_NONE != err || contacts_record == nullptr) {
      return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR, "Group not exist");
    }

    int group_addressbook_id = 0;
    status = ContactUtil::GetIntFromRecord(contacts_record,
                                           _contacts_group.address_book_id,
                                           &group_addressbook_id);
    if (status.IsError()) return status;

    if (group_addressbook_id != addressbook_id) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                            "Contact is not a member of this address book");
    }
  }

  err = contacts_db_delete_record(_contacts_group._uri, id);
  if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Problem during db_delete_record");
  }
  status = ContactUtil::ErrorChecker(err, "Problem during db_delete_record");
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookGetGroups(const JsonObject& args, JsonArray& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  int err = CONTACTS_ERROR_NONE;
  contacts_list_h groups_list = nullptr;
  long addressbook_id = AddressBookId(args);

  if (IsUnified(addressbook_id)) {
    err = contacts_db_get_all_records(_contacts_group._uri, 0, 0, &groups_list);
    status = ContactUtil::ErrorChecker(err, "Fail to get group list");
    if (status.IsError()) return status;
  } else {
    contacts_query_h query = nullptr;
    contacts_filter_h filter = nullptr;

    err = contacts_query_create(_contacts_group._uri, &query);
    status =
        ContactUtil::ErrorChecker(err, "Fail to get contacts_query_create ");
    if (status.IsError()) return status;

    err = contacts_filter_create(_contacts_group._uri, &filter);
    status =
        ContactUtil::ErrorChecker(err, "Fail to get contacts_filter_create ");
    if (status.IsError()) return status;

    err = contacts_filter_add_int(filter, _contacts_group.address_book_id,
                                  CONTACTS_MATCH_EQUAL, addressbook_id);
    status =
        ContactUtil::ErrorChecker(err, "Fail to get contacts_filter_add_int ");
    if (status.IsError()) return status;

    err = contacts_query_set_filter(query, filter);
    status = ContactUtil::ErrorChecker(
        err, "Fail to get contacts_query_set_filter ");
    if (status.IsError()) return status;

    err = contacts_db_get_records_with_query(query, 0, 0, &groups_list);
    status = ContactUtil::ErrorChecker(
        err, "Fail to get contacts_db_get_records_with_query ");
    if (status.IsError()) return status;

    // deleter to release the memory in case of an error
    ContactUtil::ContactsListHPtr group_list_ptr(
        &groups_list, ContactUtil::ContactsListDeleter);

    err = contacts_filter_destroy(filter);
    status =
        ContactUtil::ErrorChecker(err, "Fail to get contacts_filter_destroy ");
    if (status.IsError()) return status;

    err = contacts_query_destroy(query);
    status =
        ContactUtil::ErrorChecker(err, "Fail to get contacts_query_destroy ");
    if (status.IsError()) return status;

    // release the ownership, pass it back to the outer scope
    group_list_ptr.release();
  }

  // groups_list has been initialized, take the ownership
  ContactUtil::ContactsListHPtr group_list_ptr(
      &groups_list, ContactUtil::ContactsListDeleter);

  int record_count = 0;
  err = contacts_list_get_count(groups_list, &record_count);
  status =
      ContactUtil::ErrorChecker(err, "Fail to get contacts_list_get_count ");
  if (status.IsError()) return status;

  contacts_list_first(groups_list);

  for (int i = 0; i < record_count; i++) {
    contacts_record_h contacts_record;
    err = contacts_list_get_current_record_p(groups_list, &contacts_record);
    if (CONTACTS_ERROR_NONE != err || nullptr == contacts_record) {
      return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Fail to get group record");
    }

    JsonValue group{JsonObject{}};
    status = ContactUtil::ImportContactGroupFromContactsRecord(
        contacts_record, &group.get<JsonObject>());
    if (status.IsError()) return status;

    out.push_back(group);

    if (i < record_count - 1) {
      err = contacts_list_next(groups_list);
      if (CONTACTS_ERROR_NONE != err) {
        LoggerE("Fail to get next address book, error %d", err);
        break;
      }
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

namespace {

void AddressBookListenerCallback(const char* view_uri, void* user_data) {
  LoggerD("entered");
  (void)view_uri;

  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) {
    LoggerE("Fail to check db connection: %s", status.message().c_str());
    return;
  }

  contacts_list_h contacts_list = nullptr;

  int error_code;
  int current_version = 0;
  error_code = contacts_db_get_current_version(&current_version);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerW("get current version returns error, code: %d", error_code);
  }

  ContactInstance* instance = static_cast<ContactInstance*>(user_data);
  if (!instance) {
    LoggerE("instance is NULL");
    return;
  }

  for (int version = instance->current_state(); version < current_version; ++version) {
    int latest_version = 0;
    error_code = contacts_db_get_changes_by_version(
        _contacts_contact_updated_info._uri, kUnifiedAddressBookId, version,
        &contacts_list, &latest_version);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerE("cannot get changes by version, code: %d", error_code);
      continue;
    }

    ContactUtil::ContactsListHPtr contacts_list_ptr(
        &contacts_list, ContactUtil::ContactsListDeleter);

    int count = 0;
    error_code = contacts_list_get_count(contacts_list, &count);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("Cannot get updated contact count, code: %d", error_code);
      continue;
    }

    if (!count) {
      LoggerW("No updated contacts");
      continue;
    }

    contacts_list_first(contacts_list);

    JsonValue result{JsonObject{}};
    JsonObject& result_obj = result.get<JsonObject>();
    result_obj.insert(
        std::make_pair(std::string("listenerId"),
            picojson::value(std::string(kContactListenerId))));
    JsonArray& added =
        result_obj.insert(std::make_pair(std::string("added"),
            picojson::value(JsonArray{}))).first->second.get<JsonArray>();
    JsonArray& updated =
        result_obj.insert(std::make_pair(std::string("updated"),
            picojson::value(JsonArray{}))).first->second.get<JsonArray>();
    JsonArray& removed =
        result_obj.insert(std::make_pair(std::string("removed"),
            picojson::value(JsonArray{}))).first->second.get<JsonArray>();

    for (int i = 0; i < count; i++) {
      contacts_record_h contact_updated_record = nullptr;

      error_code = contacts_list_get_current_record_p(contacts_list,
                                                      &contact_updated_record);
      if (CONTACTS_ERROR_NONE != error_code) {
        LoggerW("fail to get contact from list, code: %d", error_code);
        break;
      }

      int changed_id = 0;
      int changed_ab_id = 0;
      int changed_type = 0;

      PlatformResult status = ContactUtil::GetIntFromRecord(
          contact_updated_record, _contacts_contact_updated_info.contact_id,
          &changed_id);
      if (status.IsError()) {
        LoggerE("Fail to get int from record: %s", status.message().c_str());
        break;
      }

      status = ContactUtil::GetIntFromRecord(
          contact_updated_record,
          _contacts_contact_updated_info.address_book_id, &changed_ab_id);
      if (status.IsError()) {
        LoggerE("Fail to get int from record: %s", status.message().c_str());
        break;
      }

      status = ContactUtil::GetIntFromRecord(
          contact_updated_record, _contacts_contact_updated_info.type,
          &changed_type);
      if (status.IsError()) {
        LoggerE("Fail to get int from record: %s", status.message().c_str());
        break;
      }

      if (CONTACTS_CHANGE_INSERTED == changed_type ||
          CONTACTS_CHANGE_UPDATED == changed_type) {
        contacts_record_h contacts_record = nullptr;

        error_code = contacts_db_get_record(_contacts_contact._uri, changed_id,
                                            &contacts_record);

        if (CONTACTS_ERROR_NONE != error_code) {
          LoggerW("fail to get contact from record");
          break;
        }

        ContactUtil::ContactsRecordHPtr contact_record_ptr(
            &contacts_record, ContactUtil::ContactsDeleter);

        JsonValue contact{JsonObject{}};
        status = ContactUtil::ImportContactFromContactsRecord(
            contacts_record, &contact.get<JsonObject>());
        if (status.IsError()) {
          LoggerE("Fail to get contact from record: %s",
                  status.message().c_str());
          break;
        }

        if (CONTACTS_CHANGE_INSERTED == changed_type) {
          added.push_back(std::move(contact));
        } else {
          updated.push_back(std::move(contact));
        }
      } else if (CONTACTS_CHANGE_DELETED == changed_type) {
        // Need to send the addressbook id with the removed id
        JsonValue removed_data{JsonObject{}};
        JsonObject& removed_data_obj = removed_data.get<JsonObject>();

        removed_data_obj.insert(
            std::make_pair(std::string("id"),
                picojson::value(std::to_string(changed_id))));
        removed_data_obj.insert(
            std::make_pair(std::string("addressBookId"),
                picojson::value(std::to_string(changed_ab_id))));
        removed.push_back(std::move(removed_data));
      }
    }

    Instance::PostMessage(instance, result.serialize().c_str());
  }

  instance->set_current_state(current_version);
}
}

PlatformResult AddressBookStartListening(ContactInstance& instance, const JsonObject&, JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;
  int current_state = 0;

  // Set the initial latest version before registering the callback.
  // The callback should only be registered once so no race can occur.
  int error_code = contacts_db_get_current_version(&current_state);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerW("get current version returns error, code: %d", error_code);
  }

  instance.set_current_state(current_state);

  error_code = contacts_db_add_changed_cb(_contacts_contact._uri,
                                          AddressBookListenerCallback, &instance);

  if (CONTACTS_ERROR_NONE != error_code) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Error while registering listener to contacts db",
                          ("Error while registering listener to contacts db, code: %d",
                                      error_code));
  }

  instance.set_is_listening(true);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult AddressBookStopListening(ContactInstance& instance) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  int error_code = contacts_db_remove_changed_cb(
      _contacts_contact._uri, AddressBookListenerCallback, &instance);

  if (CONTACTS_ERROR_NONE != error_code) {
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR,
                          "Error while removing listener");
  }

  instance.set_is_listening(false);

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // AddressBook
}  // contact
}  // extension
