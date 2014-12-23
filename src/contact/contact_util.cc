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

#include "contact/contact_util.h"
#include <algorithm>
#include <iomanip>
#include <string>
#include "common/converter.h"
#include "common/logger.h"

namespace extension {
namespace contact {
namespace ContactUtil {

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
}

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

using namespace common;

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

void ErrorChecker(int err, const char* message) {
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("%s, error code: %i", message, err);
    throw common::UnknownException(message);
  }
}

void GetStrFromRecord(contacts_record_h record, unsigned int property_id,
                      char** value) {
  int err = contacts_record_get_str_p(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    throw common::UnknownException("Error during getting contact record");
  }
}

void GetIntFromRecord(contacts_record_h record, unsigned int property_id,
                      int* value) {
  int err = contacts_record_get_int(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    throw common::UnknownException("Error during getting contact record");
  }
}

void GetBoolFromRecord(contacts_record_h record, unsigned int property_id,
                       bool* value) {
  int err = contacts_record_get_bool(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    throw common::UnknownException("Error during getting contact record");
  }
}

void SetStrInRecord(contacts_record_h record, unsigned int property_id,
                    const char* value) {
  int err = contacts_record_set_str(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    throw common::UnknownException("Error during setting contact record");
  }
}

void SetIntInRecord(contacts_record_h record, unsigned int property_id,
                    int value) {
  int err = contacts_record_set_int(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    throw common::UnknownException("Error during setting contact record");
  }
}

void SetBoolInRecord(contacts_record_h record, unsigned int property_id,
                     bool value) {
  int err = contacts_record_set_bool(record, property_id, value);
  if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Error during getting contact record, error code: %i", err);
    throw common::UnknownException("Error during setting contact record");
  }
}

void ClearAllContactRecord(contacts_record_h contacts_record,
                           unsigned int property_id) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  unsigned int record_count =
      GetNumberOfChildRecord(contacts_record, property_id);

  for (unsigned int i = 0; i < record_count; ++i) {
    unsigned int actual_index = record_count - 1 - i;
    contacts_record_h phone_record = nullptr;
    int err = contacts_record_get_child_record_at_p(
        contacts_record, property_id, actual_index, &phone_record);
    ContactUtil::ErrorChecker(err, "Error during getting phone record");

    err = contacts_record_remove_child_record(contacts_record, property_id,
                                              phone_record);
    ContactUtil::ErrorChecker(err, "Error during getting phone record");
  }
}

unsigned int GetNumberOfChildRecord(contacts_record_h contacts_record,
                                    unsigned int property_id) {
  int err = CONTACTS_ERROR_NONE;
  int child_count = 0;
  err = contacts_record_get_child_record_count(contacts_record, property_id,
                                               &child_count);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    throw common::UnknownException("Problem during getting child count");
  }

  return child_count;
}

JsonValue ImportBirthdayFromContactsRecord(contacts_record_h contacts_record,
                                           unsigned int index) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.event, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return {};
  }

  int value = 0;
  ContactUtil::GetIntFromRecord(child_record, _contacts_event.type, &value);

  if (CONTACTS_EVENT_TYPE_BIRTH == value) {
    int date = 0;
    ContactUtil::GetIntFromRecord(child_record, _contacts_event.date, &date);
    return JsonValue{static_cast<double>(date)};
  }
  return {};
}

void ExportBirthdayToContactsRecord(contacts_record_h contacts_record,
                                    int date) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h birthday_record = nullptr;
  err = contacts_record_create(_contacts_event._uri, &birthday_record);
  ContactUtil::ErrorChecker(err,
                            "Failed to create birthday record in database");
  ContactsRecordHPtr record(&birthday_record, ContactsDeleter);

  ContactUtil::SetIntInRecord(birthday_record, _contacts_event.type,
                              CONTACTS_EVENT_TYPE_BIRTH);

  ContactUtil::SetIntInRecord(birthday_record, _contacts_event.date, date);

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.event, birthday_record);
  ContactUtil::ErrorChecker(err, "Fail to save birthday record in database");
  // Do not delete record, it is passed to the platform
  record.release();
}

