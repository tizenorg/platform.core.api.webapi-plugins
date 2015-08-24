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

#include "contact/contact_util.h"
#include <algorithm>
#include <iomanip>
#include <string>
#include <unistd.h>
#include "common/converter.h"
#include "common/assert.h"
#include "common/logger.h"

namespace extension {
namespace contact {
namespace ContactUtil {

using common::ErrorCode;
using common::FromJson;
using common::IsNull;
using common::JsonCast;
using common::PlatformResult;

namespace {

static const std::string kSchema("file://");

std::string ConvertUriToPath(const std::string& str) {
  if (str.substr(0, kSchema.size()) == kSchema) {
    return str.substr(kSchema.size());
  }

  return str;
}

std::string ConvertPathToUri(const std::string& str) {
  if (str.substr(0, kSchema.size()) == kSchema) {
    return str;
  }

  return kSchema + str;
}

PlatformResult VerifyLocalPath(const std::string& path) {
  // path should be either empty or point to existing local path
  bool result = path.length() == 0
      || (path.length() > 0 && path[0] == '/'
          && (access(path.c_str(), F_OK) == 0));
  return PlatformResult(
      result ? ErrorCode::NO_ERROR : ErrorCode::INVALID_VALUES_ERR);
}

}  // namespace

void ContactsDeleter(contacts_record_h* contacts_record) {
  if (CONTACTS_ERROR_NONE != contacts_record_destroy(*contacts_record, true)) {
    LoggerE("failed to destroy contacts_record_h");
  }
}

void ContactsListDeleter(contacts_list_h* contacts_list) {
  if (CONTACTS_ERROR_NONE != contacts_list_destroy(*contacts_list, true)) {
    LoggerE("failed to destroy contacts_list_h");
  }
}

void ContactsFilterDeleter(contacts_filter_h contacts_filter) {
  if (CONTACTS_ERROR_NONE != contacts_filter_destroy(contacts_filter)) {
    LoggerE("failed to destroy contacts_filter_h");
  }
}

void ContactsQueryDeleter(contacts_query_h* contacts_query) {
  if (CONTACTS_ERROR_NONE != contacts_query_destroy(*contacts_query)) {
    LoggerE("failed to destroy contacts_query_h");
  }
}

namespace {
static const char kContactPhoneTypeHome[] = "HOME";
static const char kContactPhoneTypeWork[] = "WORK";
static const char kContactPhoneTypeVoice[] = "VOICE";
static const char kContactPhoneTypeFax[] = "FAX";
static const char kContactPhoneTypeMsg[] = "MSG";
static const char kContactPhoneTypeCell[] = "CELL";
static const char kContactPhoneTypePager[] = "PAGER";
static const char kContactPhoneTypeBbs[] = "BBS";
static const char kContactPhoneTypeModem[] = "MODEM";
static const char kContactPhoneTypeCar[] = "CAR";
static const char kContactPhoneTypeIsdn[] = "ISDN";
static const char kContactPhoneTypeVideo[] = "VIDEO";
static const char kContactPhoneTypePcs[] = "PCS";
static const char kContactPhoneTypeAssistant[] = "ASSISTANT";
static const char kContactPhoneTypeOther[] = "OTHER";
static const char kContactPhoneTypeCustom[] = "Custom";

static const char kContactEmailAddressTypeHome[] = "HOME";
static const char kContactEmailAddressTypeWork[] = "WORK";
static const char kContactEmailAddressTypeOther[] = "OTHER";
static const char kContactEmailAddressTypeCustom[] = "CUSTOM";

static const char kContactAddressTypeHome[] = "HOME";
static const char kContactAddressTypeWork[] = "WORK";
static const char kContactAddressTypeOther[] = "OTHER";
static const char kContactAddressTypeCustom[] = "CUSTOM";

static const char kContactWebSiteTypeHomePage[] = "HOMEPAGE";
static const char kContactWebSiteTypeBlog[] = "BLOG";

static const char kContactRelationshipTypeCustom[] = "CUSTOM";
static const char kContactRelationshipTypeAssistant[] = "ASSISTANT";
static const char kContactRelationshipTypeBrother[] = "BROTHER";
static const char kContactRelationshipTypeChild[] = "CHILD";
static const char kContactRelationshipTypeDomesticPartner[] =
    "DOMESTIC_PARTNER";
static const char kContactRelationshipTypeFather[] = "FATHER";
static const char kContactRelationshipTypeFriend[] = "FRIEND";
static const char kContactRelationshipTypeManager[] = "MANAGER";
static const char kContactRelationshipTypeMother[] = "MOTHER";
static const char kContactRelationshipTypeParent[] = "PARENT";
static const char kContactRelationshipTypePartner[] = "PARTNER";
static const char kContactRelationshipTypeReferredBy[] = "REFERRED_BY";
static const char kContactRelationshipTypeRelative[] = "RELATIVE";
static const char kContactRelationshipTypeSister[] = "SISTER";
static const char kContactRelationshipTypeSpouse[] = "SPOUSE";
static const char kContactRelationshipTypeOther[] = "OTHER";

static const char kContactInstantMessageTypeOther[] = "OTHER";
static const char kContactInstantMessageTypeGoogle[] = "GOOGLE";
static const char kContactInstantMessageTypeWlm[] = "WLM";
static const char kContactInstantMessageTypeYahoo[] = "YAHOO";
static const char kContactInstantMessageTypeFacebook[] = "FACEBOOK";
static const char kContactInstantMessageTypeIcq[] = "ICQ";
static const char kContactInstantMessageTypeAim[] = "AIM";
static const char kContactInstantMessageTypeQq[] = "QQ";
static const char kContactInstantMessageTypeJabber[] = "JABBER";
static const char kContactInstantMessageTypeSkype[] = "SKYPE";
static const char kContactInstantMessageTypeIrc[] = "IRC";
static const char kContactInstantMessageTypeCustom[] = "CUSTOM";
}

PlatformResult ErrorChecker(int err, const char* message) {
  LoggerD("Enter");
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("%s, error code: %i", message, err);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult GetStrFromRecord(contacts_record_h record,
                                unsigned int property_id, char** value) {
  LoggerD("Enter");
  int err = contacts_record_get_str_p(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error during getting contact record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult GetIntFromRecord(contacts_record_h record,
                                unsigned int property_id, int* value) {
  LoggerD("Enter");
  int err = contacts_record_get_int(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error during getting contact record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult GetBoolFromRecord(contacts_record_h record,
                                 unsigned int property_id, bool* value) {
  LoggerD("Enter");
  int err = contacts_record_get_bool(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error during getting contact record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SetStrInRecord(contacts_record_h record,
                              unsigned int property_id, const char* value) {
  LoggerD("Enter");
  int err = contacts_record_set_str(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during setting str contact record property, error code: %i",
            err);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error during setting contact record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SetIntInRecord(contacts_record_h record,
                              unsigned int property_id, int value) {
  LoggerD("Enter");
  int err = contacts_record_set_int(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error during setting contact record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SetBoolInRecord(contacts_record_h record,
                               unsigned int property_id, bool value) {
  LoggerD("Enter");
  int err = contacts_record_set_bool(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error during setting contact record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ClearAllContactRecord(contacts_record_h contacts_record,
                                     unsigned int property_id) {
  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  int record_count;
  PlatformResult status =
      GetNumberOfChildRecord(contacts_record, property_id, &record_count);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  for (int i = 0; i < record_count; ++i) {
    unsigned int actual_index = record_count - 1 - i;
    contacts_record_h phone_record = nullptr;
    int err = contacts_record_get_child_record_at_p(
        contacts_record, property_id, actual_index, &phone_record);
    PlatformResult status =
        ContactUtil::ErrorChecker(err, "Error during getting phone record");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    err = contacts_record_remove_child_record(contacts_record, property_id,
                                              phone_record);
    status =
        ContactUtil::ErrorChecker(err, "Error during getting phone record");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult GetNumberOfChildRecord(contacts_record_h contacts_record,
                                      unsigned int property_id,
                                      int* child_count) {
  LoggerD("Enter");
  Assert(child_count);

  int err = contacts_record_get_child_record_count(contacts_record, property_id,
                                                   child_count);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Problem during getting child count");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactNameFromContactsRecord(
    contacts_record_h contacts_record, JsonObject* out_ptr,
    bool* is_contact_name) {
  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  if (!contacts_record) {
    LoggerW("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }
  int count = 0;
  int err = contacts_record_get_child_record_count(contacts_record,
                                                   _contacts_contact.name, &count);
  PlatformResult status =
      ContactUtil::ErrorChecker(err, "Contacts child record get count error");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (count > 1) {
    LoggerE("More than one ContactName for one Contact");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "More than one ContactName for one Contact");
  }
  LoggerD("Contact name record count: %i", count);

  if (count == 0) {
    *is_contact_name = false;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  contacts_record_h contact_name = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.name, 0, &contact_name);
  status =
      ContactUtil::ErrorChecker(err, "Contacts name record get childerror");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // Documentation says:
  //  child_record MUST NOT be released by you.
  //  It is released when the parent record handle is destroyed.
  // so it won't be protected by unique_ptr.

  char* char_value = nullptr;
  status = ContactUtil::GetStrFromRecord(contact_name, _contacts_name.prefix,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("prefix",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(contact_name, _contacts_name.suffix,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("suffix",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(contact_name, _contacts_name.first,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("firstName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(contact_name, _contacts_name.addition,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("middleName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(contact_name, _contacts_name.last,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("lastName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(
      contact_name, _contacts_name.phonetic_first, &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("phoneticFirstName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(
      contact_name, _contacts_name.phonetic_middle, &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("phoneticMiddleName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(
      contact_name, _contacts_name.phonetic_last, &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("phoneticLastName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  err = contacts_record_get_child_record_count(
      contacts_record, _contacts_contact.nickname, &count);
  status =
      ContactUtil::ErrorChecker(err, "Contacts child record get count error");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  JsonArray nicknames;
  for (int i = 0; i < count; ++i) {
    contacts_record_h nickname = nullptr;
    err = contacts_record_get_child_record_at_p(
        contacts_record, _contacts_contact.nickname, i, &nickname);
    status = ContactUtil::ErrorChecker(
        err, "Contacts nicknames record get child error");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    status = ContactUtil::GetStrFromRecord(nickname, _contacts_nickname.name,
                                           &char_value);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (char_value) {
      nicknames.push_back(JsonValue{char_value});
    }
  }
  out["nicknames"] = picojson::value(nicknames);

  *is_contact_name = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactNameToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerW("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h contact_name = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.name, 0, &contact_name);
  bool update = true;
  if (CONTACTS_ERROR_NONE != err && nullptr == contact_name) {
    err = contacts_record_create(_contacts_name._uri, &contact_name);
    PlatformResult status =
        ContactUtil::ErrorChecker(err, "Contacts record create error");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    update = false;
  }
  // contact_name starts to be protected by unique_ptr
  ContactsRecordHPtr contacts_name_ptr(&contact_name, ContactsDeleter);

  if (!IsNull(in, "prefix")) {
    PlatformResult status =
        ContactUtil::SetStrInRecord(*contacts_name_ptr, _contacts_name.prefix,
                                    FromJson<JsonString>(in, "prefix").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "suffix")) {
    PlatformResult status =
        ContactUtil::SetStrInRecord(*contacts_name_ptr, _contacts_name.suffix,
                                    FromJson<JsonString>(in, "suffix").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "firstName")) {
    PlatformResult status = ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.first,
        FromJson<JsonString>(in, "firstName").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "middleName")) {
    PlatformResult status = ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.addition,
        FromJson<JsonString>(in, "middleName").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "lastName")) {
    PlatformResult status = ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.last,
        FromJson<JsonString>(in, "lastName").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "phoneticFirstName")) {
    PlatformResult status = ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.phonetic_first,
        FromJson<JsonString>(in, "phoneticFirstName").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "phoneticMiddleName")) {
    PlatformResult status = ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.phonetic_middle,
        FromJson<JsonString>(in, "phoneticMiddleName").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "phoneticLastName")) {
    PlatformResult status = ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.phonetic_last,
        FromJson<JsonString>(in, "phoneticLastName").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  // contact_name is being added as a child to contacts_record
  // and in future will be destroyed by contacts_record's contactsDeleter
  if (!update) {
    err = contacts_record_add_child_record(
        contacts_record, _contacts_contact.name, *contacts_name_ptr);
    PlatformResult status =
        ContactUtil::ErrorChecker(err, "Contacts record add child error");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  // and now unique_ptr can be released - contacts_name is protected
  // by its parent (contacts_record)
  contacts_name_ptr.release();

  const JsonArray& nicknames = FromJson<JsonArray>(in, "nicknames");

  for (auto& nickname : nicknames) {
    contacts_record_h nickname_record = nullptr;
    err = contacts_record_get_child_record_at_p(
        contacts_record, _contacts_contact.nickname, 0, &nickname_record);
    if (CONTACTS_ERROR_NONE != err && nullptr == nickname_record) {
      err = contacts_record_create(_contacts_nickname._uri, &nickname_record);
      PlatformResult status =
          ContactUtil::ErrorChecker(err, "Contacts record create error");
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }

      update = false;
    }
    ContactsRecordHPtr nickname_ptr(&nickname_record, ContactsDeleter);

    PlatformResult status =
        ContactUtil::SetStrInRecord(*nickname_ptr, _contacts_nickname.name,
                                    JsonCast<JsonString>(nickname).c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (!update) {
      err = contacts_record_add_child_record(
          contacts_record, _contacts_contact.nickname, *nickname_ptr);
      PlatformResult status =
          ContactUtil::ErrorChecker(err, "Contacts record add child error");
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }
    // Do not delete record, it is passed to the platform
    nickname_ptr.release();
  }

  // TODO update displayName in JS!

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactEmailAddressFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.email, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  char* email = nullptr;
  PlatformResult status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_email.email, &email);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!email) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  out.insert(std::make_pair("email", JsonValue{email}));

  bool is_default = false;
  status = ContactUtil::GetBoolFromRecord(
      child_record, _contacts_email.is_default, &is_default);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("isDefault", JsonValue{is_default}));

  char* label = nullptr;
  status = ContactUtil::GetStrFromRecord(child_record, _contacts_email.label,
                                         &label);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("label", label ? JsonValue{label} : JsonValue{}));

  int type = 0;
  status =
      ContactUtil::GetIntFromRecord(child_record, _contacts_email.type, &type);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  JsonArray types;
  if (type & CONTACTS_EMAIL_TYPE_HOME) {
    types.push_back(JsonValue{kContactEmailAddressTypeHome});
  }
  if (type & CONTACTS_EMAIL_TYPE_WORK) {
    types.push_back(JsonValue{kContactEmailAddressTypeWork});
  }
  if (CONTACTS_EMAIL_TYPE_OTHER == type) {
    types.push_back(JsonValue{kContactEmailAddressTypeOther});
  }
  if (type & CONTACTS_EMAIL_TYPE_CUSTOM) {
    types.push_back(JsonValue{kContactEmailAddressTypeCustom});
  }

  if (0 == types.size()) {
    types.push_back(JsonValue{kContactEmailAddressTypeHome});
  }

  out.insert(std::make_pair("types", JsonValue{types}));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactEmailAddressToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  contacts_record_h c_email_record_h = nullptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  int err = contacts_record_create(_contacts_email._uri, &c_email_record_h);
  PlatformResult status = ContactUtil::ErrorChecker(
      err, "Failed to create email record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&c_email_record_h, ContactsDeleter);

  status =
      ContactUtil::SetStrInRecord(c_email_record_h, _contacts_email.email,
                                  FromJson<JsonString>(in, "email").c_str());
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status =
      ContactUtil::SetBoolInRecord(c_email_record_h, _contacts_email.is_default,
                                   FromJson<bool>(in, "isDefault"));
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!IsNull(in, "label")) {
    status =
        ContactUtil::SetStrInRecord(c_email_record_h, _contacts_email.label,
                                    FromJson<JsonString>(in, "label").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  int type_to_set = 0;

  auto& types = FromJson<JsonArray>(in, "types");
  for (auto& type : types) {
    auto type_str = JsonCast<JsonString>(type);
    if (type_str == kContactEmailAddressTypeWork) {
      type_to_set |= CONTACTS_EMAIL_TYPE_WORK;
    } else if (type_str == kContactEmailAddressTypeOther) {
      type_to_set |= CONTACTS_EMAIL_TYPE_OTHER;
    } else if (type_str == kContactEmailAddressTypeCustom) {
      type_to_set |= CONTACTS_EMAIL_TYPE_CUSTOM;
    } else {
      type_to_set |= CONTACTS_EMAIL_TYPE_HOME;
    }
  }
  status = ContactUtil::SetIntInRecord(c_email_record_h, _contacts_email.type,
                                       type_to_set);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.email, c_email_record_h);
  status =
      ContactUtil::ErrorChecker(err, "Fail to save email record into database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // Do not delete record, it is passed to the platform
  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactPhoneNumberFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  contacts_record_h child_record = nullptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.number, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  char* phone = nullptr;
  PlatformResult status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_number.number, &phone);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("number", JsonValue{phone}));

  bool is_default = false;
  status = ContactUtil::GetBoolFromRecord(
      child_record, _contacts_number.is_default, &is_default);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("isDefault", JsonValue{is_default}));
  int type = 0;
  status =
      ContactUtil::GetIntFromRecord(child_record, _contacts_number.type, &type);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  JsonArray types;
  if (type & CONTACTS_NUMBER_TYPE_HOME) {
    types.push_back(JsonValue{kContactPhoneTypeHome});
  }
  if (type & CONTACTS_NUMBER_TYPE_WORK) {
    types.push_back(JsonValue{kContactPhoneTypeWork});
  }
  if (type & CONTACTS_NUMBER_TYPE_VOICE) {
    types.push_back(JsonValue{kContactPhoneTypeVoice});
  }
  if (type & CONTACTS_NUMBER_TYPE_FAX) {
    types.push_back(JsonValue{kContactPhoneTypeFax});
  }
  if (type & CONTACTS_NUMBER_TYPE_MSG) {
    types.push_back(JsonValue{kContactPhoneTypeMsg});
  }
  if (type & CONTACTS_NUMBER_TYPE_CELL) {
    types.push_back(JsonValue{kContactPhoneTypeCell});
  }
  if (type & CONTACTS_NUMBER_TYPE_PAGER) {
    types.push_back(JsonValue{kContactPhoneTypePager});
  }
  if (type & CONTACTS_NUMBER_TYPE_BBS) {
    types.push_back(JsonValue{kContactPhoneTypeBbs});
  }
  if (type & CONTACTS_NUMBER_TYPE_MODEM) {
    types.push_back(JsonValue{kContactPhoneTypeModem});
  }
  if (type & CONTACTS_NUMBER_TYPE_CAR) {
    types.push_back(JsonValue{kContactPhoneTypeCar});
  }
  if (type & CONTACTS_NUMBER_TYPE_ISDN) {
    types.push_back(JsonValue{kContactPhoneTypeIsdn});
  }
  if (type & CONTACTS_NUMBER_TYPE_VIDEO) {
    types.push_back(JsonValue{kContactPhoneTypeVideo});
  }
  if (type & CONTACTS_NUMBER_TYPE_PCS) {
    types.push_back(JsonValue{kContactPhoneTypePcs});
  }
  if (type & CONTACTS_NUMBER_TYPE_ASSISTANT) {
    types.push_back(JsonValue{kContactPhoneTypeAssistant});
  }
  if (CONTACTS_NUMBER_TYPE_OTHER == type) {
    types.push_back(JsonValue{kContactPhoneTypeOther});
  }
  if (type & CONTACTS_NUMBER_TYPE_CUSTOM) {
    types.push_back(JsonValue{kContactPhoneTypeCustom});
  }
  if (0 == types.size()) {
    types.push_back(JsonValue{kContactPhoneTypeVoice});
  }
  out["types"] = picojson::value(types);

  char* label = nullptr;
  status = ContactUtil::GetStrFromRecord(child_record, _contacts_number.label,
                                         &label);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out["label"] = label ? JsonValue{label} : JsonValue{};

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactPhoneNumberToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  contacts_record_h phone_record = nullptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  int err = contacts_record_create(_contacts_number._uri, &phone_record);
  PlatformResult status =
      ContactUtil::ErrorChecker(err, "Fail to create phone_record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&phone_record, ContactsDeleter);

  status =
      ContactUtil::SetStrInRecord(phone_record, _contacts_number.number,
                                  FromJson<JsonString>(in, "number").c_str());
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status =
      ContactUtil::SetBoolInRecord(phone_record, _contacts_number.is_default,
                                   FromJson<bool>(in, "isDefault"));
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!IsNull(in, "label")) {
    status =
        ContactUtil::SetStrInRecord(phone_record, _contacts_address.label,
                                    FromJson<JsonString>(in, "label").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  int type_to_set = 0;
  const JsonArray& types = FromJson<JsonArray>(in, "types");
  for (auto& type : types) {
    auto& type_str = JsonCast<JsonString>(type);
    if (type_str == kContactPhoneTypeHome) {
      type_to_set |= CONTACTS_NUMBER_TYPE_HOME;
    } else if (type_str == kContactPhoneTypeWork) {
      type_to_set |= CONTACTS_NUMBER_TYPE_WORK;
    } else if (type_str == kContactPhoneTypeVoice) {
      type_to_set |= CONTACTS_NUMBER_TYPE_VOICE;
    } else if (type_str == kContactPhoneTypeFax) {
      type_to_set |= CONTACTS_NUMBER_TYPE_FAX;
    } else if (type_str == kContactPhoneTypeMsg) {
      type_to_set |= CONTACTS_NUMBER_TYPE_MSG;
    } else if (type_str == kContactPhoneTypeCell) {
      type_to_set |= CONTACTS_NUMBER_TYPE_CELL;
    } else if (type_str == kContactPhoneTypePager) {
      type_to_set |= CONTACTS_NUMBER_TYPE_PAGER;
    } else if (type_str == kContactPhoneTypeBbs) {
      type_to_set |= CONTACTS_NUMBER_TYPE_BBS;
    } else if (type_str == kContactPhoneTypeModem) {
      type_to_set |= CONTACTS_NUMBER_TYPE_MODEM;
    } else if (type_str == kContactPhoneTypeCar) {
      type_to_set |= CONTACTS_NUMBER_TYPE_CAR;
    } else if (type_str == kContactPhoneTypeIsdn) {
      type_to_set |= CONTACTS_NUMBER_TYPE_ISDN;
    } else if (type_str == kContactPhoneTypeVideo) {
      type_to_set |= CONTACTS_NUMBER_TYPE_VIDEO;
    } else if (type_str == kContactPhoneTypePcs) {
      type_to_set |= CONTACTS_NUMBER_TYPE_PCS;
    } else if (type_str == kContactPhoneTypeAssistant) {
      type_to_set |= CONTACTS_NUMBER_TYPE_ASSISTANT;
    } else if (type_str == kContactPhoneTypeOther) {
      type_to_set |= CONTACTS_NUMBER_TYPE_OTHER;
    } else if (type_str == kContactPhoneTypeCustom) {
      type_to_set |= CONTACTS_NUMBER_TYPE_CUSTOM;
    } else {
      type_to_set |= CONTACTS_NUMBER_TYPE_VOICE;
    }
  }

  status = ContactUtil::SetIntInRecord(phone_record, _contacts_number.type,
                                       type_to_set);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.number, phone_record);
  status = ContactUtil::ErrorChecker(
      err, "Fail to set number value to phone_record");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // Do not delete record, it is passed to the platform
  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactOrganizationFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.company, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  char* char_value = nullptr;
  PlatformResult status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_company.name, &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair("name", char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_company.department, &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("department",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_company.job_title, &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("title",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(child_record, _contacts_company.role,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair("role", char_value ? JsonValue{char_value} : JsonValue{}));

  status = ContactUtil::GetStrFromRecord(child_record, _contacts_company.logo,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair(
          "logoURI",
          char_value ? JsonValue{ConvertPathToUri(char_value)} : JsonValue{}));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactOrganizationToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h organization_record = nullptr;
  int err = contacts_record_create(_contacts_company._uri, &organization_record);
  PlatformResult status = ContactUtil::ErrorChecker(
      err, "Failed to create organization record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&organization_record, ContactsDeleter);

  if (!IsNull(in, "name")) {
    status =
        ContactUtil::SetStrInRecord(organization_record, _contacts_company.name,
                                    FromJson<JsonString>(in, "name").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "department")) {
    status = ContactUtil::SetStrInRecord(
        organization_record, _contacts_company.department,
        FromJson<JsonString>(in, "department").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "title")) {
    status = ContactUtil::SetStrInRecord(
        organization_record, _contacts_company.job_title,
        FromJson<JsonString>(in, "title").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "role")) {
    status =
        ContactUtil::SetStrInRecord(organization_record, _contacts_company.role,
                                    FromJson<JsonString>(in, "role").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "logoURI")) {
    std::string path =
        ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "logoURI"));
    status = ContactUtil::SetStrInRecord(organization_record,
                                         _contacts_company.logo, path.c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.company, organization_record);
  status = ContactUtil::ErrorChecker(
      err, "Fail to set company value to child_record");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // Do not delete record, it is passed to the platform
  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactWebSiteFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.url, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  char* char_value = nullptr;
  PlatformResult status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_url.url, &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair(std::string("url"),
                     picojson::value(char_value ? char_value : "")));

  int type = 0;
  status =
      ContactUtil::GetIntFromRecord(child_record, _contacts_url.type, &type);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair(std::string("type"),
                            picojson::value((CONTACTS_URL_TYPE_HOME == type)
                                            ? kContactWebSiteTypeHomePage
                                                : kContactWebSiteTypeBlog)));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactWebSiteToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  if (IsNull(in, "url")) {
    LoggerD("WebSite urls are not set");
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  const std::string& url = FromJson<JsonString>(in, "url");
  if (url.empty()) {
    LoggerD("WebSite urls are not set");
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  contacts_record_h website_record_h = nullptr;
  int err = contacts_record_create(_contacts_url._uri, &website_record_h);
  PlatformResult status = ContactUtil::ErrorChecker(
      err, "Fail to create website record in database.");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&website_record_h, ContactsDeleter);

  status = ContactUtil::SetStrInRecord(website_record_h, _contacts_url.url,
                                       url.c_str());
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int type_to_set =
      (FromJson<JsonString>(in, "type") == kContactWebSiteTypeHomePage)
          ? CONTACTS_URL_TYPE_HOME
          : CONTACTS_URL_TYPE_WORK;

  status = ContactUtil::SetIntInRecord(website_record_h, _contacts_url.type,
                                       type_to_set);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  err = contacts_record_add_child_record(contacts_record, _contacts_contact.url,
                                         website_record_h);
  status = ContactUtil::ErrorChecker(
      err, "Problem during saving WebSite urls into database.");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // Do not delete record, it is passed to the platform
  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactAnniversariesFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject* out_ptr,
    bool* ret) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.event, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    *ret = false;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  int value = 0;
  PlatformResult status =
      ContactUtil::GetIntFromRecord(child_record, _contacts_event.type, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (CONTACTS_EVENT_TYPE_BIRTH == value) {
    *ret = false;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  if (CONTACTS_EVENT_TYPE_ANNIVERSARY == value) {
    status = ContactUtil::GetIntFromRecord(child_record, _contacts_event.date,
                                           &value);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    out.insert(std::make_pair("date", JsonValue{static_cast<double>(value)}));

    char* label = nullptr;
    status = ContactUtil::GetStrFromRecord(child_record, _contacts_event.label,
                                           &label);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    out.insert(std::make_pair("label", label ? JsonValue{label} : JsonValue{}));
  }

  *ret = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactAnniversariesToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  int date = static_cast<int>(FromJson<double>(in, "date"));
  if (date == 0) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  contacts_record_h anniversary_record = nullptr;
  int err = contacts_record_create(_contacts_event._uri, &anniversary_record);
  PlatformResult status = ContactUtil::ErrorChecker(
      err, "Failed to create anniversary record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&anniversary_record, ContactsDeleter);

  status = ContactUtil::SetIntInRecord(anniversary_record, _contacts_event.type,
                                       CONTACTS_EVENT_TYPE_ANNIVERSARY);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  status = ContactUtil::SetIntInRecord(anniversary_record, _contacts_event.date,
                                       date);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!IsNull(in, "label")) {
    status =
        ContactUtil::SetStrInRecord(anniversary_record, _contacts_event.label,
                                    FromJson<JsonString>(in, "label").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.event, anniversary_record);
  status = ContactUtil::ErrorChecker(
      err, "Fail to save anniversary record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // Do not delete record, it is passed to the platform
  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactRelationshipFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.relationship, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  char* relative = nullptr;
  PlatformResult status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_relationship.name, &relative);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!relative) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  out.insert(std::make_pair(std::string("relativeName"),
                            picojson::value(JsonString{relative})));

  int type = 0;
  status = ContactUtil::GetIntFromRecord(child_record,
                                         _contacts_relationship.type, &type);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // TODO Move out.insert outside of switch statement.
  switch (type) {
    case CONTACTS_RELATIONSHIP_TYPE_CUSTOM:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeCustom})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_ASSISTANT:
      out.insert(std::make_pair(std::string("type"),
                                picojson::value(JsonString{kContactRelationshipTypeAssistant})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_BROTHER:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeBrother})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_CHILD:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeChild})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_DOMESTIC_PARTNER:
      out.insert(std::make_pair(
          std::string("type"),
          picojson::value(
              JsonString{kContactRelationshipTypeDomesticPartner})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_FATHER:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeFather})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_FRIEND:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeFriend})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_MANAGER:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeManager})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_MOTHER:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeMother})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_PARENT:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeParent})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_PARTNER:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypePartner})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_REFERRED_BY:
      out.insert(std::make_pair(
          std::string("type"),
          picojson::value(JsonString{kContactRelationshipTypeReferredBy})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_RELATIVE:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeRelative})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_SISTER:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeSister})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_SPOUSE:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeSpouse})));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_OTHER:
    default:
      out.insert(
          std::make_pair(std::string("type"),
                         picojson::value(JsonString{kContactRelationshipTypeOther})));
      break;
  }

  char* label = nullptr;
  status = ContactUtil::GetStrFromRecord(child_record,
                                         _contacts_relationship.label, &label);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair(std::string("label"),
                     picojson::value(label ? JsonValue{label} : JsonValue{})));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactRelationshipToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_create(_contacts_relationship._uri, &child_record);
  PlatformResult status =
      ContactUtil::ErrorChecker(err, "Fail to create child_record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&child_record, ContactsDeleter);

  status = ContactUtil::SetStrInRecord(
      child_record, _contacts_relationship.name,
      FromJson<JsonString>(in, "relativeName").c_str());
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  const JsonString& type = FromJson<JsonString>(in, "type");
  int type_to_set;
  if (type == kContactRelationshipTypeAssistant) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_ASSISTANT;
  } else if (type == kContactRelationshipTypeBrother) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_BROTHER;
  } else if (type == kContactRelationshipTypeChild) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_CHILD;
  } else if (type == kContactRelationshipTypeDomesticPartner) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_DOMESTIC_PARTNER;
  } else if (type == kContactRelationshipTypeFather) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_FATHER;
  } else if (type == kContactRelationshipTypeFriend) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_FRIEND;
  } else if (type == kContactRelationshipTypeManager) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_MANAGER;
  } else if (type == kContactRelationshipTypeMother) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_MOTHER;
  } else if (type == kContactRelationshipTypeParent) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_PARENT;
  } else if (type == kContactRelationshipTypePartner) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_PARTNER;
  } else if (type == kContactRelationshipTypeReferredBy) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_REFERRED_BY;
  } else if (type == kContactRelationshipTypeRelative) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_RELATIVE;
  } else if (type == kContactRelationshipTypeSister) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_SISTER;
  } else if (type == kContactRelationshipTypeSpouse) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_SPOUSE;
  } else if (type == kContactRelationshipTypeCustom) {
    type_to_set = CONTACTS_RELATIONSHIP_TYPE_CUSTOM;
  } else {
    type_to_set = CONTACTS_MESSENGER_TYPE_OTHER;
  }

  status = ContactUtil::SetIntInRecord(
      child_record, _contacts_relationship.type, type_to_set);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!IsNull(in, "label")) {
    status =
        ContactUtil::SetStrInRecord(child_record, _contacts_relationship.label,
                                    FromJson<JsonString>(in, "label").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.relationship, child_record);
  status = ContactUtil::ErrorChecker(
      err, "Fail to set number value to child_record");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactInstantMessengerFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.messenger, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    LoggerW("Skipping message with index %i. error code: %i", index, err);
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  char* im_address = nullptr;
  PlatformResult status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_messenger.im_id, &im_address);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!im_address) {
    LoggerW("Skipping message with index %i. missing im address", index);
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  out.insert(std::make_pair("imAddress", JsonValue{im_address}));

  int type = 0;
  status = ContactUtil::GetIntFromRecord(child_record, _contacts_messenger.type,
                                         &type);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // TODO Move out.insert outside of switch statement.
  switch (type) {
    case CONTACTS_MESSENGER_TYPE_CUSTOM:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeCustom}));
      break;
    case CONTACTS_MESSENGER_TYPE_GOOGLE:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeGoogle}));
      break;
    case CONTACTS_MESSENGER_TYPE_WLM:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeWlm}));
      break;
    case CONTACTS_MESSENGER_TYPE_YAHOO:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeYahoo}));
      break;
    case CONTACTS_MESSENGER_TYPE_FACEBOOK:
      out.insert(std::make_pair("type",
                                JsonValue{kContactInstantMessageTypeFacebook}));
      break;
    case CONTACTS_MESSENGER_TYPE_ICQ:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeIcq}));
      break;
    case CONTACTS_MESSENGER_TYPE_AIM:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeAim}));
      break;
    case CONTACTS_MESSENGER_TYPE_QQ:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeQq}));
      break;
    case CONTACTS_MESSENGER_TYPE_JABBER:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeJabber}));
      break;
    case CONTACTS_MESSENGER_TYPE_SKYPE:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeSkype}));
      break;
    case CONTACTS_MESSENGER_TYPE_IRC:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeIrc}));
      break;
    case CONTACTS_MESSENGER_TYPE_OTHER:
    default:
      out.insert(
          std::make_pair("type", JsonValue{kContactInstantMessageTypeOther}));
      break;
  }

  char* label = nullptr;
  status = ContactUtil::GetStrFromRecord(child_record,
                                         _contacts_messenger.label, &label);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("label", label ? JsonValue{label} : JsonValue{}));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactInstantMessengerToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_create(_contacts_messenger._uri, &child_record);
  PlatformResult status =
      ContactUtil::ErrorChecker(err, "Fail to create child_record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&child_record, ContactsDeleter);

  status = ContactUtil::SetStrInRecord(
      child_record, _contacts_messenger.im_id,
      FromJson<JsonString>(in, "imAddress").c_str());
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int type_to_set = 0;
  const JsonString& type = FromJson<JsonString>(in, "type");
  if (type == kContactInstantMessageTypeGoogle) {
    type_to_set = CONTACTS_MESSENGER_TYPE_GOOGLE;
  } else if (type == kContactInstantMessageTypeWlm) {
    type_to_set = CONTACTS_MESSENGER_TYPE_WLM;
  } else if (type == kContactInstantMessageTypeYahoo) {
    type_to_set = CONTACTS_MESSENGER_TYPE_YAHOO;
  } else if (type == kContactInstantMessageTypeFacebook) {
    type_to_set = CONTACTS_MESSENGER_TYPE_FACEBOOK;
  } else if (type == kContactInstantMessageTypeIcq) {
    type_to_set = CONTACTS_MESSENGER_TYPE_ICQ;
  } else if (type == kContactInstantMessageTypeAim) {
    type_to_set = CONTACTS_MESSENGER_TYPE_AIM;
  } else if (type == kContactInstantMessageTypeQq) {
    type_to_set = CONTACTS_MESSENGER_TYPE_QQ;
  } else if (type == kContactInstantMessageTypeJabber) {
    type_to_set = CONTACTS_MESSENGER_TYPE_JABBER;
  } else if (type == kContactInstantMessageTypeSkype) {
    type_to_set = CONTACTS_MESSENGER_TYPE_SKYPE;
  } else if (type == kContactInstantMessageTypeIrc) {
    type_to_set = CONTACTS_MESSENGER_TYPE_IRC;
  } else if (type == kContactInstantMessageTypeCustom) {
    type_to_set = CONTACTS_MESSENGER_TYPE_CUSTOM;
  } else {
    type_to_set = CONTACTS_MESSENGER_TYPE_OTHER;
  }

  status = ContactUtil::SetIntInRecord(child_record, _contacts_messenger.type,
                                       type_to_set);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!IsNull(in, "label")) {
    status =
        ContactUtil::SetStrInRecord(child_record, _contacts_messenger.label,
                                    FromJson<JsonString>(in, "label").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.messenger, child_record);
  status = ContactUtil::ErrorChecker(
      err, "Fail to set number value to child_record");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactAddressFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h child_record = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.address, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  char* value = nullptr;
  PlatformResult status = ContactUtil::GetStrFromRecord(
      child_record, _contacts_address.country, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("country", value ? JsonValue{value} : JsonValue{}));
  status = ContactUtil::GetStrFromRecord(child_record, _contacts_address.region,
                                         &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("region", value ? JsonValue{value} : JsonValue{}));
  status = ContactUtil::GetStrFromRecord(child_record,
                                         _contacts_address.locality, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("city", value ? JsonValue{value} : JsonValue{}));
  status = ContactUtil::GetStrFromRecord(child_record, _contacts_address.street,
                                         &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair("streetAddress", value ? JsonValue{value} : JsonValue{}));
  status = ContactUtil::GetStrFromRecord(child_record,
                                         _contacts_address.extended, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("additionalInformation",
                            value ? JsonValue{value} : JsonValue{}));
  status = ContactUtil::GetStrFromRecord(child_record,
                                         _contacts_address.postal_code, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair("postalCode", value ? JsonValue{value} : JsonValue{}));
  status = ContactUtil::GetStrFromRecord(child_record, _contacts_address.label,
                                         &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("label", value ? JsonValue{value} : JsonValue{}));

  bool bool_value = false;
  status = ContactUtil::GetBoolFromRecord(
      child_record, _contacts_address.is_default, &bool_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("isDefault", JsonValue{bool_value}));

  int int_value = 0;
  status = ContactUtil::GetIntFromRecord(child_record, _contacts_address.type,
                                         &int_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  JsonArray types;
  if (int_value & CONTACTS_ADDRESS_TYPE_HOME) {
    types.push_back(JsonValue{kContactAddressTypeHome});
  }
  if (int_value & CONTACTS_ADDRESS_TYPE_WORK) {
    types.push_back(JsonValue{kContactAddressTypeWork});
  }
  if (CONTACTS_ADDRESS_TYPE_OTHER == int_value) {
    types.push_back(JsonValue{kContactAddressTypeOther});
  }
  if (int_value & CONTACTS_ADDRESS_TYPE_CUSTOM) {
    types.push_back(JsonValue{kContactAddressTypeCustom});
  }

  if (types.empty()) {
    types.push_back(JsonValue{kContactAddressTypeHome});
  }
  out.insert(std::make_pair("types", picojson::value(types)));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactAddressToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {

  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h address_record = nullptr;
  int err = contacts_record_create(_contacts_address._uri, &address_record);
  PlatformResult status = ContactUtil::ErrorChecker(
      err, "Failed to create address record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&address_record, ContactsDeleter);

  if (!IsNull(in, "country")) {
    status = ContactUtil::SetStrInRecord(
        address_record, _contacts_address.country,
        FromJson<JsonString>(in, "country").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "region")) {
    status =
        ContactUtil::SetStrInRecord(address_record, _contacts_address.region,
                                    FromJson<JsonString>(in, "region").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "city")) {
    status =
        ContactUtil::SetStrInRecord(address_record, _contacts_address.locality,
                                    FromJson<JsonString>(in, "city").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "streetAddress")) {
    status = ContactUtil::SetStrInRecord(
        address_record, _contacts_address.street,
        FromJson<JsonString>(in, "streetAddress").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "additionalInformation")) {
    status = ContactUtil::SetStrInRecord(
        address_record, _contacts_address.extended,
        FromJson<JsonString>(in, "additionalInformation").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "postalCode")) {
    status = ContactUtil::SetStrInRecord(
        address_record, _contacts_address.postal_code,
        FromJson<JsonString>(in, "postalCode").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(in, "label")) {
    status =
        ContactUtil::SetStrInRecord(address_record, _contacts_address.label,
                                    FromJson<JsonString>(in, "label").c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  status =
      ContactUtil::SetBoolInRecord(address_record, _contacts_address.is_default,
                                   FromJson<bool>(in, "isDefault"));
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  int type_to_set = 0;
  const JsonArray& types = FromJson<JsonArray>(in, "types");
  for (auto& type : types) {
    auto& type_str = JsonCast<JsonString>(type);
    if (type_str == kContactAddressTypeWork) {
      type_to_set |= CONTACTS_ADDRESS_TYPE_WORK;
    } else if (type_str == kContactAddressTypeOther) {
      type_to_set |= CONTACTS_ADDRESS_TYPE_OTHER;
    } else if (type_str == kContactAddressTypeCustom) {
      type_to_set |= CONTACTS_ADDRESS_TYPE_CUSTOM;
    } else {
      type_to_set |= CONTACTS_ADDRESS_TYPE_HOME;
    }
  }

  status = ContactUtil::SetIntInRecord(address_record, _contacts_address.type,
                                       type_to_set);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.address, address_record);
  status =
      ContactUtil::ErrorChecker(err, "Fail to save address record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // Do not delete record, it is passed to the platform
  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactNotesFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonValue* val) {

  LoggerD("Enter");
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  contacts_record_h notes_record = nullptr;
  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.note, index, &notes_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  char* note = nullptr;
  PlatformResult status =
      ContactUtil::GetStrFromRecord(notes_record, _contacts_note.note, &note);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (note) {
    *val = JsonValue{note};
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportNotesToContactsRecord(contacts_record_h contacts_record,
                                           const std::string& value) {

  LoggerD("Enter");
  contacts_record_h notes_record = nullptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  int err = contacts_record_create(_contacts_note._uri, &notes_record);
  PlatformResult status =
      ContactUtil::ErrorChecker(err, "Fail to create note record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  ContactsRecordHPtr record(&notes_record, ContactsDeleter);

  status = ContactUtil::SetStrInRecord(notes_record, _contacts_note.note,
                                       value.c_str());
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  err = contacts_record_add_child_record(contacts_record,
                                         _contacts_contact.note, notes_record);
  status =
      ContactUtil::ErrorChecker(err, "Fail to save note record in database");
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  // Do not delete record, it is passed to the platform
  record.release();

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactFromContactsRecord(
    contacts_record_h contacts_record, JsonObject* out_ptr) {

  LoggerD("Enter");
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  int id = 0;
  PlatformResult status =
      ContactUtil::GetIntFromRecord(contacts_record, _contacts_contact.id, &id);
  if (status.IsError())
  {
    LoggerE("Fail: GetIntFromRecord");
    return status;
  }
  out.insert(std::make_pair("id", JsonValue{std::to_string(id)}));
  status = ContactUtil::GetIntFromRecord(
      contacts_record, _contacts_contact.address_book_id, &id);
  if (status.IsError())
  {
    LoggerE("Fail: GetIntFromRecord");
    return status;
  }
  out.insert(std::make_pair("addressBookId", JsonValue{std::to_string(id)}));
  status = ContactUtil::GetIntFromRecord(contacts_record,
                                         _contacts_contact.person_id, &id);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("personId", JsonValue{std::to_string(id)}));

  bool is_favorite = false;
  status = ContactUtil::GetBoolFromRecord(
      contacts_record, _contacts_contact.is_favorite, &is_favorite);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("isFavorite", JsonValue{is_favorite}));

  int last_update = 0;
  status = ContactUtil::GetIntFromRecord(
      contacts_record, _contacts_contact.changed_time, &last_update);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("lastUpdated",
                            JsonValue{static_cast<double>(last_update)}));

  //### ContactName: ###
  JsonObject name;
  bool is_contact_name;
  status = ImportContactNameFromContactsRecord(contacts_record, &name,
                                               &is_contact_name);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (is_contact_name) {
    out.insert(std::make_pair(std::string("name"), picojson::value(name)));
  } else {
    out.insert(std::make_pair(std::string("name"),
                              picojson::value(JsonValue{})));
  }

  typedef PlatformResult (*ImportFunc)(contacts_record_h, unsigned int,
      JsonObject*);
  struct ImportData {
    const char* name;
    unsigned int property_id;
    ImportFunc import_func;
  };
  ImportData imports[] = {
      {"emails", _contacts_contact.email,
          ImportContactEmailAddressFromContactsRecord},
      {"phoneNumbers", _contacts_contact.number,
          ImportContactPhoneNumberFromContactsRecord},
      {"organizations", _contacts_contact.company,
          ImportContactOrganizationFromContactsRecord},
      {"urls", _contacts_contact.url, ImportContactWebSiteFromContactsRecord},
      {"addresses", _contacts_contact.address,
          ImportContactAddressFromContactsRecord},
      {"messengers", _contacts_contact.messenger,
          ImportContactInstantMessengerFromContactsRecord},
      {"relationships", _contacts_contact.relationship,
          ImportContactRelationshipFromContactsRecord},
  };

  for (auto& data : imports) {
    JsonArray& array = out.insert(std::make_pair(data.name,
        picojson::value(JsonArray()))).first->second.get<JsonArray>();

    int child_rec_count;
    status = ContactUtil::GetNumberOfChildRecord(
        contacts_record, data.property_id, &child_rec_count);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    for (int i = 0; i < child_rec_count; ++i) {
      JsonValue val{JsonObject{}};
      data.import_func(contacts_record, static_cast<unsigned int>(i), &val.get<JsonObject>());
      array.push_back(val);
    }
  }

  //### ContactAnniversary: ###
  JsonArray& anniversaries =
      out.insert(std::make_pair(std::string("anniversaries"),
                                picojson::value(JsonArray()))).first->second.get<JsonArray>();

  int child_rec_count;
  status = ContactUtil::GetNumberOfChildRecord(
      contacts_record, _contacts_contact.event, &child_rec_count);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  bool is_contact_anniv;
  for (int i = 0; i < child_rec_count; ++i) {
    JsonValue anniversary{JsonObject{}};

    PlatformResult status = ImportContactAnniversariesFromContactsRecord(
        contacts_record, static_cast<unsigned int>(i), &anniversary.get<JsonObject>(), &is_contact_anniv);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    if (is_contact_anniv) {
      anniversaries.push_back(anniversary);
    } else {
      out.insert(std::make_pair(std::string("birthday"),
                                picojson::value(anniversaries)));
    }
  }

  //### m_notes: ###
  JsonArray& notes = out.insert(std::make_pair(std::string("notes"),
      picojson::value(JsonArray()))).first->second.get<JsonArray>();

  status = ContactUtil::GetNumberOfChildRecord(
      contacts_record, _contacts_contact.note, &child_rec_count);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  for (int i = 0; i < child_rec_count; ++i) {
    JsonValue val{JsonObject{}};

    status = ImportContactNotesFromContactsRecord(contacts_record, static_cast<unsigned int>(i), &val);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    notes.push_back(val);
  }

  //### m_photo_uri ###
  //### m_ringtone_uri ###
  {
    char* value = nullptr;

    status = ContactUtil::GetStrFromRecord(
        contacts_record, _contacts_contact.image_thumbnail_path, &value);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out.insert(
        std::make_pair(
            "photoURI",
            value ? JsonValue{ConvertPathToUri(value)} : JsonValue{}));
    value = nullptr;

    status = ContactUtil::GetStrFromRecord(
        contacts_record, _contacts_contact.ringtone_path, &value);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out.insert(
        std::make_pair(
            "ringtoneURI",
            value ? JsonValue{ConvertPathToUri(value)} : JsonValue{}));
    value = nullptr;

    status = ContactUtil::GetStrFromRecord(
        contacts_record, _contacts_contact.message_alert, &value);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out.insert(
        std::make_pair(
            "messageAlertURI",
            value ? JsonValue{ConvertPathToUri(value)} : JsonValue{}));
    value = nullptr;

    status = ContactUtil::GetStrFromRecord(contacts_record,
                                           _contacts_contact.vibration, &value);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    out.insert(
        std::make_pair(
            "vibrationURI",
            value ? JsonValue{ConvertPathToUri(value)} : JsonValue{}));
    value = nullptr;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactToContactsRecord(contacts_record_h contacts_record,
                                             const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerW("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  //### ContactName: ###
  if (!IsNull(in, "name")) {
    PlatformResult status = ExportContactNameToContactsRecord(
        contacts_record, FromJson<JsonObject>(in, "name"));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  typedef PlatformResult (*ExportFunc)(contacts_record_h, const JsonObject&);
  struct ExportDataHelper {
    unsigned int property_id;
    const char* name;
    ExportFunc export_func;
  };
  const ExportDataHelper exports[] = {
      {_contacts_contact.email, "emails",
          ExportContactEmailAddressToContactsRecord},
      {_contacts_contact.number, "phoneNumbers",
          ExportContactPhoneNumberToContactsRecord},
      {_contacts_contact.company, "organizations",
          ExportContactOrganizationToContactsRecord},
      {_contacts_contact.url, "urls", ExportContactWebSiteToContactsRecord},
      {_contacts_contact.event, "anniversaries",
          ExportContactAnniversariesToContactsRecord},
      {_contacts_contact.address, "addresses",
          ExportContactAddressToContactsRecord},
      {_contacts_contact.messenger, "messengers",
          ExportContactInstantMessengerToContactsRecord},
      {_contacts_contact.relationship, "relationships",
          ExportContactRelationshipToContactsRecord},
  };

  for (auto& data : exports) {
    PlatformResult status =
        ContactUtil::ClearAllContactRecord(contacts_record, data.property_id);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    const JsonArray& elements = FromJson<JsonArray>(in, data.name);
    for (auto& element : elements) {
      data.export_func(contacts_record, JsonCast<JsonObject>(element));
    }
  }

  {
    //### m_notes: ###
    PlatformResult status = ContactUtil::ClearAllContactRecord(
        contacts_record, _contacts_contact.note);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    const JsonArray& elements = FromJson<JsonArray>(in, "notes");
    for (auto& element : elements) {
      auto& str = JsonCast<JsonString>(element);
      if (str.empty()) {
        continue;
      }
      ExportNotesToContactsRecord(contacts_record, str);
    }
  }

  // Contact.photoURI
  if (IsNull(in, "photoURI")) {
    contacts_record_h child_record = nullptr;
    int err = contacts_record_create(_contacts_image._uri, &child_record);
    PlatformResult status = ContactUtil::ErrorChecker(
        err, "Fail to create image uri record in database.");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    ContactsRecordHPtr record(&child_record, ContactsDeleter);
    err = contacts_record_add_child_record(
        contacts_record, _contacts_contact.image, child_record);
    status = ContactUtil::ErrorChecker(err, "Fail to add child to image uri.");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }

    // Do not delete record, it is passed to the platform
    record.release();
  } else {
    bool is_first = false;
    contacts_record_h child_record = nullptr;
    int err = contacts_record_get_child_record_at_p(
        contacts_record, _contacts_contact.image, 0, &child_record);
    if (CONTACTS_ERROR_NONE != err || nullptr == child_record) {
      err = contacts_record_create(_contacts_image._uri, &child_record);
      PlatformResult status = ContactUtil::ErrorChecker(
          err, "Fail to create image uri record in database.");
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }

      is_first = true;
    }
    ContactsRecordHPtr record(&child_record, ContactsDeleter);
    // TODO this was never used anywhere in the old module. Can this be removed?
    // char *old_value_str = nullptr;
    // ContactUtil::getStrFromRecord(child_record, _contacts_image.path,
    // &old_value_str);

    std::string real_path;
    if (!IsNull(in, "photoURI")) {
      real_path =
          ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "photoURI"));
      PlatformResult status = ContactUtil::SetStrInRecord(
          child_record, _contacts_image.path, real_path.c_str());
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }

    if (is_first) {
      err = contacts_record_add_child_record(
          contacts_record, _contacts_contact.image, child_record);
      PlatformResult status =
          ContactUtil::ErrorChecker(err, "Fail to add child to image uri.");
      if (status.IsError()) {
        LoggerE("Error: %s", status.message().c_str());
        return status;
      }
    }
    // Do not delete record, it is passed to the platform
    record.release();
  }

  std::string real_path;
  // Contact.ringtoneURI
  if (!IsNull(in, "ringtoneURI")) {
    real_path =
        ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "ringtoneURI"));
    PlatformResult status = VerifyLocalPath(real_path);
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
    status = ContactUtil::SetStrInRecord(
        contacts_record, _contacts_contact.ringtone_path, real_path.c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  // Contact.messageAlertURI
  if (!IsNull(in, "messageAlertURI")) {
    real_path = ContactUtil::ConvertUriToPath(
        FromJson<JsonString>(in, "messageAlertURI"));
    PlatformResult status = ContactUtil::SetStrInRecord(
        contacts_record, _contacts_contact.message_alert, real_path.c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  // Contact.vibrationURI
  if (!IsNull(in, "vibrationURI")) {
    real_path =
        ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "vibrationURI"));
    PlatformResult status = ContactUtil::SetStrInRecord(
        contacts_record, _contacts_contact.vibration, real_path.c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ImportContactGroupFromContactsRecord(
    contacts_record_h contacts_record, JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Contacts record is null");
  }

  // id
  int int_val = 0;
  PlatformResult status = ContactUtil::GetIntFromRecord(
      contacts_record, _contacts_group.id, &int_val);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("id", JsonValue{std::to_string(int_val)}));

  // addressBookId
  status = ContactUtil::GetIntFromRecord(
      contacts_record, _contacts_group.address_book_id, &int_val);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair("addressBookId", JsonValue{std::to_string(int_val)}));

  // name
  char* value = nullptr;
  status = ContactUtil::GetStrFromRecord(contacts_record, _contacts_group.name,
                                         &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("name", value ? JsonValue{value} : JsonValue{}));

  // photoURI
  value = nullptr;
  status = ContactUtil::GetStrFromRecord(contacts_record,
                                         _contacts_group.image_path, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair(
          "photoURI",
          value ? JsonValue{ConvertPathToUri(value)} : JsonValue{}));

  // ringtoneURI
  value = nullptr;
  status = ContactUtil::GetStrFromRecord(contacts_record,
                                         _contacts_group.ringtone_path, &value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair(
          "ringtoneURI",
          value ? JsonValue{ConvertPathToUri(value)} : JsonValue{}));

  // is_read_only
  bool bool_value = false;
  status = ContactUtil::GetBoolFromRecord(
      contacts_record, _contacts_group.is_read_only, &bool_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("readOnly", JsonValue{bool_value}));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ExportContactGroupToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {
  // name
  PlatformResult status =
      ContactUtil::SetStrInRecord(contacts_record, _contacts_group.name,
                                  FromJson<JsonString>(in, "name").c_str());
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  std::string real_path;
  // photoURI
  if (!IsNull(in, "photoURI")) {
    real_path = ConvertUriToPath(FromJson<JsonString>(in, "photoURI"));
    PlatformResult status = ContactUtil::SetStrInRecord(
        contacts_record, _contacts_group.image_path, real_path.c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  // ringtoneURI
  if (!IsNull(in, "ringtoneURI")) {
    real_path =
        ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "ringtoneURI"));
    // NOTE in the original code real path was not read
    PlatformResult status = ContactUtil::SetStrInRecord(
        contacts_record, _contacts_group.ringtone_path, real_path.c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

/**
 * @brief   Fills Person object with values from record
 * @param[in]   contacts_record_h  Record which is used to fill Person
 */
PlatformResult ImportPersonFromContactsRecord(contacts_record_h record,
                                              JsonObject* out_ptr) {
  if (nullptr == record) {
    LoggerW("Platform person record did not set");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Platform person record did not set");
  }

  JsonObject& arguments_obj = *out_ptr;

  int int_value = 0;
  // id
  PlatformResult status =
      ContactUtil::GetIntFromRecord(record, _contacts_person.id, &int_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(
      std::make_pair("id", JsonValue(std::to_string(int_value))));

  char* char_value = nullptr;
  // displayName
  status = ContactUtil::GetStrFromRecord(record, _contacts_person.display_name,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(std::make_pair(
      "displayName", char_value ? JsonValue(char_value) : JsonValue{}));

  // contactCount
  status = ContactUtil::GetIntFromRecord(record, _contacts_person.link_count,
                                         &int_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(std::make_pair(
      "contactCount", JsonValue(static_cast<double>(int_value))));

  bool bool_value = false;
  // hasPhoneNumber
  status = ContactUtil::GetBoolFromRecord(
      record, _contacts_person.has_phonenumber, &bool_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(std::make_pair("hasPhoneNumber", JsonValue(bool_value)));

  // hasEmail
  status = ContactUtil::GetBoolFromRecord(record, _contacts_person.has_email,
                                          &bool_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(std::make_pair("hasEmail", JsonValue(bool_value)));

  // isFavorite
  status = ContactUtil::GetBoolFromRecord(record, _contacts_person.is_favorite,
                                          &bool_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(std::make_pair("isFavorite", JsonValue(bool_value)));

  // photoURI
  status = ContactUtil::GetStrFromRecord(
      record, _contacts_person.image_thumbnail_path, &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(
      std::make_pair(
          "photoURI",
          char_value ? JsonValue(ConvertPathToUri(char_value)) : JsonValue{}));

  // ringtoneURI
  status = ContactUtil::GetStrFromRecord(record, _contacts_person.ringtone_path,
                                         &char_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(
      std::make_pair(
          "ringtoneURI",
          char_value ? JsonValue(ConvertPathToUri(char_value)) : JsonValue{}));

  // displayContactId
  status = ContactUtil::GetIntFromRecord(
      record, _contacts_person.display_contact_id, &int_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  arguments_obj.insert(
      std::make_pair("displayContactId",
                     picojson::value(std::to_string(int_value))));

  return PlatformResult(ErrorCode::NO_ERROR);
}

/**
 * @brief   Updates contacts_record_h with values from Person object
 * @param[out]   contacts_record_h  Record which is updated
 */
PlatformResult ExportPersonToContactsRecord(contacts_record_h record,
                                            const JsonObject& args) {
  if (nullptr == record) {
    LoggerE("Platform person object did not set");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Platform person object did not set");
  }

  PlatformResult status = ContactUtil::SetBoolInRecord(
      record, _contacts_person.is_favorite, FromJson<bool>(args, "isFavorite"));
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  if (!IsNull(args, "photoURI") &&
      !FromJson<JsonString>(args, "photoURI").empty()) {
    PlatformResult status = ContactUtil::SetStrInRecord(
        record, _contacts_person.image_thumbnail_path,
        ConvertUriToPath(FromJson<JsonString>(args, "photoURI")).c_str());
    if (status.IsError()) {
      LoggerE("Try updating read only attribute photoURI");
      return status;
    }
  } else {
    // TO DO: fix when photoURI attribute changed from read only to write mode
  }

  if (!IsNull(args, "ringtoneURI")) {
    PlatformResult status = ContactUtil::SetStrInRecord(
        record, _contacts_person.ringtone_path,
        ConvertUriToPath(FromJson<JsonString>(args, "ringtoneURI")).c_str());
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  } else {
    PlatformResult status =
        ContactUtil::SetStrInRecord(record, _contacts_person.ringtone_path, "");
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }
  if (!IsNull(args, "displayContactId")) {
    PlatformResult status = ContactUtil::SetIntInRecord(
        record, _contacts_person.display_contact_id,
        common::stol(FromJson<JsonString>(args, "displayContactId")));
    if (status.IsError()) {
      LoggerE("Error: %s", status.message().c_str());
      return status;
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult UpdateAdditionalInformation(
    const ContactsRecordHPtr& contacts_record_ptr, JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  int int_value = -1;
  PlatformResult status = ContactUtil::GetIntFromRecord(
      *contacts_record_ptr, _contacts_contact.person_id, &int_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("personId", JsonValue{std::to_string(int_value)}));
  status = ContactUtil::GetIntFromRecord(
      *contacts_record_ptr, _contacts_contact.address_book_id, &int_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair("addressBookId", JsonValue{std::to_string(int_value)}));
  status = ContactUtil::GetIntFromRecord(
      *contacts_record_ptr, _contacts_contact.changed_time, &int_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(
      std::make_pair("lastUpdated", JsonValue{static_cast<double>(int_value)}));
  bool bool_value = false;
  status = ContactUtil::GetBoolFromRecord(
      *contacts_record_ptr, _contacts_contact.is_favorite, &bool_value);
  if (status.IsError()) {
    LoggerE("Error: %s", status.message().c_str());
    return status;
  }

  out.insert(std::make_pair("isFavorite", JsonValue{bool_value}));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult CheckDBConnection() {
  static bool _connected = false;
  if (_connected) return PlatformResult(ErrorCode::NO_ERROR);

  int err = contacts_connect();
  if (CONTACTS_ERROR_NONE == err) {
    LoggerI("Connection established!");
    _connected = true;
  } else {
    LoggerE("DB connection error occured: %s", std::to_string(err).c_str());
    return PlatformResult(
        ErrorCode::UNKNOWN_ERR,
        "DB connection error occured: " + std::to_string(err));
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // ContactUtil
}  // contact
}  // extension
