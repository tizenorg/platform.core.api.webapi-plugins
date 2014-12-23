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

#include "contact/contact_manager.h"
#include <memory>

#include "common/converter.h"
#include "common/picojson.h"
#include "common/logger.h"

#include <contacts.h>
#include "contact/contact_instance.h"
#include "contact/person.h"

namespace extension {
namespace contact {
namespace ContactManager {

namespace {
const char* kContactPersonListenerId = "ContactPersonChangeListener";
const char* kTokenDelimiter = " ,:";
}

using namespace common;

void ContactManager_getAddressBooks(const JsonObject &args, JsonArray &out) {
  LoggerD("entered");

  ContactUtil::CheckDBConnection();

  contacts_list_h address_book_list = nullptr;

  int error_code = contacts_db_get_all_records(_contacts_address_book._uri, 0, 0,
      &address_book_list);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Fail to get address book list, error: %d", error_code);
    throw UnknownException("Fail to get address book list");
  }

  ContactUtil::ContactsListHPtr contacts_list_ptr(&address_book_list,
      ContactUtil::ContactsListDeleter);

  int record_count = 0;
  error_code = contacts_list_get_count(*contacts_list_ptr, &record_count);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Fail to get address book list count, error: %d", error_code);
    throw UnknownException("Fail to get address book list count");
  }

  error_code = contacts_list_first(*contacts_list_ptr);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Fail to get address book from list, error: %d", error_code);
    throw UnknownException("Fail to get address book from list");
  }

  for (unsigned int i = 0; i < record_count; i++) {
    contacts_record_h contacts_record = nullptr;
    error_code = contacts_list_get_current_record_p(*contacts_list_ptr, &contacts_record);

    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("Fail to get address book record");
      continue;
    }

    int id = 0;
    int mode = 0;
    char *name = nullptr;
    try {
      ContactUtil::GetIntFromRecord(contacts_record, _contacts_address_book.id, &id);
      ContactUtil::GetIntFromRecord(contacts_record, _contacts_address_book.mode, &mode);
      ContactUtil::GetStrFromRecord(contacts_record, _contacts_address_book.name, &name);
    }
    catch (...) {
      LoggerW("Fail to get data from address book");
      continue;
    }

    JsonValue single = JsonValue(JsonObject());
    JsonObject &single_obj = single.get<JsonObject>();
    single_obj["id"] = JsonValue(std::to_string(id));
    single_obj["name"] = JsonValue(name);
    single_obj["readOnly"] = JsonValue(CONTACTS_ADDRESS_BOOK_MODE_READONLY == mode);
    out.push_back(single);

    contacts_list_next(*contacts_list_ptr);
  }
}

void ContactManager_getAddressBook(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();
  long address_book_id;
  try {
    address_book_id = common::stol(FromJson<JsonString>(args, "addressBookID"));
  }
  catch (const common::InvalidValuesException&) {
    throw common::NotFoundException("Invalid id");
  }

  contacts_record_h contacts_record;
  int error_code = contacts_db_get_record(_contacts_address_book._uri,
                                          address_book_id, &contacts_record);
  if (CONTACTS_ERROR_NONE != error_code || nullptr == contacts_record) {
    LoggerE("Fail to get addressbook record, error code: %d", error_code);
    throw NotFoundException("Fail to get address book with given id");
  }

  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  int mode = 0;
  ContactUtil::GetIntFromRecord(contacts_record, _contacts_address_book.mode,
                                &mode);

  char* name = nullptr;
  ContactUtil::GetStrFromRecord(contacts_record, _contacts_address_book.name,
                                &name);

  out.insert(std::make_pair("name", std::string(name)));
  out.insert(std::make_pair(
      "readOnly",
      ((CONTACTS_ADDRESS_BOOK_MODE_READONLY == mode) ? "true" : "false")));
}

namespace {
void ContactManager_get_internal(int person_id, JsonObject* out) {
  int error_code = 0;
  contacts_record_h contacts_record = nullptr;

  error_code = contacts_db_get_record(_contacts_person._uri, person_id,
                                      &contacts_record);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Person with id: %d, not found, error: %d", person_id, error_code);
    throw NotFoundException("Person not found");
  }

  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  ContactUtil::ImportPersonFromContactsRecord(contacts_record, out);
}
}

void ContactManager_get(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();
  long person_id = common::stol(FromJson<JsonString>(args, "personID"));

  JsonValue val{JsonObject{}};
  ContactManager_get_internal(person_id, &out);
}

void ContactManager_update(const JsonObject& args, JsonObject&) {
  ContactUtil::CheckDBConnection();
  const JsonObject& person = FromJson<JsonObject>(args, "person");
  long person_id = common::stol(FromJson<JsonString>(person, "id"));
  int error_code = 0;

  contacts_record_h contacts_record = nullptr;

  error_code = contacts_db_get_record(_contacts_person._uri, person_id,
                                      &contacts_record);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Error during updating person, error code: %d", error_code);
    throw UnknownException("Error during updating person");
  }

  ContactUtil::ExportPersonToContactsRecord(contacts_record, person);
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  error_code = contacts_db_update_record(*contacts_record_ptr);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("error code: %d", error_code);
    throw UnknownException(
        "Error during executing contacts_db_update_record()");
  }
}