bool ImportContactNameFromContactsRecord(contacts_record_h contacts_record,
                                         JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  if (!contacts_record) {
    LoggerW("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }
  int count = 0;
  int err = CONTACTS_ERROR_NONE;
  err = contacts_record_get_child_record_count(contacts_record,
                                               _contacts_contact.name, &count);
  ContactUtil::ErrorChecker(err, "Contacts child record get count error");

  if (count > 1) {
    LoggerE("More than one ContactName for one Contact");
    throw common::UnknownException("More than one ContactName for one Contact");
  }
  LoggerD("Contact name record count: %i", count);

  if (count == 0) {
    return false;
  }

  contacts_record_h contact_name = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.name, 0, &contact_name);
  ContactUtil::ErrorChecker(err, "Contacts name record get childerror");

  // Documentation says:
  //  child_record MUST NOT be released by you.
  //  It is released when the parent record handle is destroyed.
  // so it won't be protected by unique_ptr.

  char* char_value = nullptr;
  ContactUtil::GetStrFromRecord(contact_name, _contacts_name.prefix,
                                &char_value);
  out.insert(std::make_pair("prefix",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(contact_name, _contacts_name.suffix,
                                &char_value);

  out.insert(std::make_pair("suffix",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(contact_name, _contacts_name.first,
                                &char_value);

  out.insert(std::make_pair("firstName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(contact_name, _contacts_name.addition,
                                &char_value);

  out.insert(std::make_pair("middleName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(contact_name, _contacts_name.last, &char_value);

  out.insert(std::make_pair("lastName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(contact_name, _contacts_name.phonetic_first,
                                &char_value);

  out.insert(std::make_pair("phoneticFirstName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(contact_name, _contacts_name.phonetic_middle,
                                &char_value);

  out.insert(std::make_pair("phoneticMiddleName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(contact_name, _contacts_name.phonetic_last,
                                &char_value);

  out.insert(std::make_pair("phoneticLastName",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  err = contacts_record_get_child_record_count(
      contacts_record, _contacts_contact.nickname, &count);
  ContactUtil::ErrorChecker(err, "Contacts child record get count error");

  JsonArray& nicknames = out.insert(std::make_pair("nicknames", JsonArray()))
                             .first->second.get<JsonArray>();
  for (unsigned int i = 0; i < count; ++i) {
    contacts_record_h nickname = nullptr;
    err = contacts_record_get_child_record_at_p(
        contacts_record, _contacts_contact.nickname, i, &nickname);
    ContactUtil::ErrorChecker(err, "Contacts nicknames record get child error");
    ContactUtil::GetStrFromRecord(nickname, _contacts_nickname.name,
                                  &char_value);

    if (char_value) {
      nicknames.push_back(JsonValue{char_value});
    }
  }

  return true;
}

void ExportContactNameToContactsRecord(contacts_record_h contacts_record,
                                       const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerW("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h contact_name = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.name, 0, &contact_name);
  bool update = true;
  if (CONTACTS_ERROR_NONE != err && nullptr == contact_name) {
    err = contacts_record_create(_contacts_name._uri, &contact_name);
    ContactUtil::ErrorChecker(err, "Contacts record create error");
    update = false;
  }
  // contact_name starts to be protected by unique_ptr
  ContactsRecordHPtr contacts_name_ptr(&contact_name, ContactsDeleter);

  if (!IsNull(in, "prefix")) {
    ContactUtil::SetStrInRecord(*contacts_name_ptr, _contacts_name.prefix,
                                FromJson<JsonString>(in, "prefix").c_str());
  }
  if (!IsNull(in, "suffix")) {
    ContactUtil::SetStrInRecord(*contacts_name_ptr, _contacts_name.suffix,
                                FromJson<JsonString>(in, "suffix").c_str());
  }
  if (!IsNull(in, "firstName")) {
    ContactUtil::SetStrInRecord(*contacts_name_ptr, _contacts_name.first,
                                FromJson<JsonString>(in, "firstName").c_str());
  }
  if (!IsNull(in, "middleName")) {
    ContactUtil::SetStrInRecord(*contacts_name_ptr, _contacts_name.addition,
                                FromJson<JsonString>(in, "middleName").c_str());
  }
  if (!IsNull(in, "lastName")) {
    ContactUtil::SetStrInRecord(*contacts_name_ptr, _contacts_name.last,
                                FromJson<JsonString>(in, "lastName").c_str());
  }
  if (!IsNull(in, "phoneticFirstName")) {
    ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.phonetic_first,
        FromJson<JsonString>(in, "phoneticFirstName").c_str());
  }
  if (!IsNull(in, "phoneticMiddleName")) {
    ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.phonetic_middle,
        FromJson<JsonString>(in, "phoneticMiddleName").c_str());
  }
  if (!IsNull(in, "phoneticLastName")) {
    ContactUtil::SetStrInRecord(
        *contacts_name_ptr, _contacts_name.phonetic_last,
        FromJson<JsonString>(in, "phoneticLastName").c_str());
  }

  // contact_name is being added as a child to contacts_record
  // and in future will be destroyed by contacts_record's contactsDeleter
  if (!update) {
    err = contacts_record_add_child_record(
        contacts_record, _contacts_contact.name, *contacts_name_ptr);
    ContactUtil::ErrorChecker(err, "Contacts record add child error");
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
      ContactUtil::ErrorChecker(err, "Contacts record create error");
      update = false;
    }
    ContactsRecordHPtr nickname_ptr(&nickname_record, ContactsDeleter);

    ContactUtil::SetStrInRecord(*nickname_ptr, _contacts_nickname.name,
                                JsonCast<JsonString>(nickname).c_str());
    if (!update) {
      err = contacts_record_add_child_record(
          contacts_record, _contacts_contact.nickname, *nickname_ptr);
      ContactUtil::ErrorChecker(err, "Contacts record add child error");
    }
    // Do not delete record, it is passed to the platform
    nickname_ptr.release();
  }

  // TODO update displayName in JS!
}

void ImportContactEmailAddressFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.email, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return;
  }

  char* email = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_email.email, &email);
  if (!email) {
    return;
  }
  out.insert(std::make_pair("email", JsonValue{email}));

  bool is_default = false;
  ContactUtil::GetBoolFromRecord(child_record, _contacts_email.is_default,
                                 &is_default);
  out.insert(std::make_pair("isDefault", JsonValue{is_default}));

  char* label = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_email.label, &label);
  out.insert(std::make_pair("label", label ? JsonValue{label} : JsonValue{}));

  int type = 0;
  ContactUtil::GetIntFromRecord(child_record, _contacts_email.type, &type);

  JsonArray types;
  if (type & CONTACTS_EMAIL_TYPE_HOME) {
    types.push_back(JsonValue{kContactEmailAddressTypeHome});
  }
  if (type & CONTACTS_EMAIL_TYPE_WORK) {
    types.push_back(JsonValue{kContactEmailAddressTypeWork});
  }
  if (type & CONTACTS_EMAIL_TYPE_OTHER) {
    types.push_back(JsonValue{kContactEmailAddressTypeOther});
  }
  if (type & CONTACTS_EMAIL_TYPE_CUSTOM) {
    types.push_back(JsonValue{kContactEmailAddressTypeCustom});
  }

  if (0 == types.size()) {
    types.push_back(JsonValue{kContactEmailAddressTypeHome});
  }

  out.insert(std::make_pair("types", JsonValue{types}));
}

void ExportContactEmailAddressToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {
  contacts_record_h c_email_record_h = nullptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  err = contacts_record_create(_contacts_email._uri, &c_email_record_h);
  ContactUtil::ErrorChecker(err, "Failed to create email record in database");
  ContactsRecordHPtr record(&c_email_record_h, ContactsDeleter);

  ContactUtil::SetStrInRecord(c_email_record_h, _contacts_email.email,
                              FromJson<JsonString>(in, "email").c_str());
  ContactUtil::SetBoolInRecord(c_email_record_h, _contacts_email.is_default,
                               FromJson<bool>(in, "isDefault"));
  if (!IsNull(in, "label")) {
    ContactUtil::SetStrInRecord(c_email_record_h, _contacts_email.label,
                                FromJson<JsonString>(in, "label").c_str());
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
  ContactUtil::SetIntInRecord(c_email_record_h, _contacts_email.type,
                              type_to_set);

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.email, c_email_record_h);
  ContactUtil::ErrorChecker(err, "Fail to save email record into database");
  // Do not delete record, it is passed to the platform
  record.release();
}

void ImportContactPhoneNumberFromContactsRecord(contacts_record_h contacts_record,
                                                unsigned int index, JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  contacts_record_h child_record = nullptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.number, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return;
  }

  char* phone = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_number.number, &phone);

  out.insert(std::make_pair("number", JsonValue{phone}));

  bool is_default = false;
  ContactUtil::GetBoolFromRecord(child_record, _contacts_number.is_default,
                                 &is_default);
  out.insert(std::make_pair("isDefault", JsonValue{is_default}));
  int type = 0;
  ContactUtil::GetIntFromRecord(child_record, _contacts_number.type, &type);

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
  if (type & CONTACTS_NUMBER_TYPE_OTHER) {
    types.push_back(JsonValue{kContactPhoneTypeOther});
  }
  if (type & CONTACTS_NUMBER_TYPE_CUSTOM) {
    types.push_back(JsonValue{kContactPhoneTypeCustom});
  }
  if (0 == types.size()) {
    types.push_back(JsonValue{kContactPhoneTypeVoice});
  }
  out.insert(std::make_pair("types", types));

  char* label = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_number.label, &label);
  out.insert(std::make_pair("label", label ? JsonValue{label} : JsonValue{}));
}

void ExportContactPhoneNumberToContactsRecord(contacts_record_h contacts_record,
                                              const JsonObject& in) {
  contacts_record_h phone_record = nullptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = contacts_record_create(_contacts_number._uri, &phone_record);
  ContactUtil::ErrorChecker(err, "Fail to create phone_record in database");
  ContactsRecordHPtr record(&phone_record, ContactsDeleter);

  ContactUtil::SetStrInRecord(phone_record, _contacts_number.number,
                              FromJson<JsonString>(in, "number").c_str());

  ContactUtil::SetBoolInRecord(phone_record, _contacts_number.is_default,
                               FromJson<bool>(in, "isDefault"));
  if (!IsNull(in, "label")) {
    ContactUtil::SetStrInRecord(phone_record, _contacts_address.label,
                                FromJson<JsonString>(in, "label").c_str());
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

  ContactUtil::SetIntInRecord(phone_record, _contacts_number.type, type_to_set);

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.number, phone_record);
  ContactUtil::ErrorChecker(err, "Fail to set number value to phone_record");
  // Do not delete record, it is passed to the platform
  record.release();
}

void ImportContactOrganizationFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.company, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return;
  }

  char* char_value = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_company.name,
                                &char_value);
  out.insert(
      std::make_pair("name", char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(child_record, _contacts_company.department,
                                &char_value);
  out.insert(std::make_pair("department",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(child_record, _contacts_company.job_title,
                                &char_value);
  out.insert(std::make_pair("title",
                            char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(child_record, _contacts_company.role,
                                &char_value);
  out.insert(
      std::make_pair("role", char_value ? JsonValue{char_value} : JsonValue{}));

  ContactUtil::GetStrFromRecord(child_record, _contacts_company.logo,
                                &char_value);
  out.insert(std::make_pair("logoURI",
                            char_value ? JsonValue{char_value} : JsonValue{}));
}

void ExportContactOrganizationToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  contacts_record_h organization_record = nullptr;
  int err = CONTACTS_ERROR_NONE;
  err = contacts_record_create(_contacts_company._uri, &organization_record);
  ContactUtil::ErrorChecker(err,
                            "Failed to create organization record in database");
  ContactsRecordHPtr record(&organization_record, ContactsDeleter);

  if (!IsNull(in, "name")) {
    ContactUtil::SetStrInRecord(organization_record, _contacts_company.name,
                                FromJson<JsonString>(in, "name").c_str());
  }
  if (!IsNull(in, "department")) {
    ContactUtil::SetStrInRecord(organization_record,
                                _contacts_company.department,
                                FromJson<JsonString>(in, "department").c_str());
  }
  if (!IsNull(in, "title")) {
    ContactUtil::SetStrInRecord(organization_record,
                                _contacts_company.job_title,
                                FromJson<JsonString>(in, "title").c_str());
  }
  if (!IsNull(in, "role")) {
    ContactUtil::SetStrInRecord(organization_record, _contacts_company.role,
                                FromJson<JsonString>(in, "role").c_str());
  }
  if (!IsNull(in, "logoURI")) {
    std::string path =
        ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "logoURI"));
    ContactUtil::SetStrInRecord(organization_record, _contacts_company.logo,
                                path.c_str());
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.company, organization_record);
  ContactUtil::ErrorChecker(err, "Fail to set company value to child_record");
  // Do not delete record, it is passed to the platform
  record.release();
}

void ImportContactWebSiteFromContactsRecord(contacts_record_h contacts_record,
                                            unsigned int index,
                                            JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.url, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return;
  }

  char* char_value = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_url.url, &char_value);
  out.insert(std::make_pair("logoURI", char_value ? char_value : ""));

  int type = 0;
  ContactUtil::GetIntFromRecord(child_record, _contacts_url.type, &type);

  out.insert(std::make_pair("logoURI", (CONTACTS_URL_TYPE_HOME == type)
                                           ? kContactWebSiteTypeHomePage
                                           : kContactWebSiteTypeBlog));
}

void ExportContactWebSiteToContactsRecord(contacts_record_h contacts_record,
                                          const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  if (IsNull(in, "url")) {
    LoggerD("WebSite urls are not set");
    return;
  }

  const std::string& url = FromJson<JsonString>(in, "url");
  if (url.empty()) {
    LoggerD("WebSite urls are not set");
    return;
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h website_record_h = nullptr;
  err = contacts_record_create(_contacts_url._uri, &website_record_h);
  ContactUtil::ErrorChecker(err, "Fail to create website record in database.");
  ContactsRecordHPtr record(&website_record_h, ContactsDeleter);

  ContactUtil::SetStrInRecord(website_record_h, _contacts_url.url, url.c_str());

  int type_to_set =
      (FromJson<JsonString>(in, "type") == kContactWebSiteTypeHomePage)
          ? CONTACTS_URL_TYPE_HOME
          : CONTACTS_URL_TYPE_WORK;

  ContactUtil::SetIntInRecord(website_record_h, _contacts_url.type,
                              type_to_set);

  err = contacts_record_add_child_record(contacts_record, _contacts_contact.url,
                                         website_record_h);
  ContactUtil::ErrorChecker(
      err, "Problem during saving WebSite urls into database.");
  // Do not delete record, it is passed to the platform
  record.release();
}

bool ImportContactAnniversariesFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.event, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return false;
  }

  int value = 0;
  ContactUtil::GetIntFromRecord(child_record, _contacts_event.type, &value);

  if (CONTACTS_EVENT_TYPE_BIRTH == value) {
    return false;
  }

  if (CONTACTS_EVENT_TYPE_ANNIVERSARY == value) {
    ContactUtil::GetIntFromRecord(child_record, _contacts_event.date, &value);
    out.insert(std::make_pair("date", JsonValue{static_cast<double>(value)}));

    char* label = nullptr;
    ContactUtil::GetStrFromRecord(child_record, _contacts_event.label, &label);

    out.insert(std::make_pair("label", label ? JsonValue{label} : JsonValue{}));
  }
  return true;
}

void ExportContactAnniversariesToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int date = static_cast<int>(FromJson<double>(in, "date"));
  if (date == 0) {
    return;
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h anniversary_record = nullptr;
  err = contacts_record_create(_contacts_event._uri, &anniversary_record);
  ContactUtil::ErrorChecker(err,
                            "Failed to create anniversary record in database");
  ContactsRecordHPtr record(&anniversary_record, ContactsDeleter);

  ContactUtil::SetIntInRecord(anniversary_record, _contacts_event.type,
                              CONTACTS_EVENT_TYPE_ANNIVERSARY);

  ContactUtil::SetIntInRecord(anniversary_record, _contacts_event.date, date);

  if (!IsNull(in, "label")) {
    ContactUtil::SetStrInRecord(anniversary_record, _contacts_event.label,
                                FromJson<JsonString>(in, "label").c_str());
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.event, anniversary_record);
  ContactUtil::ErrorChecker(err, "Fail to save anniversary record in database");
  // Do not delete record, it is passed to the platform
  record.release();
}

void ImportContactRelationshipFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.relationship, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return;
  }

  char* relative = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_relationship.name,
                                &relative);
  if (!relative) {
    return;
  }

  out.insert(std::make_pair("relativeName", JsonString{relative}));

  int type = 0;
  ContactUtil::GetIntFromRecord(child_record, _contacts_relationship.type,
                                &type);

  // TODO Move out.insert outside of switch statement.
  switch (type) {
    case CONTACTS_RELATIONSHIP_TYPE_CUSTOM:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeCustom}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_ASSISTANT:
      out.insert(std::make_pair("type",
                                JsonString{kContactRelationshipTypeAssistant}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_BROTHER:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeBrother}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_CHILD:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeChild}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_DOMESTIC_PARTNER:
      out.insert(std::make_pair(
          "type", JsonString{kContactRelationshipTypeDomesticPartner}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_FATHER:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeFather}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_FRIEND:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeFriend}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_MANAGER:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeManager}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_MOTHER:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeMother}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_PARENT:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeParent}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_PARTNER:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypePartner}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_REFERRED_BY:
      out.insert(std::make_pair(
          "type", JsonString{kContactRelationshipTypeReferredBy}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_RELATIVE:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeRelative}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_SISTER:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeSister}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_SPOUSE:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeSpouse}));
      break;
    case CONTACTS_RELATIONSHIP_TYPE_OTHER:
    default:
      out.insert(
          std::make_pair("type", JsonString{kContactRelationshipTypeOther}));
      break;
  }

  char* label = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_relationship.label,
                                &label);
  out.insert(std::make_pair("label", label ? JsonValue{label} : JsonValue{}));
}

void ExportContactRelationshipToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_create(_contacts_relationship._uri, &child_record);
  ContactUtil::ErrorChecker(err, "Fail to create child_record in database");
  ContactsRecordHPtr record(&child_record, ContactsDeleter);

  ContactUtil::SetStrInRecord(child_record, _contacts_relationship.name,
                              FromJson<JsonString>(in, "relativeName").c_str());

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

  ContactUtil::SetIntInRecord(child_record, _contacts_relationship.type,
                              type_to_set);

  if (!IsNull(in, "label")) {
    ContactUtil::SetStrInRecord(child_record, _contacts_relationship.label,
                                FromJson<JsonString>(in, "label").c_str());
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.relationship, child_record);
  ContactUtil::ErrorChecker(err, "Fail to set number value to child_record");
  record.release();
}

void ImportContactInstantMessengerFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index,
    JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.messenger, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    LoggerW("Skipping message with index %i. error code: %i", index, err);
    return;
  }

  char* im_address = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_messenger.im_id,
                                &im_address);
  if (!im_address) {
    LoggerW("Skipping message with index %i. missing im address", index);
    return;
  }

  out.insert(std::make_pair("imAddress", JsonValue{im_address}));

  int type = 0;
  ContactUtil::GetIntFromRecord(child_record, _contacts_messenger.type, &type);

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
  ContactUtil::GetStrFromRecord(child_record, _contacts_messenger.label,
                                &label);
  out.insert(std::make_pair("label", label ? JsonValue{label} : JsonValue{}));
}

