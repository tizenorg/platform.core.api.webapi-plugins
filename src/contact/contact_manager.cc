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

#include "contact/contact_manager.h"
#include <memory>
#include <set>

#include "common/converter.h"
#include "common/filter-utils.h"
#include "common/logger.h"
#include "common/picojson.h"
#include "common/scope_exit.h"

#include <contacts.h>
#include <contacts_db_extension.h>
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

PlatformResult ContactManagerGetAddressBooks(const JsonObject& args,
                                             JsonArray& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  contacts_list_h address_book_list = nullptr;

  int error_code = contacts_db_get_all_records(_contacts_address_book._uri, 0,
                                               0, &address_book_list);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Fail to get address book list, error: %d", error_code);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Fail to get address book list");
  }

  ContactUtil::ContactsListHPtr contacts_list_ptr(
      &address_book_list, ContactUtil::ContactsListDeleter);

  int record_count = 0;
  error_code = contacts_list_get_count(*contacts_list_ptr, &record_count);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Fail to get address book list count, error: %d", error_code);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Fail to get address book list count");
  }

  error_code = contacts_list_first(*contacts_list_ptr);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Fail to get address book from list, error: %d", error_code);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Fail to get address book from list");
  }

  for (int i = 0; i < record_count; i++) {
    contacts_record_h contacts_record = nullptr;
    error_code = contacts_list_get_current_record_p(*contacts_list_ptr,
                                                    &contacts_record);

    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("Fail to get address book record");
      continue;
    }

    int id = 0;
    int account_id = 0;
    int mode = 0;
    char* name = nullptr;
    status = ContactUtil::GetIntFromRecord(contacts_record,
                                           _contacts_address_book.id, &id);
    if (status.IsError()) return status;

    status = ContactUtil::GetIntFromRecord(
        contacts_record, _contacts_address_book.account_id, &account_id);
    if (status.IsError()) return status;

    status = ContactUtil::GetIntFromRecord(contacts_record,
                                           _contacts_address_book.mode, &mode);
    if (status.IsError()) return status;

    status = ContactUtil::GetStrFromRecord(contacts_record,
                                           _contacts_address_book.name, &name);
    if (status.IsError()) return status;

    JsonValue single = JsonValue(JsonObject());
    JsonObject& single_obj = single.get<JsonObject>();

    single_obj["id"] = JsonValue(std::to_string(id));
    single_obj["accountId"] = JsonValue(static_cast<double>(account_id));
    single_obj["name"] = JsonValue(name);
    single_obj["readOnly"] =
        JsonValue(CONTACTS_ADDRESS_BOOK_MODE_READONLY == mode);
    out.push_back(single);

    contacts_list_next(*contacts_list_ptr);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactManagerGetAddressBook(const JsonObject& args,
                                            JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long address_book_id =
      common::stol(FromJson<JsonString>(args, "addressBookId"));

  contacts_record_h contacts_record;
  int error_code = contacts_db_get_record(_contacts_address_book._uri,
                                          static_cast<int>(address_book_id),
                                          &contacts_record);
  if (CONTACTS_ERROR_NONE != error_code || nullptr == contacts_record) {
    LoggerE("Fail to get addressbook record, error code: %d", error_code);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Fail to get address book with given id");
  }

  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  int account_id;
  status = ContactUtil::GetIntFromRecord(
      contacts_record, _contacts_address_book.account_id, &account_id);
  if (status.IsError()) return status;

  int mode;
  status = ContactUtil::GetIntFromRecord(contacts_record,
                                         _contacts_address_book.mode, &mode);
  if (status.IsError()) return status;

  char* name;
  status = ContactUtil::GetStrFromRecord(contacts_record,
                                         _contacts_address_book.name, &name);
  if (status.IsError()) return status;

  out["accountId"] = picojson::value(static_cast<double>(account_id));
  out["name"] = picojson::value(name);
  out["readOnly"] =
      picojson::value(CONTACTS_ADDRESS_BOOK_MODE_READONLY == mode);

  return PlatformResult(ErrorCode::NO_ERROR);
}

namespace {
PlatformResult ContactManagerGetInternal(int person_id, JsonObject* out) {
  LoggerD("Enter");
  contacts_record_h contacts_record = nullptr;

  int error_code = contacts_db_get_record(_contacts_person._uri, person_id,
                                          &contacts_record);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Person with id: %d, not found, error: %d", person_id, error_code);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Person not found");
  }

  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  PlatformResult status =
      ContactUtil::ImportPersonFromContactsRecord(contacts_record, out);
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
}
}

