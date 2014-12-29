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
#include "common/platform_exception.h"
#include "common/logger.h"

#include <contacts.h>
#include "contact/contact_instance.h"

namespace extension {
namespace contact {
namespace AddressBook {

using namespace common;
//using namespace wrt::common;

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

void AddressBook_get(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();

  int contact_id = common::stol(FromJson<JsonString>(args, "id"));

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h contacts_record = nullptr;
  err = contacts_db_get_record(_contacts_contact._uri, contact_id,
                               &contacts_record);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerW("Contacts record get error, error code: %d", err);
    throw common::NotFoundException("Contacts record get error");
  }
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  out.insert(std::make_pair("id", std::to_string(contact_id)));
  ContactUtil::ImportContactFromContactsRecord(*contacts_record_ptr,
                                               &out);
  ContactUtil::UpdateAdditionalInformation(contacts_record_ptr, &out);
}

void AddressBook_add(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();

  const JsonObject& contact = FromJson<JsonObject>(args, "contact");

  if (!IsNull(contact, "id")) {
    LoggerW("Contact already exists");
    throw common::UnknownException("Contact already exists");
  }

  contacts_record_h contacts_record = nullptr;
  int err = 0;
  err = contacts_record_create(_contacts_contact._uri, &contacts_record);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerW("Contacts record create error, error code: %d", err);
    throw common::UnknownException("Contacts record create error");
  }

  // contacts_record starts to be protected by unique_ptr
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);
  ContactUtil::ExportContactToContactsRecord(*contacts_record_ptr, contact);

  int id = -1;
  err = contacts_db_insert_record(*contacts_record_ptr, &id);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerW("Contacts db insert error, error code: %d", err);
    throw common::UnknownException("Contacts db insert error");
  }

  contacts_record_h reset_record;
  err = contacts_db_get_record(_contacts_contact._uri, id, &reset_record);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerW("Contacts record get error, error code: %d", err);
    throw common::UnknownException("Contacts record get error");
  }
  if (nullptr != reset_record) {
    LoggerE("reset");
    contacts_record_ptr.reset(&reset_record);
  }

  out.insert(std::make_pair("id", JsonValue{std::to_string(id)}));

  ContactUtil::UpdateAdditionalInformation(contacts_record_ptr, &out);
}

void AddressBook_update(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();

  const JsonObject& contact = FromJson<JsonObject>(args, "contact");
  const JsonObject& addressbook = FromJson<JsonObject>(args, "addressBook");
  long contactId = common::stol(FromJson<JsonString>(contact, "id"));

  if (IsNull(contact, "id")) {
    LoggerW("Contact doesn't exist");
    throw common::UnknownException("Contact doesn't exist");
  }

  if (IsNull(addressbook, "id")) {
    LoggerE("Contact is not saved in database");
    throw common::InvalidValuesException("Contact is not saved in database");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h to_update = nullptr;
  err = contacts_db_get_record(_contacts_contact._uri, contactId, &to_update);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerW("Problem with getting contact. Error: %d", err);
    throw common::NotFoundException("Problem with getting contact");
  }
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &to_update, ContactUtil::ContactsDeleter);
  ContactUtil::ExportContactToContactsRecord(*contacts_record_ptr, contact);
  err = contacts_db_update_record(*contacts_record_ptr);
  if (CONTACTS_ERROR_NONE != err) {
    if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
      LoggerE("Error during executing contacts_db_update_record(). Error: %d",
           err);
      throw common::NotFoundException(
          "Error during executing contacts_db_update_record().");
    }
    if (CONTACTS_ERROR_DB == err) {
      LoggerE("Error during executing contacts_db_update_record(). Error: %d",
           err);
      throw common::UnknownException(
          "Error during executing contacts_db_update_record().");
    }
  }

  ContactUtil::UpdateAdditionalInformation(contacts_record_ptr, &out);
}