void ContactManager_remove(const JsonObject& args, JsonObject&) {
  ContactUtil::CheckDBConnection();
  long person_id = common::stol(FromJson<JsonString>(args, "personId"));

  if (person_id < 0) {
    throw common::InvalidValuesException("Negative person id");
  }

  int error_code = contacts_db_delete_record(_contacts_person._uri, person_id);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Error during removing contact, error: %d", error_code);
    throw NotFoundException("Error during removing contact");
  }
}

void ContactManager_find(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();

  // @todo implement
  throw common::NotFoundException("Not implemented");
}

void ContactManager_importFromVCard(const JsonObject& args, JsonObject& out) {
  // I'm not sure how to call it. Should it be 'Contact', 'vCard' or what?
  ContactUtil::CheckDBConnection();
  const char* vcard_char_ptr = FromJson<JsonString>(args, "contact").c_str();

  contacts_list_h contacts_list = nullptr;

  int err = CONTACTS_ERROR_NONE;
  err = contacts_vcard_parse_to_contacts(vcard_char_ptr, &contacts_list);
  if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
    LoggerE("Invalid vCard string");
    throw UnknownException("Invalid vCard string");
  } else if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Fail to convert vCard from string");
    throw UnknownException("Fail to convert vCard from string");
  }

  int record_count = 0;
  err = contacts_list_get_count(contacts_list, &record_count);
  if (CONTACTS_ERROR_NONE != err || 0 == record_count) {
    contacts_list_destroy(contacts_list, true);
    LoggerE("Invalid vCard string.");
    throw UnknownException("Invalid vCard string.");
  }

  contacts_record_h contacts_record;
  contacts_list_first(contacts_list);
  err = contacts_list_get_current_record_p(contacts_list, &contacts_record);
  if (CONTACTS_ERROR_NONE != err || nullptr == contacts_record) {
    contacts_list_destroy(contacts_list, true);
    LoggerE("Invalid vCard string.");
    throw UnknownException("Invalid vCard string.");
  }

  JsonValue result{JsonObject{}};
  ContactUtil::ImportContactFromContactsRecord(contacts_record, &out);
}

namespace {
bool IsNumeric(const char* s) {
  for (; *s; s++) {
    if (!isdigit(*s)) {
      return false;
    }
  }
  return true;
}

void ContactManager_listenerCallback(const char* view_uri, char* changes,
                                     void* user_data) {
  (void)view_uri;
  (void)user_data;

  if (nullptr == changes) {
    LoggerW("changes is NULL");
    return;
  }
  if (strlen(changes) == 0) {
    LoggerW("changes is empty");
    return;
  }

  JsonValue result{JsonObject{}};
  JsonObject& result_obj = result.get<JsonObject>();
  result_obj.insert(std::make_pair("listenerId", kContactPersonListenerId));
  JsonArray& added = result_obj.insert(std::make_pair("added", JsonArray{}))
                           .first->second.get<JsonArray>();
  JsonArray& updated =
      result_obj.insert(std::make_pair("updated", JsonArray{}))
          .first->second.get<JsonArray>();
  JsonArray& removed =
      result_obj.insert(std::make_pair("removed", JsonArray{}))
          .first->second.get<JsonArray>();

  std::unique_ptr<char, void (*)(char*)> tmp(strdup(changes),
                                             [](char* p) { free(p); });

  char* token = strtok(tmp.get(), kTokenDelimiter);
  while (token) {
    try {
      if (IsNumeric(token)) {
        int type = atoi(token);
        token = strtok(nullptr, kTokenDelimiter);
        if (!token) {
          break;
        }
        if (IsNumeric(token)) {
          int person_id = atoi(token);
          switch (type) {
            case CONTACTS_CHANGE_INSERTED: {
              added.push_back(JsonValue{JsonObject{}});
              ContactManager_get_internal(person_id,
                                          &added.back().get<JsonObject>());
              break;
            }
            case CONTACTS_CHANGE_UPDATED: {
              updated.push_back(JsonValue{JsonObject{}});
              ContactManager_get_internal(person_id,
                                          &updated.back().get<JsonObject>());
              break;
            }
            case CONTACTS_CHANGE_DELETED: {
              std::string id_str{std::to_string(person_id)};
              removed.push_back(JsonValue{id_str.c_str()});
              break;
            }
            default: {}
          }
        }
      }
    }
    catch (common::PlatformException& ex) {
      LoggerE("Caught exception %s\" in listener callback: %s",
              ex.name().c_str(), ex.message().c_str());
    }

    token = strtok(nullptr, kTokenDelimiter);
  }

  ContactInstance::GetInstance().PostMessage(result.serialize().c_str());
}
}

void ContactManager_startListening(const JsonObject& /*args*/, JsonObject& /*out*/) {
  ContactUtil::CheckDBConnection();
  int error_code = contacts_db_add_changed_cb_with_info(
      _contacts_person._uri, ContactManager_listenerCallback, nullptr);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("contacts_db_add_changed_cb(_contacts_person._uri) error: %d",
            error_code);
    throw UnknownException("Failed to start listening");
  }
}

void ContactManager_stopListening(const JsonObject& /*args*/, JsonObject& /*out*/) {
  ContactUtil::CheckDBConnection();
  int error_code = contacts_db_remove_changed_cb_with_info(
      _contacts_person._uri, ContactManager_listenerCallback, nullptr);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("contacts_db_remove_changed_cb(_contacts_person._uri) error: %d",
            error_code);
    throw UnknownException("Failed to stop listening");
  }
}

}  // namespace ContactManager
}  // namespace contact
}  // namespace extension