void ExportContactInstantMessengerToContactsRecord(
    contacts_record_h contacts_record, const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_create(_contacts_messenger._uri, &child_record);
  ContactUtil::ErrorChecker(err, "Fail to create child_record in database");
  ContactsRecordHPtr record(&child_record, ContactsDeleter);

  ContactUtil::SetStrInRecord(child_record, _contacts_messenger.im_id,
                              FromJson<JsonString>(in, "imAddress").c_str());

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

  ContactUtil::SetIntInRecord(child_record, _contacts_messenger.type,
                              type_to_set);

  if (!IsNull(in, "label")) {
    ContactUtil::SetStrInRecord(child_record, _contacts_messenger.label,
                                FromJson<JsonString>(in, "label").c_str());
  }

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.messenger, child_record);
  ContactUtil::ErrorChecker(err, "Fail to set number value to child_record");
  record.release();
}

void ImportContactAddressFromContactsRecord(contacts_record_h contacts_record,
                                            unsigned int index,
                                            JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h child_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.address, index, &child_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return;
  }

  char* value = nullptr;
  ContactUtil::GetStrFromRecord(child_record, _contacts_address.country,
                                &value);
  out.insert(std::make_pair("country", value ? JsonValue{value} : JsonValue{}));
  ContactUtil::GetStrFromRecord(child_record, _contacts_address.region, &value);
  out.insert(std::make_pair("region", value ? JsonValue{value} : JsonValue{}));
  ContactUtil::GetStrFromRecord(child_record, _contacts_address.locality,
                                &value);
  out.insert(std::make_pair("city", value ? JsonValue{value} : JsonValue{}));
  ContactUtil::GetStrFromRecord(child_record, _contacts_address.street, &value);
  out.insert(
      std::make_pair("streetAddress", value ? JsonValue{value} : JsonValue{}));
  ContactUtil::GetStrFromRecord(child_record, _contacts_address.extended,
                                &value);
  out.insert(std::make_pair("additionalInformation",
                            value ? JsonValue{value} : JsonValue{}));
  ContactUtil::GetStrFromRecord(child_record, _contacts_address.postal_code,
                                &value);
  out.insert(
      std::make_pair("postalCode", value ? JsonValue{value} : JsonValue{}));
  ContactUtil::GetStrFromRecord(child_record, _contacts_address.label, &value);
  out.insert(std::make_pair("label", value ? JsonValue{value} : JsonValue{}));

  bool bool_value = false;
  ContactUtil::GetBoolFromRecord(child_record, _contacts_address.is_default,
                                 &bool_value);
  out.insert(std::make_pair("isDefault", JsonValue{bool_value}));

  int int_value = 0;
  ContactUtil::GetIntFromRecord(child_record, _contacts_address.type,
                                &int_value);

  JsonArray types;
  if (int_value & CONTACTS_ADDRESS_TYPE_HOME) {
    types.push_back(JsonValue{kContactAddressTypeHome});
  }
  if (int_value & CONTACTS_ADDRESS_TYPE_WORK) {
    types.push_back(JsonValue{kContactAddressTypeWork});
  }
  if (int_value & CONTACTS_ADDRESS_TYPE_OTHER) {
    types.push_back(JsonValue{kContactAddressTypeOther});
  }
  if (int_value & CONTACTS_ADDRESS_TYPE_CUSTOM) {
    types.push_back(JsonValue{kContactAddressTypeCustom});
  }

  if (0 == types.size()) {
    types.push_back(JsonValue{kContactAddressTypeHome});
  }
  out.insert(std::make_pair("types", types));
}