void AddressBook_remove(const JsonObject& args, JsonObject&) {
  LoggerE("entered");
  ContactUtil::CheckDBConnection();

  int contact_id = common::stol(FromJson<JsonString>(args, "id"));

  if (contact_id < 0) {
    throw common::InvalidValuesException("Nagative contact id");
  }

  int err = contacts_db_delete_record(_contacts_contact._uri, contact_id);
  if (CONTACTS_ERROR_NO_DATA == err) {
    throw common::NotFoundException("Remove failed: contact not found");
  } else if (CONTACTS_ERROR_NONE != err) {
    throw common::UnknownException("Contacts record delete error");
  }
}

void AddressBook_addBatch(const JsonObject& args, JsonArray& out) {
  LoggerD("Enter");

  ContactUtil::CheckDBConnection();

  const JsonArray& batch_args = FromJson<JsonArray>(args, "batchArgs");
  long addressBookId = common::stol(FromJson<JsonString>(args, "addressBookId"));
  addressBookId = addressBookId == -1 ? 0 : addressBookId;

  unsigned length = batch_args.size();
  contacts_list_h contacts_list = NULL;
  int error_code = contacts_list_create(&contacts_list);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("list creation failed, code: %d", error_code);
    throw new common::UnknownException("list creation failed");
  }
  ContactUtil::ContactsListHPtr contacts_list_ptr(&contacts_list, ContactUtil::ContactsListDeleter);

  for (auto& item : batch_args) {
    contacts_record_h contacts_record = nullptr;
    int err = 0;
    err = contacts_record_create(_contacts_contact._uri, &contacts_record);
    if (CONTACTS_ERROR_NONE != err) {
      LoggerW("Contacts record create error, error code: %d", err);
      throw common::UnknownException("Contacts record create error");
    }
    ContactUtil::ContactsRecordHPtr x(&contacts_record, ContactUtil::ContactsDeleter);
    ContactUtil::ExportContactToContactsRecord(contacts_record, JsonCast<JsonObject>(item));
    ContactUtil::SetIntInRecord(contacts_record, _contacts_contact.address_book_id, addressBookId);
    error_code = contacts_list_add(*contacts_list_ptr, *(x.release()));
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerE("error during add record to list, code: %d", error_code);
      throw new common::UnknownException("error during add record to list");
    }
  }

  int* ids;
  int count;
  error_code = contacts_db_insert_records(*contacts_list_ptr, &ids, &count);
  if (CONTACTS_ERROR_NONE != error_code) {
    if (ids) {
      free(ids);
      ids = NULL;
    }
    LoggerE("inserting contacts to db fails, code: %d", error_code);
    throw new common::UnknownException("inserting contacts to db fails");
  }
  if (length != count) {
    LoggerW("Added different number of contacts");
  }

  for (unsigned int i = 0; i < count; i++) {
    JsonObject out_object;
    contacts_record_h contact_record = nullptr;
    error_code = contacts_db_get_record(_contacts_contact._uri, ids[i], &contact_record);
    if (CONTACTS_ERROR_NONE != error_code) {
      if (ids) {
        free(ids);
        ids = NULL;
      }
      LoggerW("Contacts record get error, error code: %d", error_code);
      throw common::UnknownException("Contacts record get error");
    }
    ContactUtil::ImportContactFromContactsRecord(contact_record, &out_object);
    out.push_back(JsonValue{out_object});
  }
  if (ids) {
    free(ids);
    ids = NULL;
  }
}

// TODO all batch operations should be implemented using CAPI batch functions
void AddressBook_batchFunc(NativeFunction impl, const char *single_arg_name,
                           const JsonObject &args, JsonArray &out) {
  const JsonArray &batch_args = FromJson<JsonArray>(args, "batchArgs");
  const JsonObject &address_book = FromJson<JsonObject>(args, "addressBook");

  int i = 0;
  for (auto &item : batch_args) {
    ++i;
    JsonObject single_args{};

    single_args.insert(std::make_pair("addressBook", address_book));
    single_args.insert(std::make_pair(single_arg_name, item));

    JsonObject single_out;
    impl(single_args, single_out);
    if (!single_out.empty()) {
      out.push_back(JsonValue{single_out});
    }
  }
}

