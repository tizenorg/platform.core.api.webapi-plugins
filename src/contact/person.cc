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
#include "contact/person.h"
#include "common/converter.h"
#include "common/platform_exception.h"
#include "common/logger.h"

namespace extension {
namespace contact {

extern const std::string kUsageTypeOutgoingCall  = "OUTGOING_CALL";
extern const std::string kUsageTypeOutgoingMsg   = "OUTGOING_MSG";
extern const std::string kUsageTypeOutgoingEmail = "OUTGOING_EMAIL";
extern const std::string kUsageTypeIncomingCall  = "INCOMING_CALL";
extern const std::string kUsageTypeIncomingMsg   = "INCOMING_MSG";
extern const std::string kUsageTypeIncomingEmail = "INCOMING_EMAIL";
extern const std::string kUsageTypeMissedCall    = "MISSED_CALL";
extern const std::string kUsageTypeRejectedCall  = "REJECTED_CALL";
extern const std::string kUsageTypeBlockedCall   = "BLOCKED_CALL";
extern const std::string kUsageTypeBlockedMsg    = "BLOCKED_MSG";

namespace Person {

using namespace common;

static const PersonPropertyMap personPropertyMap = {
    {"id", {_contacts_person.id, kPrimitiveTypeId}},
    {"displayName", {_contacts_person.display_name, kPrimitiveTypeString}},
    {"contactCount", {_contacts_person.link_count, kPrimitiveTypeLong}},
    {"hasPhoneNumber", {_contacts_person.has_phonenumber, kPrimitiveTypeBoolean}},
    {"hasEmail", {_contacts_person.has_email, kPrimitiveTypeBoolean}},
    {"isFavorite", {_contacts_person.is_favorite, kPrimitiveTypeBoolean}},
    {"photoURI", {_contacts_person.image_thumbnail_path, kPrimitiveTypeString}},
    {"ringtoneURI", {_contacts_person.ringtone_path, kPrimitiveTypeString}},
    {"displayContactId", {_contacts_person.display_contact_id, kPrimitiveTypeId}},
};

extern const std::string kUsageTypeOutgoingCall  = "OUTGOING_CALL";
extern const std::string kUsageTypeOutgoingMsg   = "OUTGOING_MSG";
extern const std::string kUsageTypeOutgoingEmail = "OUTGOING_EMAIL";
extern const std::string kUsageTypeIncomingCall  = "INCOMING_CALL";
extern const std::string kUsageTypeIncomingMsg   = "INCOMING_MSG";
extern const std::string kUsageTypeIncomingEmail = "INCOMING_EMAIL";
extern const std::string kUsageTypeMissedCall    = "MISSED_CALL";
extern const std::string kUsageTypeRejectedCall  = "REJECTED_CALL";
extern const std::string kUsageTypeBlockedCall   = "BLOCKED_CALL";
extern const std::string kUsageTypeBlockedMsg    = "BLOCKED_MSG";

namespace {
  const char* kGetUsageCountArgPersonId  = "personId";
  const char* kGetUsageCountArgUsageType  = "usage_type";

  const char* kGetUsageCountResultUsageCount = "usageCount";
};

PlatformResult PersonLink(const JsonObject& args, JsonObject&) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long id = common::stol(FromJson<JsonString>(args, "id"));
  long person_id = common::stol(FromJson<JsonString>(args, "personId"));

  contacts_record_h contacts_record = nullptr;

  int err = contacts_db_get_record(_contacts_person._uri, id, &contacts_record);
  contacts_record_destroy(contacts_record, true);
  contacts_record = nullptr;

  if (CONTACTS_ERROR_NONE != err) {
    return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR, "Person not found",
                              ("Person was not found, error code: %d", err));
  }

  err = contacts_person_link_person(person_id, id);

  status =
      ContactUtil::ErrorChecker(err, "Error during executing person link()");
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PersonUnlink(const JsonObject& args, JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long contact_id = common::stol(FromJson<JsonString>(args, "id"));

  contacts_record_h contacts_record = nullptr;
  int error_code = contacts_db_get_record(_contacts_simple_contact._uri,
                                          contact_id, &contacts_record);

  if (CONTACTS_ERROR_NONE != error_code) {
    contacts_record_destroy(contacts_record, true);
    contacts_record = nullptr;
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Contact not found",
                          ("Contact not found, error code: %d", error_code));
  }