void ExportContactAddressToContactsRecord(contacts_record_h contacts_record,
                                          const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h address_record = nullptr;
  err = contacts_record_create(_contacts_address._uri, &address_record);
  ContactUtil::ErrorChecker(err, "Failed to create address record in database");
  ContactsRecordHPtr record(&address_record, ContactsDeleter);

  if (!IsNull(in, "country")) {
    ContactUtil::SetStrInRecord(address_record, _contacts_address.country,
                                FromJson<JsonString>(in, "country").c_str());
  }
  if (!IsNull(in, "region")) {
    ContactUtil::SetStrInRecord(address_record, _contacts_address.region,
                                FromJson<JsonString>(in, "region").c_str());
  }
  if (!IsNull(in, "city")) {
    ContactUtil::SetStrInRecord(address_record, _contacts_address.locality,
                                FromJson<JsonString>(in, "city").c_str());
  }
  if (!IsNull(in, "streetAddress")) {
    ContactUtil::SetStrInRecord(
        address_record, _contacts_address.street,
        FromJson<JsonString>(in, "streetAddress").c_str());
  }
  if (!IsNull(in, "additionalInformation")) {
    ContactUtil::SetStrInRecord(
        address_record, _contacts_address.extended,
        FromJson<JsonString>(in, "additionalInformation").c_str());
  }
  if (!IsNull(in, "postalCode")) {
    ContactUtil::SetStrInRecord(address_record, _contacts_address.postal_code,
                                FromJson<JsonString>(in, "postalCode").c_str());
  }
  if (!IsNull(in, "label")) {
    ContactUtil::SetStrInRecord(address_record, _contacts_address.label,
                                FromJson<JsonString>(in, "label").c_str());
  }

  ContactUtil::SetBoolInRecord(address_record, _contacts_address.is_default,
                               FromJson<bool>(in, "isDefault"));

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

  ContactUtil::SetIntInRecord(address_record, _contacts_address.type,
                              type_to_set);

  err = contacts_record_add_child_record(
      contacts_record, _contacts_contact.address, address_record);
  ContactUtil::ErrorChecker(err, "Fail to save address record in database");
  // Do not delete record, it is passed to the platform
  record.release();
}