void AddressBook_find(const JsonObject& args, JsonArray& array) {
  ContactUtil::CheckDBConnection();

  // TODO implement contact filter and sorting.
  const JsonObject& address_book = FromJson<JsonObject>(args, "addressBook");
  long addressbook_id = common::stol(FromJson<std::string>(address_book, "id"));
  // Read calendar by ID
  int error_code = 0;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;
  contacts_list_h list = nullptr;

  error_code = contacts_query_create(_contacts_contact._uri, &query);
  ContactUtil::ErrorChecker(error_code, "Failed contacts_query_create");
  ContactUtil::ContactsQueryHPtr query_ptr(&query, ContactUtil::ContactsQueryDeleter);
  error_code = contacts_filter_create(_contacts_contact._uri, &filter);
  ContactUtil::ErrorChecker(error_code, "Failed contacts_filter_create");
  ContactUtil::ContactsFilterPtr filter_ptr(filter, ContactUtil::ContactsFilterDeleter);
  error_code = contacts_filter_add_int(filter, _contacts_contact.address_book_id,
                                       CONTACTS_MATCH_EQUAL, addressbook_id);
  ContactUtil::ErrorChecker(error_code, "Failed contacts_filter_add_int");
  error_code = contacts_query_set_filter(query, filter);
  ContactUtil::ErrorChecker(error_code, "Failed contacts_query_set_filter");
  error_code = contacts_db_get_records_with_query(query, 0, 0, &list);
  ContactUtil::ErrorChecker(error_code, "Failed contacts_db_get_records_with_query");
  ContactUtil::ContactsListHPtr list_ptr(&list, ContactUtil::ContactsListDeleter);

  int record_count = 0;
  error_code = contacts_list_get_count(list, &record_count);

  contacts_list_first(list);
  for (unsigned int i = 0; i < record_count; i++) {
    contacts_record_h record;
    error_code = contacts_list_get_current_record_p(list, &record);
    ContactUtil::ErrorChecker(error_code, "Failed contacts_list_get_current_record_p");

    int id_value = 0;
    error_code = contacts_record_get_int(record, _contacts_contact.id, &id_value);
    ContactUtil::ErrorChecker(error_code, "Failed contacts_record_get_int");

    array.push_back(JsonValue(static_cast<double>(id_value)));
    contacts_list_next(list);
  }
}