  int contacts_person_id = 0;
  error_code = contacts_record_get_int(
      contacts_record, _contacts_simple_contact.person_id, &contacts_person_id);
  contacts_record_destroy(contacts_record, true);
  contacts_record = nullptr;

  status = ContactUtil::ErrorChecker(error_code,
                                     "Contact is not a member of person");
  if (status.IsError()) return status;

  long person_id = common::stol(FromJson<JsonString>(args, "personId"));
  if (contacts_person_id != person_id) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR,
                          "Contact is not a member of person",
                          ("Contact is not a member of person (wrong id's)"));
  }

  int new_person_id = 0;

  error_code =
      contacts_person_unlink_contact(person_id, contact_id, &new_person_id);

  status =
      ContactUtil::ErrorChecker(error_code, "Error during executing unlink()");
  if (status.IsError()) return status;

  error_code = contacts_db_get_record(_contacts_person._uri, new_person_id,
                                      &contacts_record);
  if (CONTACTS_ERROR_NONE != error_code) {
    contacts_record_destroy(contacts_record, true);
    contacts_record = nullptr;
    return LogAndCreateResult(ErrorCode::UNKNOWN_ERR, "Person not found",
                          ("Person not found, error code: %d", error_code));
  }

  status = ContactUtil::ImportPersonFromContactsRecord(contacts_record, &out);
  if (status.IsError()) return status;

  contacts_record_destroy(contacts_record, true);
  contacts_record = nullptr;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult GetUsageCount(const JsonObject& args, JsonObject& out) {
  LoggerD("Entered");

  // Retrieve person_id
  long person_id = common::stol(FromJson<JsonString>(args, kGetUsageCountArgPersonId));
  if (person_id < 0) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Negative person id");
  }

  PlatformResult status(ErrorCode::NO_ERROR);

  // Create query
  contacts_query_h query = nullptr;
  int ret = contacts_query_create(_contacts_person_usage._uri, &query);
  status = ContactUtil::ErrorChecker(ret, "Failed contacts_query_create");
  if (!status) {
    return status;
  }
  ContactUtil::ContactsQueryHPtr query_ptr(&query, ContactUtil::ContactsQueryDeleter);

  // Create and set filter on person_id
  contacts_filter_h filter = nullptr;
  ret = contacts_filter_create(_contacts_person_usage._uri, &filter);
  status = ContactUtil::ErrorChecker(ret, "Failed contacts_filter_create");
  if (!status) {
    return status;
  }
  ContactUtil::ContactsFilterPtr filter_ptr(filter, ContactUtil::ContactsFilterDeleter);
  ret = contacts_filter_add_int(filter,
                                _contacts_person_usage.person_id,
                                CONTACTS_MATCH_EQUAL,
                                person_id);
  status = ContactUtil::ErrorChecker(ret, "Failed contacts_filter_add_int");
  if (!status) {
    return status;
  }

  if (!IsNull(args, kGetUsageCountArgUsageType)) {
    contacts_usage_type_e type = UsageTypeFromString(FromJson<JsonString>(args, kGetUsageCountArgUsageType));

    ret = contacts_filter_add_operator(filter, CONTACTS_FILTER_OPERATOR_AND);
    status = ContactUtil::ErrorChecker(ret, "Failed contacts_filter_add_operator");
    if (!status) {
      return status;
    }

    ret = contacts_filter_add_int(filter,
                                  _contacts_person_usage.usage_type,
                                  CONTACTS_MATCH_EQUAL,
                                  type);
    status = ContactUtil::ErrorChecker(ret, "Failed contacts_filter_add_int");
    if (!status) {
      return status;
    }
  }

  // Setting filter
  ret = contacts_query_set_filter(query, filter);
  status = ContactUtil::ErrorChecker(ret, "Failed contacts_query_set_filter");
  if (!status) {
    return status;
  }

  // Call query
  contacts_list_h list = nullptr;
  ret = contacts_db_get_records_with_query(query, 0, 0, &list);
  status = ContactUtil::ErrorChecker(ret, "Failed contacts_db_get_records_with_query");
  if (!status) {
    return status;
  }
  ContactUtil::ContactsListHPtr list_ptr(&list, ContactUtil::ContactsListDeleter);

  int record_count = 0;
  ret = contacts_list_get_count(list, &record_count);
  status = ContactUtil::ErrorChecker(ret, "Failed contacts_list_get_count");
  if (!status) {
    return status;
  }

  int usage_count = 0;
  int usage_count_tmp = 0;
  contacts_record_h record = nullptr;

  contacts_list_first(list);
  for (int i = 0; i < record_count; i++) {
    ret = contacts_list_get_current_record_p(list, &record);
    status = ContactUtil::ErrorChecker(ret, "Failed contacts_list_get_current_record_p");
    if (!status) {
      return status;
    }

    ret = contacts_record_get_int(record, _contacts_person_usage.times_used, &usage_count_tmp );
    status = ContactUtil::ErrorChecker(ret, "Failed contacts_record_get_int");
    if (!status) {
      return status;
    }

    usage_count += usage_count_tmp;

    contacts_list_next(list);
  }

  out.insert(std::make_pair(kGetUsageCountResultUsageCount, JsonValue(static_cast<double>(usage_count))));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ResetUsageCount(const long& person_id, const std::string& type) {
  contacts_usage_type_e type_enum = UsageTypeFromString(type);
  int ret = CONTACTS_ERROR_NONE;

  for (int i = CONTACTS_USAGE_STAT_TYPE_OUTGOING_CALL; i <= CONTACTS_USAGE_STAT_TYPE_BLOCKED_MSG; i++) {
    if (type == "" || type_enum == static_cast<contacts_usage_type_e>(i)) {
      ret = contacts_person_reset_usage(person_id, static_cast<contacts_usage_type_e>(i));
      if (CONTACTS_ERROR_NONE != ret) {
        return LogAndCreateResult(ErrorCode::NOT_FOUND_ERR,
                                  "Error during reset usage count",
                                  ("Error during reset usage count for %s: %d", type.c_str(), ret));
      }
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult PersonResetUsageCount(const JsonObject& args) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long person_id = common::stol(FromJson<JsonString>(args, "personId"));
  if (person_id < 0) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR, "Negative person id");
  }

  if (IsNull(args, "usage_type")) {
    return ResetUsageCount(person_id, std::string(""));
  } else {
    return ResetUsageCount(person_id, FromJson<JsonString>(args, "usage_type"));
  }

}