JsonValue ImportContactNotesFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  contacts_record_h notes_record = nullptr;
  err = contacts_record_get_child_record_at_p(
      contacts_record, _contacts_contact.note, index, &notes_record);
  if (CONTACTS_ERROR_NONE != err && CONTACTS_ERROR_NO_DATA != err) {
    return {};
  }

  char* note = nullptr;
  ContactUtil::GetStrFromRecord(notes_record, _contacts_note.note, &note);

  if (note) {
    return JsonValue{note};
  }
  return {};
}

void ExportNotesToContactsRecord(contacts_record_h contacts_record,
                                 const std::string& value) {
  contacts_record_h notes_record = nullptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int err = CONTACTS_ERROR_NONE;
  err = contacts_record_create(_contacts_note._uri, &notes_record);
  ContactUtil::ErrorChecker(err, "Fail to create note record in database");
  ContactsRecordHPtr record(&notes_record, ContactsDeleter);

  ContactUtil::SetStrInRecord(notes_record, _contacts_note.note, value.c_str());

  err = contacts_record_add_child_record(contacts_record,
                                         _contacts_contact.note, notes_record);
  ContactUtil::ErrorChecker(err, "Fail to save note record in database");

  // Do not delete record, it is passed to the platform
  record.release();
}

void ImportContactFromContactsRecord(contacts_record_h contacts_record,
                                     JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerW("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  int id = 0;
  ContactUtil::GetIntFromRecord(contacts_record, _contacts_contact.id, &id);
  out.insert(std::make_pair("id", JsonValue{std::to_string(id)}));
  ContactUtil::GetIntFromRecord(contacts_record,
                                _contacts_contact.address_book_id, &id);
  out.insert(std::make_pair("addressBookId", JsonValue{std::to_string(id)}));
  ContactUtil::GetIntFromRecord(contacts_record, _contacts_contact.person_id,
                                &id);
  out.insert(std::make_pair("personId", JsonValue{std::to_string(id)}));

  bool is_favorite = false;
  ContactUtil::GetBoolFromRecord(contacts_record, _contacts_contact.is_favorite,
                                 &is_favorite);
  out.insert(std::make_pair("isFavorite", JsonValue{is_favorite}));

  int last_update = 0;
  ContactUtil::GetIntFromRecord(contacts_record, _contacts_contact.changed_time,
                                &last_update);
  out.insert(std::make_pair("lastUpdated",
                            JsonValue{static_cast<double>(last_update)}));

  //### ContactName: ###
  JsonObject name;
  if (ImportContactNameFromContactsRecord(contacts_record, &name)) {
    out.insert(std::make_pair("name", name));
  } else {
    out.insert(std::make_pair("name", JsonValue{}));
  }

  typedef void (*ImportFunc)(contacts_record_h, unsigned int, JsonObject*);
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
       ImportContactRelationshipFromContactsRecord}, };

  for (auto& data : imports) {
    JsonArray& array = out.insert(std::make_pair(data.name, JsonArray()))
                           .first->second.get<JsonArray>();

    for (unsigned int i = 0, n = ContactUtil::GetNumberOfChildRecord(
                                 contacts_record, data.property_id);
         i < n; ++i) {
      JsonValue val{JsonObject{}};
      data.import_func(contacts_record, i, &val.get<JsonObject>());
      array.push_back(val);
    }
  }

  //### ContactAnniversary: ###
  JsonArray& anniversaries =
      out.insert(std::make_pair("anniversaries", JsonArray()))
          .first->second.get<JsonArray>();
  for (unsigned int i = 0, n = ContactUtil::GetNumberOfChildRecord(
                               contacts_record, _contacts_contact.event);
       i < n; ++i) {
    JsonValue anniversary{JsonObject{}};
    if (ImportContactAnniversariesFromContactsRecord(
            contacts_record, i, &anniversary.get<JsonObject>())) {
      anniversaries.push_back(anniversary);
    } else {
      out.insert(std::make_pair("birthday", anniversaries));
    }
  }

  //### m_notes: ###
  JsonArray& notes = out.insert(std::make_pair("notes", JsonArray()))
                         .first->second.get<JsonArray>();
  for (unsigned int i = 0, n = ContactUtil::GetNumberOfChildRecord(
                               contacts_record, _contacts_contact.note);
       i < n; ++i) {
    notes.push_back(ImportContactNotesFromContactsRecord(contacts_record, i));
  }

  //### m_photo_uri ###
  //### m_ringtone_uri ###
  {
    char* value = nullptr;
    ContactUtil::GetStrFromRecord(
        contacts_record, _contacts_contact.image_thumbnail_path, &value);

    std::make_pair("photoURI", value ? JsonValue{value} : JsonValue{});

    ContactUtil::GetStrFromRecord(contacts_record,
                                  _contacts_contact.ringtone_path, &value);
    std::make_pair("ringtoneURI", value ? JsonValue{value} : JsonValue{});
    value = nullptr;
    ContactUtil::GetStrFromRecord(contacts_record,
                                  _contacts_contact.message_alert, &value);
    out.insert(std::make_pair(
        "messageAlertURI",
        value ? JsonValue{ConvertPathToUri(value)} : JsonValue{}));
    value = nullptr;
    ContactUtil::GetStrFromRecord(contacts_record, _contacts_contact.vibration,
                                  &value);
    out.insert(std::make_pair(
        "vibrationURI",
        value ? JsonValue{ConvertPathToUri(value)} : JsonValue{}));
  }
}