void AddressBook_addGroup(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();
  const JsonObject& group = FromJson<JsonObject>(args, "group");
  if (!IsNull(group, "id") || !IsNull(group, "addressBookId")) {
    LoggerE("Group object is previously added");
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
      common::stol(FromJson<JsonString>(args, "addressBookId"));
  addressbook_id = (IsUnified(addressbook_id)) ? 0 : addressbook_id;
  ContactUtil::SetIntInRecord(contacts_record, _contacts_group.address_book_id,
                              addressbook_id);

  ContactUtil::ExportContactGroupToContactsRecord(contacts_record, group);
  int groupId = 0;
  err = contacts_db_insert_record(contacts_record, &groupId);
  ContactUtil::ErrorChecker(err, "Error during insert group record");

  out.insert(std::make_pair("id", std::to_string(groupId)));
  out.insert(
      std::make_pair("addressBookId", std::to_string(addressbook_id)));
}

void AddressBook_getGroup(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();

  long id = common::stol(FromJson<JsonString>(args, "id"));

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
      common::stol(FromJson<JsonString>(args, "addressBook", "id"));
  if (IsUnified(addressbook_id)) {
    int address_book_id = 0;
    ContactUtil::GetIntFromRecord(
        contacts_record, _contacts_group.address_book_id, &address_book_id);
    if (address_book_id != addressbook_id) {
      throw common::NotFoundException("No group in this address book.");
    }
  }

  ContactUtil::ImportContactGroupFromContactsRecord(contacts_record, &out);
}

void AddressBook_updateGroup(const JsonObject& args, JsonObject&) {
  ContactUtil::CheckDBConnection();

  const JsonObject& group = FromJson<JsonObject>(args, "group");

  if (IsNull(group, "id") || IsNull(group, "addressBookId")) {
    LoggerE("Group object is not added");
    throw common::InvalidValuesException("Group object is not added");
  }

  long addressbook_id =
      common::stol(FromJson<JsonString>(args, "addressBookId"));
  long group_addressbook_id =
      common::stol(FromJson<JsonString>(group, "addressBookId"));
  if (IsUnified(addressbook_id) && (addressbook_id != group_addressbook_id)) {
    LoggerE("Wrong address book");
    throw common::InvalidValuesException("Wrong address book");
  }

  if (FromJson<bool>(group, "readOnly")) {
    LoggerW("Group is readonly - cancel update");
    throw common::UnknownException("Group is readonly - cancel update");
  }

  long group_id = common::stol(FromJson<JsonString>(group, "id"));
  if (group_id < 0) {
    throw common::InvalidValuesException("Incorrect group id");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h contacts_record = nullptr;
  err =
      contacts_db_get_record(_contacts_group._uri, group_id, &contacts_record);
  if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
    LoggerE("Error during executing contacts_db_get_record()");
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
    LoggerE("Problem during db_update_record");
    throw common::NotFoundException("Problem during db_update_record");
  }
  ContactUtil::ErrorChecker(err, "Problem during db_update_record");
}

void AddressBook_removeGroup(const JsonObject& args, JsonObject&) {
  ContactUtil::CheckDBConnection();

  long id = common::stol(FromJson<JsonString>(args, "id"));
  if (id < 0) {
    throw common::InvalidValuesException("Incorrect group id");
  }

  int err = CONTACTS_ERROR_NONE;
  long addressbook_id = AddressBookId(args);
  if (!IsUnified(addressbook_id)) {
    contacts_record_h contacts_record = nullptr;
    err = contacts_db_get_record(_contacts_group._uri, id, &contacts_record);
    if (CONTACTS_ERROR_NONE != err || contacts_record == nullptr) {
      LoggerE("No group");
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
    LoggerE("Problem during db_update_record");
    throw common::NotFoundException("Problem during db_delete_record");
  }
  ContactUtil::ErrorChecker(err, "Problem during db_delete_record");
}

void AddressBook_getGroups(const JsonObject& args, JsonArray& out) {
  ContactUtil::CheckDBConnection();

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
  int record_count = 0;
  err = contacts_list_get_count(groups_list, &record_count);
  ContactUtil::ErrorChecker(err, "Fail to get contacts_list_get_count ");

  contacts_list_first(groups_list);

  for (unsigned int i = 0; i < record_count; i++) {
    contacts_record_h contacts_record;
    err = contacts_list_get_current_record_p(groups_list, &contacts_record);
    if (CONTACTS_ERROR_NONE != err || nullptr == contacts_record) {
      LoggerE("Fail to get group record");
      throw common::UnknownException("Fail to get group record");
    }

    JsonValue group{JsonObject{}};
    ContactUtil::ImportContactGroupFromContactsRecord(contacts_record, &group.get<JsonObject>());
    out.push_back(group);

    if (i < record_count - 1) {
      err = contacts_list_next(groups_list);
      if (CONTACTS_ERROR_NONE != err) {
        LoggerE("Fail to get next address book, error %d", err);
        break;
      }
    }
  }
}

namespace {
void AddressBook_listenerCallback(const char* view_uri, void* user_data) {
  (void)view_uri;
  (void)user_data;
  ContactUtil::CheckDBConnection();

  int* last_database_version = static_cast<int*>(user_data);

  contacts_list_h contacts_list = nullptr;

  int error_code = contacts_db_get_changes_by_version(
      _contacts_contact_updated_info._uri, kUnifiedAddressBookId,
      *last_database_version, &contacts_list, last_database_version);

  ContactUtil::ContactsListHPtr contacts_list_ptr(
      &contacts_list, ContactUtil::ContactsListDeleter);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("cannot get changes by version, code: %d", error_code);
  } else {
    int count = 0;

    error_code = contacts_list_get_count(contacts_list, &count);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("Cannot get updated contact count, code: %d", error_code);
      return;
    }

    if (!count) {
      LoggerW("No updated contacts");
      return;
    }

    contacts_list_first(contacts_list);

    JsonValue result{JsonObject{}};
    JsonObject& result_obj = result.get<JsonObject>();
    result_obj.insert(std::make_pair("listenerId", kContactListenerId));
    JsonArray& added = result_obj.insert(std::make_pair("added", JsonArray{}))
                           .first->second.get<JsonArray>();
    JsonArray& updated =
        result_obj.insert(std::make_pair("updated", JsonArray{}))
            .first->second.get<JsonArray>();
    JsonArray& removed =
        result_obj.insert(std::make_pair("removed", JsonArray{}))
            .first->second.get<JsonArray>();

    for (unsigned int i = 0; i < count; i++) {
      contacts_record_h contact_updated_record = nullptr;

      error_code = contacts_list_get_current_record_p(contacts_list,
                                                      &contact_updated_record);
      if (CONTACTS_ERROR_NONE != error_code) {
        LoggerW("fail to get contact from list, code: %d", error_code);
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
      catch (const PlatformException&) {
        LoggerE("failt to get int from record");
        return;
      }

      if (CONTACTS_CHANGE_INSERTED == changed_type ||
          CONTACTS_CHANGE_UPDATED == changed_type) {
        contacts_record_h contacts_record = nullptr;

        error_code = contacts_db_get_record(_contacts_contact._uri, changed_id,
                                            &contacts_record);

        if (CONTACTS_ERROR_NONE != error_code) {
          LoggerW("fail to get contact from record");
          return;
        }

        ContactUtil::ContactsRecordHPtr contact_record_ptr(
            &contacts_record, ContactUtil::ContactsDeleter);

        JsonValue contact{JsonObject{}};
        ContactUtil::ImportContactFromContactsRecord(
            *contact_record_ptr, &contact.get<JsonObject>());

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
            std::make_pair("id", std::to_string(changed_id)));
        removed_data_obj.insert(
            std::make_pair("addressBookId", std::to_string(changed_ab_id)));
        removed.push_back(std::move(removed_data));
      }
    }
    ContactInstance::GetInstance().PostMessage(result.serialize().c_str());
  }
}
}

void AddressBook_startListening(const JsonObject&, JsonObject& out) {
  ContactUtil::CheckDBConnection();
  // Set the initial latest version before registering the callback.
  // The callback should only be registered once so no race can occur.
  int error_code = contacts_db_get_current_version(&ContactInstance::current_state);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerW("get current version returns error, code: %d", error_code);
  }

  error_code = contacts_db_add_changed_cb(
      _contacts_contact._uri, AddressBook_listenerCallback, &ContactInstance::current_state);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Error while registering listener to contacts db, code: %d",
         error_code);
    throw UnknownException("Error while registering listener to contacts db");
  }
}

void AddressBook_stopListening(const JsonObject&, JsonObject& out) {
  ContactUtil::CheckDBConnection();
  int error_code = contacts_db_remove_changed_cb(
      _contacts_contact._uri, AddressBook_listenerCallback, &ContactInstance::current_state);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Error while removing listener");
    throw UnknownException("Error while removing listener");
  }
}

}  // AddressBook
}  // contact
}  // extension
