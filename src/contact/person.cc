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

#include "contact/person.h"
#include "common/converter.h"
#include "common/platform_exception.h"
#include "common/logger.h"

namespace extension {
namespace contact {
namespace Person {

using namespace common;

static const PersonPropertyMap personPropertyMap = {
    {"id", {_contacts_person.id, kPrimitiveTypeId}},
    {"displayName", {_contacts_person.display_name, kPrimitiveTypeString}},
    {"contactCount", {_contacts_person.link_count, kPrimitiveTypeLong}},
    {"hasPhoneNumber",
     {_contacts_person.has_phonenumber, kPrimitiveTypeBoolean}},
    {"hasEmail", {_contacts_person.has_email, kPrimitiveTypeBoolean}},
    {"isFavorite", {_contacts_person.is_favorite, kPrimitiveTypeBoolean}},
    {"photoURI", {_contacts_person.image_thumbnail_path, kPrimitiveTypeString}},
    {"ringtoneURI", {_contacts_person.ringtone_path, kPrimitiveTypeString}},
    {"displayContactId",
     {_contacts_person.display_contact_id, kPrimitiveTypeId}}, };

void PersonLink(const JsonObject& args, JsonObject&) {
  ContactUtil::CheckDBConnection();

  long id = common::stol(FromJson<JsonString>(args, "id"));
  long person_id = common::stol(FromJson<JsonString>(args, "person", "id"));

  contacts_record_h contacts_record = nullptr;

  int err = contacts_db_get_record(_contacts_person._uri, id, &contacts_record);
  contacts_record_destroy(contacts_record, true);
  contacts_record = nullptr;

  if (CONTACTS_ERROR_NONE != err) {
    LoggerW("Person was not found, error code: %d", err);
    throw common::NotFoundException("Person not found");
  }

  err = contacts_person_link_person(person_id, id);

  ContactUtil::ErrorChecker(err, "Error during executing person link()");
}

void PersonUnlink(const JsonObject& args, JsonObject& out) {
  ContactUtil::CheckDBConnection();

  long contact_id = common::stol(FromJson<JsonString>(args, "id"));

  contacts_record_h contacts_record = nullptr;
  int error_code = contacts_db_get_record(_contacts_simple_contact._uri,
                                          contact_id, &contacts_record);

  if (CONTACTS_ERROR_NONE != error_code) {
    contacts_record_destroy(contacts_record, true);
    contacts_record = nullptr;
    LoggerW("Contact not found, error code: %d", error_code);
    throw common::InvalidValuesException("Contact not found");
  }

  int contacts_person_id = 0;
  error_code = contacts_record_get_int(
      contacts_record, _contacts_simple_contact.person_id, &contacts_person_id);
  contacts_record_destroy(contacts_record, true);
  contacts_record = nullptr;

  ContactUtil::ErrorChecker(error_code, "Contact is not a member of person");

  long person_id = common::stol(FromJson<JsonString>(args, "person", "id"));
  if (contacts_person_id != person_id) {
    LoggerW("Contact is not a member of person (wrong id's)");
    throw common::InvalidValuesException("Contact is not a member of person");
  }

  int new_person_id = 0;

  error_code =
      contacts_person_unlink_contact(person_id, contact_id, &new_person_id);

  ContactUtil::ErrorChecker(error_code, "Error during executing unlink()");

  error_code = contacts_db_get_record(_contacts_person._uri, new_person_id,
                                      &contacts_record);
  if (CONTACTS_ERROR_NONE != error_code) {
    contacts_record_destroy(contacts_record, true);
    contacts_record = nullptr;
    LoggerW("Person not found, error code: %d", error_code);
    throw common::UnknownException("Person not found");
  }

  ContactUtil::ImportPersonFromContactsRecord(contacts_record, &out);

  contacts_record_destroy(contacts_record, true);
  contacts_record = nullptr;
}

const PersonProperty& PersonPropertyFromString(const std::string& name) {
  auto iter = personPropertyMap.find(name);
  if (iter == personPropertyMap.end()) {
    LoggerE("Invalid property name (not in map): %s", name.c_str());
    throw InvalidValuesException("Invalid property name");
  }
  return iter->second;
}

}  // Person
}  // contact
}  // extension