void ExportContactToContactsRecord(contacts_record_h contacts_record,
                                   const JsonObject& in) {
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerW("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  //### ContactName: ###
  if (!IsNull(in, "name")) {
    ExportContactNameToContactsRecord(contacts_record,
                                      FromJson<JsonObject>(in, "name"));
  }

  typedef void (*ExportFunc)(contacts_record_h, const JsonObject&);
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
       ExportContactRelationshipToContactsRecord}, };

  for (auto& data : exports) {
    ContactUtil::ClearAllContactRecord(contacts_record, data.property_id);
    const JsonArray& elements = FromJson<JsonArray>(in, data.name);
    for (auto& element : elements) {
      data.export_func(contacts_record, JsonCast<JsonObject>(element));
    }
  }

  {
    //### m_notes: ###
    ContactUtil::ClearAllContactRecord(contacts_record, _contacts_contact.note);
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
    ContactUtil::ErrorChecker(err,
                              "Fail to create image uri record in database.");
    ContactsRecordHPtr record(&child_record, ContactsDeleter);
    err = contacts_record_add_child_record(
        contacts_record, _contacts_contact.image, child_record);
    ContactUtil::ErrorChecker(err, "Fail to add child to image uri.");
    // Do not delete record, it is passed to the platform
    record.release();
  } else {
    bool is_first = false;
    contacts_record_h child_record = nullptr;
    int err = contacts_record_get_child_record_at_p(
        contacts_record, _contacts_contact.image, 0, &child_record);
    if (CONTACTS_ERROR_NONE != err || nullptr == child_record) {
      err = contacts_record_create(_contacts_image._uri, &child_record);
      ContactUtil::ErrorChecker(err,
                                "Fail to create image uri record in database.");
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
      ContactUtil::SetStrInRecord(child_record, _contacts_image.path,
                                  real_path.c_str());
    }

    if (is_first) {
      err = contacts_record_add_child_record(
          contacts_record, _contacts_contact.image, child_record);
      ContactUtil::ErrorChecker(err, "Fail to add child to image uri.");
    }
    // Do not delete record, it is passed to the platform
    record.release();
  }

  std::string real_path;
  // Contact.ringtoneURI
  if (!IsNull(in, "ringtoneURI")) {
    real_path =
        ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "ringtoneURI"));
    ContactUtil::SetStrInRecord(
        contacts_record, _contacts_contact.ringtone_path, real_path.c_str());
  }
  // Contact.messageAlertURI
  if (!IsNull(in, "messageAlertURI")) {
    real_path = ContactUtil::ConvertUriToPath(
        FromJson<JsonString>(in, "messageAlertURI"));
    ContactUtil::SetStrInRecord(
        contacts_record, _contacts_contact.message_alert, real_path.c_str());
  }

  // Contact.vibrationURI
  if (!IsNull(in, "vibrationURI")) {
    real_path =
        ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "vibrationURI"));
    ContactUtil::SetStrInRecord(contacts_record, _contacts_contact.vibration,
                                real_path.c_str());
  }
}

void ImportContactGroupFromContactsRecord(contacts_record_h contacts_record,
                                          JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  // contacts_record is protected by unique_ptr and its ownership is not passed
  // here
  if (!contacts_record) {
    LoggerE("Contacts record is null");
    throw common::UnknownException("Contacts record is null");
  }

  // id
  int int_val = 0;
  ContactUtil::GetIntFromRecord(contacts_record, _contacts_group.id, &int_val);
  out.insert(std::make_pair("id", JsonValue{std::to_string(int_val)}));

  // addressBookId
  ContactUtil::GetIntFromRecord(contacts_record,
                                _contacts_group.address_book_id, &int_val);
  out.insert(
      std::make_pair("addressBookId", JsonValue{std::to_string(int_val)}));

  // name
  char* value = nullptr;
  ContactUtil::GetStrFromRecord(contacts_record, _contacts_group.name, &value);
  out.insert(std::make_pair("name", value ? JsonValue{value} : JsonValue{}));

  // photoURI
  value = nullptr;
  ContactUtil::GetStrFromRecord(contacts_record, _contacts_group.image_path,
                                &value);
  out.insert(
      std::make_pair("photoURI", value ? JsonValue{value} : JsonValue{}));

  // ringtoneURI
  value = nullptr;
  ContactUtil::GetStrFromRecord(contacts_record, _contacts_group.ringtone_path,
                                &value);
  out.insert(
      std::make_pair("ringtoneURI", value ? JsonValue{value} : JsonValue{}));

  // is_read_only
  bool bool_value = false;
  ContactUtil::GetBoolFromRecord(contacts_record, _contacts_group.is_read_only,
                                 &bool_value);
  out.insert(std::make_pair("readOnly", JsonValue{bool_value}));
}

void ExportContactGroupToContactsRecord(contacts_record_h contacts_record,
                                        const JsonObject& in) {
  // name
  ContactUtil::SetStrInRecord(contacts_record, _contacts_group.name,
                              FromJson<JsonString>(in, "name").c_str());

  std::string real_path;
  // photoURI
  if (!IsNull(in, "photoURI")) {
    real_path = ConvertUriToPath(FromJson<JsonString>(in, "photoURI"));
    ContactUtil::SetStrInRecord(contacts_record, _contacts_group.image_path,
                                real_path.c_str());
  }
  // ringtoneURI
  if (!IsNull(in, "ringtoneURI")) {
    real_path =
        ContactUtil::ConvertUriToPath(FromJson<JsonString>(in, "ringtoneURI"));
    // NOTE in the original code real path was not read
    ContactUtil::SetStrInRecord(contacts_record, _contacts_group.ringtone_path,
                                real_path.c_str());
  }
}