PlatformResult ContactManagerAddAddressBook(const JsonObject& args,
                                            JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const JsonObject& addressBook = FromJson<JsonObject>(args, "addressBook");

  if (!IsNull(addressBook, "id")) {
    LoggerW("AddressBook already exists");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "AddressBook already exists");
  }

  contacts_record_h contacts_record;
  int ret =
      contacts_record_create(_contacts_address_book._uri, &contacts_record);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerE("Failed to create address book record, error code : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to create address book record");
  }
  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  status = ContactUtil::SetStrInRecord(
      contacts_record, _contacts_address_book.name,
      FromJson<JsonString>(addressBook, "name").c_str());
  if (status.IsError()) return status;

  contacts_address_book_mode_e mode = FromJson<bool>(addressBook, "readOnly")
                                          ? CONTACTS_ADDRESS_BOOK_MODE_READONLY
                                          : CONTACTS_ADDRESS_BOOK_MODE_NONE;
  status = ContactUtil::SetIntInRecord(
      contacts_record, _contacts_address_book.mode, static_cast<int>(mode));
  if (status.IsError()) return status;

  double account_id = FromJson<double>(addressBook, "accountId");
  status = ContactUtil::SetIntInRecord(contacts_record,
                                       _contacts_address_book.account_id,
                                       static_cast<int>(account_id));
  if (status.IsError()) return status;

  int address_book_id;
  ret = contacts_db_insert_record(*contacts_record_ptr, &address_book_id);
  if (CONTACTS_ERROR_NONE != ret) {
    LoggerE("Failed to insert address book record, error code: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to insert address book record");
  }

  out["id"] = picojson::value(std::to_string(address_book_id));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactManagerRemoveAddressBook(const JsonObject& args,
                                               JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long address_book_id =
      common::stol(FromJson<JsonString>(args, "addressBookId"));

  contacts_record_h contacts_record;
  int error_code = contacts_db_get_record(_contacts_address_book._uri,
                                          static_cast<int>(address_book_id),
                                          &contacts_record);
  if (CONTACTS_ERROR_NONE != error_code || nullptr == contacts_record) {
    LoggerE("Fail to get addressbook record, error code: %d", error_code);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Fail to get address book with given id");
  }

  int ret = contacts_db_delete_record(_contacts_address_book._uri,
                                      static_cast<int>(address_book_id));
  if (CONTACTS_ERROR_NONE != ret) {
    LOGE("Failed to delete address book record, error code : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to delete address book record");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactManagerGet(const JsonObject& args, JsonObject& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long person_id = common::stol(FromJson<JsonString>(args, "personId"));

  return ContactManagerGetInternal(person_id, &out);
}

PlatformResult ContactManagerUpdate(const JsonObject& args, JsonObject&) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const JsonObject& person = FromJson<JsonObject>(args, "person");
  long person_id = common::stol(FromJson<JsonString>(person, "id"));
  contacts_record_h contacts_record = nullptr;
  int error_code = contacts_db_get_record(_contacts_person._uri, person_id,
                                          &contacts_record);

  if (CONTACTS_ERROR_NONE != error_code) {
    return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Person not found");
  }

  status = ContactUtil::ExportPersonToContactsRecord(contacts_record, person);
  if (status.IsError()) return status;

  ContactUtil::ContactsRecordHPtr contacts_record_ptr(
      &contacts_record, ContactUtil::ContactsDeleter);

  error_code = contacts_db_update_record(*contacts_record_ptr);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("error code: %d", error_code);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Error during executing contacts_db_update_record()");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactManagerRemove(const JsonObject& args, JsonObject&) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  long person_id = common::stol(FromJson<JsonString>(args, "personId"));

  if (person_id < 0) {
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Negative person id");
  }

  int error_code = contacts_db_delete_record(_contacts_person._uri, person_id);
  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("Error during removing contact, error: %d", error_code);
    return PlatformResult(ErrorCode::NOT_FOUND_ERR,
                          "Error during removing contact");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactManagerFind(const JsonObject& args, JsonArray& out) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  contacts_query_h contacts_query = nullptr;
  int error_code =
      contacts_query_create(_contacts_person._uri, &contacts_query);
  status =
      ContactUtil::ErrorChecker(error_code, "Failed contacts_query_create");
  if (status.IsError()) return status;

  ContactUtil::ContactsQueryHPtr contacts_query_ptr(
      &contacts_query, ContactUtil::ContactsQueryDeleter);

  // Add filter to query
  std::vector<std::vector<ContactUtil::ContactsFilterPtr>> intermediate_filters(
      1);

  if (!IsNull(args, "filter")) {
    FilterVisitor visitor;
    visitor.SetOnAttributeFilter([&](const std::string& name,
                                     AttributeMatchFlag match_flag,
                                     const JsonValue& match_value) {
      Person::PersonProperty property;
      status = Person::PersonPropertyFromString(name, &property);
      if (status.IsError()) return status;

      contacts_filter_h contacts_filter = nullptr;
      int error_code =
          contacts_filter_create(_contacts_person._uri, &contacts_filter);
      status = ContactUtil::ErrorChecker(error_code,
                                         "Failed contacts_query_set_filter");
      if (status.IsError()) return status;

      ContactUtil::ContactsFilterPtr contacts_filter_ptr(
          contacts_filter, ContactUtil::ContactsFilterDeleter);

      if (property.type == kPrimitiveTypeBoolean) {
        bool value = true;
        if (AttributeMatchFlag::kExists != match_flag) {
          value = JsonCast<bool>(match_value);
        }
        error_code = contacts_filter_add_bool(contacts_filter,
                                              property.propertyId, value);
        status = ContactUtil::ErrorChecker(error_code,
                                           "Failed contacts_filter_add_bool");
        if (status.IsError()) return status;
      } else if (property.type == kPrimitiveTypeString) {
        std::string value = JsonCast<std::string>(match_value);

        contacts_match_str_flag_e flag = CONTACTS_MATCH_EXISTS;
        if (AttributeMatchFlag::kExactly == match_flag) {
          flag = CONTACTS_MATCH_EXACTLY;
        } else if (AttributeMatchFlag::kFullString == match_flag) {
          flag = CONTACTS_MATCH_FULLSTRING;
        } else if (AttributeMatchFlag::kContains == match_flag) {
          flag = CONTACTS_MATCH_CONTAINS;
        } else if (AttributeMatchFlag::kStartsWith == match_flag) {
          flag = CONTACTS_MATCH_STARTSWITH;
        } else if (AttributeMatchFlag::kEndsWith == match_flag) {
          flag = CONTACTS_MATCH_ENDSWITH;
        } else if (AttributeMatchFlag::kExists == match_flag) {
          flag = CONTACTS_MATCH_EXISTS;
          value = "";
        }
        error_code = contacts_filter_add_str(
            contacts_filter, property.propertyId, flag, value.c_str());
        status = ContactUtil::ErrorChecker(error_code,
                                           "Failed contacts_filter_add_str");
        if (status.IsError()) return status;
      } else if (property.type == kPrimitiveTypeLong ||
                 property.type == kPrimitiveTypeId) {
        int value;
        if (property.type == kPrimitiveTypeLong) {
          value = static_cast<int>(JsonCast<double>(match_value));
        } else {
          value = common::stol(JsonCast<std::string>(match_value));
        }
        if (value < 0) {
          return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                                "Match value cannot be less than 0");
        }
        contacts_match_int_flag_e flag;
        if (AttributeMatchFlag::kExists == match_flag) {
          flag = CONTACTS_MATCH_GREATER_THAN_OR_EQUAL;
          value = 0;
        } else if (AttributeMatchFlag::kStartsWith == match_flag ||
                   AttributeMatchFlag::kContains == match_flag) {
          flag = CONTACTS_MATCH_GREATER_THAN_OR_EQUAL;
        } else if (AttributeMatchFlag::kEndsWith == match_flag) {
          flag = CONTACTS_MATCH_LESS_THAN_OR_EQUAL;
        } else {
          flag = CONTACTS_MATCH_EQUAL;
        }

        error_code = contacts_filter_add_int(contacts_filter,
                                             property.propertyId, flag, value);
        status = ContactUtil::ErrorChecker(error_code,
                                           "Failed contacts_filter_add_str");
        if (status.IsError()) return status;
      } else {
        LoggerE("Invalid primitive type!");
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Invalid primitive type!");
      }
      intermediate_filters[intermediate_filters.size() - 1].push_back(
          std::move(contacts_filter_ptr));

      return PlatformResult(ErrorCode::NO_ERROR);
    });

    visitor.SetOnAttributeRangeFilter([&](const std::string& name,
                                          const JsonValue& initial_value,
                                          const JsonValue& end_value) {
      Person::PersonProperty property;
      status = Person::PersonPropertyFromString(name, &property);
      if (status.IsError()) return status;

      contacts_filter_h contacts_filter = nullptr;
      int error_code =
          contacts_filter_create(_contacts_person._uri, &contacts_filter);
      status = ContactUtil::ErrorChecker(error_code,
                                         "Failed contacts_query_set_filter");
      if (status.IsError()) return status;

      ContactUtil::ContactsFilterPtr contacts_filter_ptr(
          contacts_filter, ContactUtil::ContactsFilterDeleter);

      bool initial_value_exists = (!IsNull(initial_value));
      bool end_value_exists = (!IsNull(end_value));

      if (property.type == kPrimitiveTypeBoolean) {
        bool initial_value_bool = false;
        bool end_value_bool = false;

        if (initial_value_exists) {
          initial_value_bool = JsonCast<bool>(initial_value);
        }
        if (end_value_exists) {
          end_value_bool = JsonCast<bool>(end_value);
        }

        if (initial_value_exists && end_value_exists) {
          if (initial_value_bool == end_value_bool) {
            error_code = contacts_filter_add_bool(
                contacts_filter, property.propertyId, initial_value_bool);
            status = ContactUtil::ErrorChecker(
                error_code, "Failed contacts_filter_add_bool");
            if (status.IsError()) return status;
          }
        } else if (initial_value_exists) {
          if (initial_value_bool) {
            error_code = contacts_filter_add_bool(contacts_filter,
                                                  property.propertyId, true);
            status = ContactUtil::ErrorChecker(
                error_code, "Failed contacts_filter_add_bool");
            if (status.IsError()) return status;
          }
        } else if (end_value_exists) {
          if (!end_value_bool) {
            error_code = contacts_filter_add_bool(contacts_filter,
                                                  property.propertyId, false);
            status = ContactUtil::ErrorChecker(
                error_code, "Failed contacts_filter_add_bool");
            if (status.IsError()) return status;
          }
        }
      } else if (property.type == kPrimitiveTypeString) {
        std::string initial_value_str;
        std::string end_value_str;

        if (initial_value_exists) {
          initial_value_str = JsonCast<std::string>(initial_value);
        }

        if (end_value_exists) {
          end_value_str = JsonCast<std::string>(end_value);
        }

        if (initial_value_exists && end_value_exists) {
          contacts_filter_h sub_filter = NULL;

          error_code =
              contacts_filter_create(_contacts_person._uri, &sub_filter);
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_str");
          if (status.IsError()) return status;

          ContactUtil::ContactsFilterPtr sub_filter_ptr(
              sub_filter, ContactUtil::ContactsFilterDeleter);

          error_code = contacts_filter_add_str(sub_filter, property.propertyId,
                                               CONTACTS_MATCH_STARTSWITH,
                                               initial_value_str.c_str());
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_str");
          if (status.IsError()) return status;

          error_code = contacts_filter_add_operator(
              sub_filter, CONTACTS_FILTER_OPERATOR_AND);
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_str");
          if (status.IsError()) return status;

          error_code = contacts_filter_add_str(sub_filter, property.propertyId,
                                               CONTACTS_MATCH_ENDSWITH,
                                               end_value_str.c_str());
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_str");
          if (status.IsError()) return status;

          error_code = contacts_filter_add_filter(contacts_filter, sub_filter);
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_str");
          if (status.IsError()) return status;
        } else if (initial_value_exists) {
          error_code = contacts_filter_add_str(
              contacts_filter, property.propertyId, CONTACTS_MATCH_STARTSWITH,
              initial_value_str.c_str());
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_str");
          if (status.IsError()) return status;
        } else if (end_value_exists) {
          error_code = contacts_filter_add_str(
              contacts_filter, property.propertyId, CONTACTS_MATCH_ENDSWITH,
              end_value_str.c_str());
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_str");
          if (status.IsError()) return status;
        }
      } else if (property.type == kPrimitiveTypeLong ||
                 property.type == kPrimitiveTypeId) {
        int initial_value_int = 0;
        int end_value_int = 0;

        if (initial_value_exists) {
          if (property.type == kPrimitiveTypeLong) {
            initial_value_int =
                static_cast<int>(JsonCast<double>(initial_value));
          } else {
            initial_value_int =
                common::stol(JsonCast<std::string>(initial_value));
          }
        }

        if (end_value_exists) {
          if (property.type == kPrimitiveTypeLong) {
            end_value_int = static_cast<int>(JsonCast<double>(end_value));
          } else {
            end_value_int = common::stol(JsonCast<std::string>(end_value));
          }
        }

        if (initial_value_exists && end_value_exists) {
          contacts_filter_h sub_filter = NULL;

          error_code =
              contacts_filter_create(_contacts_person._uri, &sub_filter);
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_bool");
          if (status.IsError()) return status;

          ContactUtil::ContactsFilterPtr sub_filter_ptr(
              sub_filter, ContactUtil::ContactsFilterDeleter);

          error_code = contacts_filter_add_int(
              sub_filter, property.propertyId,
              CONTACTS_MATCH_GREATER_THAN_OR_EQUAL, initial_value_int);
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_int");
          if (status.IsError()) return status;

          error_code = contacts_filter_add_operator(
              sub_filter, CONTACTS_FILTER_OPERATOR_AND);
          status = ContactUtil::ErrorChecker(
              error_code, "Failed contacts_filter_add_operator");
          if (status.IsError()) return status;

          error_code = contacts_filter_add_int(
              sub_filter, property.propertyId,
              CONTACTS_MATCH_LESS_THAN_OR_EQUAL, end_value_int);
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_int");
          if (status.IsError()) return status;

          error_code = contacts_filter_add_filter(contacts_filter, sub_filter);
          status = ContactUtil::ErrorChecker(
              error_code, "Failed contacts_filter_add_filter");
          if (status.IsError()) return status;
        } else if (initial_value_exists) {
          error_code = contacts_filter_add_int(
              contacts_filter, property.propertyId,
              CONTACTS_MATCH_GREATER_THAN_OR_EQUAL, initial_value_int);
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_int");
          if (status.IsError()) return status;
        } else if (end_value_exists) {
          error_code = contacts_filter_add_int(
              contacts_filter, property.propertyId,
              CONTACTS_MATCH_LESS_THAN_OR_EQUAL, end_value_int);
          status = ContactUtil::ErrorChecker(error_code,
                                             "Failed contacts_filter_add_int");
          if (status.IsError()) return status;
        }
      } else {
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Invalid primitive type!");
      }
      intermediate_filters[intermediate_filters.size() - 1].push_back(
          std::move(contacts_filter_ptr));

      return PlatformResult(ErrorCode::NO_ERROR);
    });

    visitor.SetOnCompositeFilterBegin([&](CompositeFilterType type) {
      intermediate_filters.push_back(
          std::vector<ContactUtil::ContactsFilterPtr>());

      return PlatformResult(ErrorCode::NO_ERROR);
    });

    visitor.SetOnCompositeFilterEnd([&](CompositeFilterType type) {
      if (intermediate_filters.size() == 0) {
        return PlatformResult(ErrorCode::UNKNOWN_ERR,
                              "Reached stack size equal to 0!");
      }

      contacts_filter_h merged_filter = nullptr;
      int error_code =
          contacts_filter_create(_contacts_person._uri, &merged_filter);
      status = ContactUtil::ErrorChecker(error_code,
                                         "Failed contacts_query_set_filter");
      if (status.IsError()) return status;

      ContactUtil::ContactsFilterPtr merged_filter_ptr(
          merged_filter, ContactUtil::ContactsFilterDeleter);

      for (std::size_t i = 0; i < intermediate_filters.back().size(); ++i) {
        error_code = contacts_filter_add_filter(
            merged_filter, intermediate_filters.back().at(i).get());
        status = ContactUtil::ErrorChecker(error_code,
                                           "Failed contacts_query_set_filter");
        if (status.IsError()) return status;

        if (CompositeFilterType::kIntersection == type) {
          error_code = contacts_filter_add_operator(
              merged_filter, CONTACTS_FILTER_OPERATOR_AND);
          status = ContactUtil::ErrorChecker(
              error_code, "Failed contacts_query_set_filter");
          if (status.IsError()) return status;
        } else if (CompositeFilterType::kUnion == type) {
          error_code = contacts_filter_add_operator(
              merged_filter, CONTACTS_FILTER_OPERATOR_OR);
          status = ContactUtil::ErrorChecker(
              error_code, "Failed contacts_query_set_filter");
          if (status.IsError()) return status;
        } else {
          return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                                "Invalid union type!");
        }
      }

      intermediate_filters.pop_back();
      intermediate_filters.back().push_back(std::move(merged_filter_ptr));

      return PlatformResult(ErrorCode::NO_ERROR);
    });

    status = visitor.Visit(FromJson<JsonObject>(args, "filter"));
    if (status.IsError()) return status;

    // Should compute only one filter always.
    if ((intermediate_filters.size() != 1) ||
        (intermediate_filters[0].size() != 1)) {
      LoggerE("Bad filter evaluation!");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Bad filter evaluation!");
    }
    // Filter is generated
    error_code = contacts_query_set_filter(contacts_query,
                                           intermediate_filters[0][0].get());
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_query_set_filter");
    if (status.IsError()) return status;
  }

  contacts_list_h person_list = nullptr;
  error_code =
      contacts_db_get_records_with_query(contacts_query, 0, 0, &person_list);

  status = ContactUtil::ErrorChecker(
      error_code, "Failed contacts_db_get_records_with_query");
  if (status.IsError()) return status;

  ContactUtil::ContactsListHPtr person_list_ptr(
      &person_list, ContactUtil::ContactsListDeleter);

  int record_count = 0;
  error_code = contacts_list_get_count(person_list, &record_count);
  status =
      ContactUtil::ErrorChecker(error_code, "Failed contacts_list_get_count");
  if (status.IsError()) return status;

  contacts_list_first(person_list);

  for (int i = 0; i < record_count; i++) {
    contacts_record_h contacts_record;
    error_code =
        contacts_list_get_current_record_p(person_list, &contacts_record);
    if (error_code != CONTACTS_ERROR_NONE || contacts_record == NULL) {
      LoggerW("Failed group record (ret:%d)", error_code);
      continue;
    }

    int id_value = 0;
    error_code = contacts_record_get_int(contacts_record, _contacts_person.id,
                                         &id_value);

    status =
        ContactUtil::ErrorChecker(error_code, "Failed contacts_record_get_int");
    if (status.IsError()) return status;

    out.push_back(JsonValue(static_cast<double>(id_value)));

    contacts_list_next(person_list);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactManagerImportFromVCard(const JsonObject& args,
                                             JsonObject& out) {
  LoggerD("Enter");
  // I'm not sure how to call it. Should it be 'Contact', 'vCard' or what?
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  const char* vcard_char_ptr = FromJson<JsonString>(args, "contact").c_str();

  contacts_list_h contacts_list = nullptr;

  int err = contacts_vcard_parse_to_contacts(vcard_char_ptr, &contacts_list);
  if (CONTACTS_ERROR_INVALID_PARAMETER == err) {
    LoggerE("Invalid vCard string");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Invalid vCard string");
  } else if (CONTACTS_ERROR_NONE != err) {
    LoggerE("Fail to convert vCard from string");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Fail to convert vCard from string");
  }

  SCOPE_EXIT {
    contacts_list_destroy(contacts_list, true);
  };

  int record_count = 0;
  err = contacts_list_get_count(contacts_list, &record_count);
  if (CONTACTS_ERROR_NONE != err || 0 == record_count) {
    LoggerE("Invalid vCard string.");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Invalid vCard string.");
  }

  contacts_record_h contacts_record;
  contacts_list_first(contacts_list);
  err = contacts_list_get_current_record_p(contacts_list, &contacts_record);
  if (CONTACTS_ERROR_NONE != err || nullptr == contacts_record) {
    LoggerE("Invalid vCard string.");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Invalid vCard string.");
  }

  status = ContactUtil::ImportContactFromContactsRecord(contacts_record, &out);
  if (status.IsError()) return status;

  return PlatformResult(ErrorCode::NO_ERROR);
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

void ContactManagerListenerCallback(const char* view_uri, char* changes,
                                    void* user_data) {
  LoggerD("ContactManagerListenerCallback");

  (void)view_uri;

  if (nullptr == changes) {
    LoggerW("changes is NULL");
    return;
  }
  if (strlen(changes) == 0) {
    LoggerW("changes is empty");
    return;
  }

  SLoggerD("view_uri: %s", view_uri);
  SLoggerD("changes: %s", changes);

  JsonValue result{JsonObject{}};
  JsonObject& result_obj = result.get<JsonObject>();
  result_obj.insert(std::make_pair(std::string("listenerId"),
      picojson::value(kContactPersonListenerId)));
  JsonArray& added = result_obj.insert(std::make_pair(std::string("added"),
      picojson::value(JsonArray{}))).first->second.get<JsonArray>();
  JsonArray& updated = result_obj.insert(std::make_pair(std::string("updated"),
      picojson::value(JsonArray{}))).first->second.get<JsonArray>();
  JsonArray& removed = result_obj.insert(std::make_pair(std::string("removed"),
      picojson::value(JsonArray{}))).first->second.get<JsonArray>();

  std::unique_ptr<char, void (*)(char*)> tmp(strdup(changes),
                                             [](char* p) { free(p); });

  // 'changes' may contain repeated values, we need to filter it
  std::set<int> added_ids;
  std::set<int> updated_ids;
  std::set<int> removed_ids;

  char* tmpptr = nullptr;

  char* token = strtok_r(tmp.get(), kTokenDelimiter, &tmpptr);
  while (token) {
    if (IsNumeric(token)) {
      int type = atoi(token);
      token = strtok_r(nullptr, kTokenDelimiter, &tmpptr);
      if (!token) {
        break;
      }
      if (IsNumeric(token)) {
        int person_id = atoi(token);
        switch (type) {
          case CONTACTS_CHANGE_INSERTED: {
            if (added_ids.find(person_id) == added_ids.end()) {
              added.push_back(JsonValue{JsonObject{}});
              PlatformResult status = ContactManagerGetInternal(
                  person_id, &added.back().get<JsonObject>());
              if (status.IsError()) {
                LoggerE("Caught exception in listener callback: %s",
                        status.message().c_str());
                return;
              }
              added_ids.insert(person_id);
            }

            break;
          }
          case CONTACTS_CHANGE_UPDATED: {
            if (updated_ids.find(person_id) == updated_ids.end()) {
              updated.push_back(JsonValue{JsonObject{}});
              PlatformResult status = ContactManagerGetInternal(
                  person_id, &updated.back().get<JsonObject>());
              if (status.IsError()) {
                LoggerE("Caught exception in listener callback: %s",
                        status.message().c_str());
                return;
              }
              updated_ids.insert(person_id);
            }

            break;
          }
          case CONTACTS_CHANGE_DELETED: {
            if (removed_ids.find(person_id) == removed_ids.end()) {
              removed.push_back(JsonValue{std::to_string(person_id)});
              removed_ids.insert(person_id);
            }
            break;
          }
          default: {
            break;
          }
        }
      }
    }

    token = strtok_r(nullptr, kTokenDelimiter, &tmpptr);
  }

  ContactInstance* instance = static_cast<ContactInstance*>(user_data);
  Instance::PostMessage(instance, result.serialize().c_str());
}
}

PlatformResult ContactManagerStartListening(ContactInstance& instance, const JsonObject& /*args*/,
                                            JsonObject& /*out*/) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  int error_code = contacts_db_add_changed_cb_with_info(
      _contacts_person._uri, ContactManagerListenerCallback, &instance);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("contacts_db_add_changed_cb(_contacts_person._uri) error: %d",
            error_code);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to start listening");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactManagerStopListening(ContactInstance& instance, const JsonObject& /*args*/,
                                           JsonObject& /*out*/) {
  LoggerD("Enter");
  PlatformResult status = ContactUtil::CheckDBConnection();
  if (status.IsError()) return status;

  int error_code = contacts_db_remove_changed_cb_with_info(
      _contacts_person._uri, ContactManagerListenerCallback, &instance);

  if (CONTACTS_ERROR_NONE != error_code) {
    LoggerE("contacts_db_remove_changed_cb(_contacts_person._uri) error: %d",
            error_code);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to stop listening");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace ContactManager
}  // namespace contact
}  // namespace extension