PlatformResult PersonPropertyFromString(const std::string& name,
                                        PersonProperty* person_prop) {
  LoggerD("Enter");
  auto iter = personPropertyMap.find(name);
  if (iter == personPropertyMap.end()) {
    return LogAndCreateResult(ErrorCode::INVALID_VALUES_ERR,
                          "Invalid property name",
                          ("Invalid property name (not in map): %s", name.c_str()));
  }
  (*person_prop).propertyId = iter->second.propertyId;
  (*person_prop).type = iter->second.type;

  return PlatformResult(ErrorCode::NO_ERROR);
}

contacts_usage_type_e UsageTypeFromString(const std::string& type_str) {
  if (type_str == kUsageTypeOutgoingCall) {
    return CONTACTS_USAGE_STAT_TYPE_OUTGOING_CALL;
  } else if (type_str == kUsageTypeOutgoingMsg) {
    return CONTACTS_USAGE_STAT_TYPE_OUTGOING_MSG;
  } else if (type_str == kUsageTypeOutgoingEmail) {
    return CONTACTS_USAGE_STAT_TYPE_OUTGOING_EMAIL;
  } else if (type_str == kUsageTypeIncomingCall) {
    return CONTACTS_USAGE_STAT_TYPE_INCOMING_CALL;
  } else if (type_str == kUsageTypeIncomingMsg) {
    return CONTACTS_USAGE_STAT_TYPE_INCOMING_MSG;
  } else if (type_str == kUsageTypeIncomingEmail) {
    return CONTACTS_USAGE_STAT_TYPE_INCOMING_EMAIL;
  } else if (type_str == kUsageTypeMissedCall) {
    return CONTACTS_USAGE_STAT_TYPE_MISSED_CALL;
  } else if (type_str == kUsageTypeRejectedCall) {
    return CONTACTS_USAGE_STAT_TYPE_REJECTED_CALL;
  } else if (type_str == kUsageTypeBlockedCall) {
    return CONTACTS_USAGE_STAT_TYPE_BLOCKED_CALL;
  } else if (type_str == kUsageTypeBlockedMsg) {
    return CONTACTS_USAGE_STAT_TYPE_BLOCKED_MSG;
  } else {
    return CONTACTS_USAGE_STAT_TYPE_NONE;
  }
}

}  // Person
}  // contact
}  // extension