/**
 * @brief   Fills Person object with values from record
 * @param[in]   contacts_record_h  Record which is used to fill Person
 */
void ImportPersonFromContactsRecord(contacts_record_h record,
                                    JsonObject* out_ptr) {
  if (nullptr == record) {
    LoggerW("Platform person record did not set");
    throw InvalidValuesException("Platform person record did not set");
  }

  JsonObject& arguments_obj = *out_ptr;

  int int_value = 0;
  // id
  ContactUtil::GetIntFromRecord(record, _contacts_person.id, &int_value);
  arguments_obj.insert(
      std::make_pair("id", JsonValue(std::to_string(int_value))));

  char* char_value = nullptr;
  // displayName
  ContactUtil::GetStrFromRecord(record, _contacts_person.display_name,
                                &char_value);
  arguments_obj.insert(std::make_pair(
      "displayName", char_value ? JsonValue(char_value) : JsonValue{}));

  // contactCount
  ContactUtil::GetIntFromRecord(record, _contacts_person.link_count,
                                &int_value);
  arguments_obj.insert(std::make_pair(
      "contactCount", JsonValue(static_cast<double>(int_value))));

  bool bool_value = false;
  // hasPhoneNumber
  ContactUtil::GetBoolFromRecord(record, _contacts_person.has_phonenumber,
                                 &bool_value);
  arguments_obj.insert(std::make_pair("hasPhoneNumber", JsonValue(bool_value)));

  // hasEmail
  ContactUtil::GetBoolFromRecord(record, _contacts_person.has_email,
                                 &bool_value);
  arguments_obj.insert(std::make_pair("hasEmail", JsonValue(bool_value)));

  // isFavorite
  ContactUtil::GetBoolFromRecord(record, _contacts_person.is_favorite,
                                 &bool_value);
  arguments_obj.insert(std::make_pair("isFavorite", JsonValue(bool_value)));

  // photoURI
  ContactUtil::GetStrFromRecord(record, _contacts_person.image_thumbnail_path,
                                &char_value);
  arguments_obj.insert(std::make_pair(
      "photoURI", char_value ? JsonValue(char_value) : JsonValue{}));

  // ringtoneURI
  ContactUtil::GetStrFromRecord(record, _contacts_person.ringtone_path,
                                &char_value);
  arguments_obj.insert(std::make_pair(
      "ringtoneURI", char_value ? JsonValue(char_value) : JsonValue{}));

  // displayContactId
  ContactUtil::GetIntFromRecord(record, _contacts_person.display_contact_id,
                                &int_value);
  arguments_obj.insert(
      std::make_pair("displayContactId", std::to_string(int_value)));
}

/**
 * @brief   Updates contacts_record_h with values from Person object
 * @param[out]   contacts_record_h  Record which is updated
 */
void ExportPersonToContactsRecord(contacts_record_h record,
                                  const JsonObject& args) {
  if (nullptr == record) {
    LoggerE("Platform person object did not set");
    throw UnknownException("Platform person object did not set");
  }

  ContactUtil::SetBoolInRecord(record, _contacts_person.is_favorite,
                               FromJson<bool>(args, "isFavorite"));
  try {
    if (!IsNull(args, "photoURI") &&
        !FromJson<JsonString>(args, "photoURI").empty()) {
      ContactUtil::SetStrInRecord(
          record, _contacts_person.image_thumbnail_path,
          FromJson<JsonString>(args, "photoURI").c_str());
    } else {
      ContactUtil::SetStrInRecord(record, _contacts_person.image_thumbnail_path,
                                  "");
    }
  }
  catch (const PlatformException& ex) {
    LoggerD("Platform field is readonly. %s", ex.message().c_str());
  }
  if (!IsNull(args, "ringtoneURI")) {
    ContactUtil::SetStrInRecord(
        record, _contacts_person.ringtone_path,
        FromJson<JsonString>(args, "ringtoneURI").c_str());
  } else {
    ContactUtil::SetStrInRecord(record, _contacts_person.ringtone_path, "");
  }
  if (!IsNull(args, "displayContactId")) {
    ContactUtil::SetIntInRecord(
        record, _contacts_person.display_contact_id,
        common::stol(FromJson<JsonString>(args, "displayContactId")));
  }
}

void UpdateAdditionalInformation(const ContactsRecordHPtr& contacts_record_ptr,
                                 JsonObject* out_ptr) {
  JsonObject& out = *out_ptr;
  int int_value = -1;
  ContactUtil::GetIntFromRecord(*contacts_record_ptr,
                                _contacts_contact.person_id, &int_value);
  out.insert(std::make_pair("personId", JsonValue{std::to_string(int_value)}));
  ContactUtil::GetIntFromRecord(*contacts_record_ptr,
                                _contacts_contact.address_book_id, &int_value);
  out.insert(
      std::make_pair("addressBookId", JsonValue{std::to_string(int_value)}));
  ContactUtil::GetIntFromRecord(*contacts_record_ptr,
                                _contacts_contact.changed_time, &int_value);
  out.insert(
      std::make_pair("lastUpdated", JsonValue{static_cast<double>(int_value)}));
  bool bool_value = false;
  ContactUtil::GetBoolFromRecord(*contacts_record_ptr,
                                 _contacts_contact.is_favorite, &bool_value);
  out.insert(std::make_pair("isFavorite", JsonValue{bool_value}));
}

void CheckDBConnection() {
  static bool _connected = false;
  if (_connected) return;

  int err = contacts_connect();
  if (CONTACTS_ERROR_NONE == err) {
    LoggerI("Connection established!");
    _connected = true;
  } else {
    LoggerE("DB connection error occured: %s", std::to_string(err).c_str());
    throw UnknownException("DB connection error occured: " +
                           std::to_string(err));
  }
}

}  // ContactUtil
}  // contact
}  // extension
