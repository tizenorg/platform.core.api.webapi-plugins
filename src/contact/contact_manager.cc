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

#include <wrt-common/native-context.h>
#include "common/converter.h"
#include "common/picojson.h"
#include "common/logger.h"
#include "common/native-plugin.h"
#include "common/task-queue.h"
#include "common/filter-utils.h"

#include <contacts.h>
#include "contact/person.h"

namespace extension {
namespace contact {
namespace ContactManager {

namespace {
const char* kContactPersonListenerId = "ContactPersonChangeListener";
const char* kTokenDelimiter = " ,:";
}

using namespace extension::common;
using namespace wrt::common;

void ContactManager_getAddressBooks(const JsonObject& args,
                                    JsonObject& out) {
  ContactUtil::CheckDBConnection();
  typedef std::shared_ptr<JsonValue> shared_json_value;
  auto work_func = [=](const shared_json_value & response)->void {
    JsonObject& response_obj = response->get<JsonObject>();
    JsonArray& batch_result =
        response_obj.insert(
                         std::make_pair("result", JsonValue{JsonArray{}}))
            .first->second.get<JsonArray>();
    try {
      contacts_list_h address_book_list = nullptr;

      int error_code = contacts_db_get_all_records(_contacts_address_book._uri,
                                                   0, 0, &address_book_list);
      if (CONTACTS_ERROR_NONE != error_code) {
        LOGE("Fail to get address book list, error: %d", error_code);
        throw UnknownException("Fail to get address book list");
      }

      ContactUtil::ContactsListHPtr contacts_list_ptr(
          &address_book_list, ContactUtil::ContactsListDeleter);

      unsigned int record_count = 0;
      error_code = contacts_list_get_count(*contacts_list_ptr, &record_count);
      if (CONTACTS_ERROR_NONE != error_code) {
        LOGE("Fail to get address book list count, error: %d", error_code);
        throw UnknownException("Fail to get address book list count");
      }

      error_code = contacts_list_first(*contacts_list_ptr);
      if (CONTACTS_ERROR_NONE != error_code) {
        LOGE("Fail to get address book from list, error: %d", error_code);
        throw UnknownException("Fail to get address book from list");
      }

      for (unsigned int i = 0; i < record_count; i++) {
        contacts_record_h contacts_record = nullptr;
        error_code = contacts_list_get_current_record_p(*contacts_list_ptr,
                                                        &contacts_record);

        if (CONTACTS_ERROR_NONE != error_code) {
          LOGW("Fail to get address book record");
          continue;
        }

        int id = 0;
        int mode = 0;
        char* name = nullptr;
        try {
          ContactUtil::GetIntFromRecord(contacts_record,
                                        _contacts_address_book.id, &id);

          ContactUtil::GetIntFromRecord(contacts_record,
                                        _contacts_address_book.mode, &mode);

          ContactUtil::GetStrFromRecord(contacts_record,
                                        _contacts_address_book.name, &name);
        }
        catch (...) {
          LOGW("Fail to get data from address book");
          continue;
        }

        JsonObject single_out;
        single_out.insert(std::make_pair("id", std::to_string(id)));
        single_out.insert(std::make_pair("name", name));
        single_out.insert(std::make_pair(
            "readOnly", CONTACTS_ADDRESS_BOOK_MODE_READONLY == mode));
        batch_result.push_back(JsonValue{single_out});

        contacts_list_next(*contacts_list_ptr);
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

  TaskQueue::GetInstance().Queue<JsonValue>(
      work_func, after_work_func,
      std::shared_ptr<JsonValue>{new JsonValue{JsonObject{}}});
}

void ContactManager_getAddressBook(const JsonObject& args, JsonObject& out) {
  NativePlugin::CheckAccess(ContactUtil::kContactReadPrivileges);
  ContactUtil::CheckDBConnection();
  long address_book_id;
  try {
    address_book_id =
        common::stol(FromJson<JsonString>(args, "addressBookID"));
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

void ContactManager_update(const JsonObject& args) {
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

void ContactManager_remove(const JsonObject& args) {
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

  // TODO implement contact sorting.
  LoggerD("Entered");

  int callback_handle = NativePlugin::GetAsyncCallbackHandle(args);

  auto get = [args](const std::shared_ptr<JsonValue> & response)->void {
    LoggerD("Entered");
    try {
      int error_code = 0;
      contacts_query_h contacts_query = nullptr;
      error_code =
          contacts_query_create(_contacts_person._uri, &contacts_query);
      ContactUtil::ErrorChecker(error_code, "Failed contacts_query_create");

      ContactUtil::ContactsQueryHPtr contacts_query_ptr(
          &contacts_query, ContactUtil::ContactsQueryDeleter);

      // Add filter to query
      std::vector<std::vector<ContactUtil::ContactsFilterPtr>>
          intermediate_filters(1);
      if (!IsNull(args, "filter")) {
        FilterVisitor visitor;
        visitor.SetOnAttributeFilter([&](const std::string& name,
                                         AttributeMatchFlag match_flag,
                                         const JsonValue& match_value) {
          const Person::PersonProperty& property =
              Person::PersonProperty_fromString(name);

          contacts_filter_h contacts_filter = nullptr;
          int error_code =
              contacts_filter_create(_contacts_person._uri, &contacts_filter);
          ContactUtil::ErrorChecker(error_code,
                                    "Failed contacts_query_set_filter");
          ContactUtil::ContactsFilterPtr contacts_filter_ptr(
              contacts_filter, ContactUtil::ContactsFilterDeleter);

          if (property.type == kPrimitiveTypeBoolean) {
            bool value = true;
            if (AttributeMatchFlag::kExists != match_flag) {
              value = JsonCast<bool>(match_value);
            }
            error_code = contacts_filter_add_bool(contacts_filter,
                                                  property.propertyId, value);
            ContactUtil::ErrorChecker(error_code,
                                      "Failed contacts_filter_add_bool");
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
            ContactUtil::ErrorChecker(error_code,
                                      "Failed contacts_filter_add_str");
          } else if (property.type == kPrimitiveTypeLong ||
                     property.type == kPrimitiveTypeId) {
            int value;
            if (property.type == kPrimitiveTypeLong) {
              value = static_cast<int>(JsonCast<double>(match_value));
            } else {
              value = common::stol(JsonCast<std::string>(match_value));
            }
            if (value < 0) {
              throw InvalidValuesException("Match value cannot be less than 0");
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

            error_code = contacts_filter_add_int(
                contacts_filter, property.propertyId, flag, value);
            ContactUtil::ErrorChecker(error_code,
                                      "Failed contacts_filter_add_str");
          } else {
            throw UnknownException("Invalid primitive type!");
          }
          intermediate_filters[intermediate_filters.size() - 1]
              .push_back(std::move(contacts_filter_ptr));
        });

        visitor.SetOnAttributeRangeFilter([&](const std::string& name,
                                              const JsonValue& initial_value,
                                              const JsonValue& end_value) {
          const Person::PersonProperty& property =
              Person::PersonProperty_fromString(name);

          contacts_filter_h contacts_filter = nullptr;
          int error_code =
              contacts_filter_create(_contacts_person._uri, &contacts_filter);
          ContactUtil::ErrorChecker(error_code,
                                    "Failed contacts_query_set_filter");
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
                ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_filter_add_bool");
              }
            } else if (initial_value_exists) {
              if (initial_value_bool) {
                error_code = contacts_filter_add_bool(
                    contacts_filter, property.propertyId, true);
                ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_filter_add_bool");
              }
            } else if (end_value_exists) {
              if (!end_value_bool) {
                error_code = contacts_filter_add_bool(
                    contacts_filter, property.propertyId, false);
                ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_filter_add_bool");
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
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_str");

              ContactUtil::ContactsFilterPtr sub_filter_ptr(
                  sub_filter, ContactUtil::ContactsFilterDeleter);

              error_code = contacts_filter_add_str(
                  sub_filter, property.propertyId, CONTACTS_MATCH_STARTSWITH,
                  initial_value_str.c_str());
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_str");
              error_code = contacts_filter_add_operator(
                  sub_filter, CONTACTS_FILTER_OPERATOR_AND);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_str");

              error_code = contacts_filter_add_str(
                  sub_filter, property.propertyId, CONTACTS_MATCH_ENDSWITH,
                  end_value_str.c_str());
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_str");

              error_code =
                  contacts_filter_add_filter(contacts_filter, sub_filter);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_str");
            } else if (initial_value_exists) {
              error_code = contacts_filter_add_str(
                  contacts_filter, property.propertyId,
                  CONTACTS_MATCH_STARTSWITH, initial_value_str.c_str());
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_str");
            } else if (end_value_exists) {
              error_code = contacts_filter_add_str(
                  contacts_filter, property.propertyId, CONTACTS_MATCH_ENDSWITH,
                  end_value_str.c_str());
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_str");
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
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_bool");
              ContactUtil::ContactsFilterPtr sub_filter_ptr(
                  sub_filter, ContactUtil::ContactsFilterDeleter);

              error_code = contacts_filter_add_int(
                  sub_filter, property.propertyId,
                  CONTACTS_MATCH_GREATER_THAN_OR_EQUAL, initial_value_int);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_int");

              error_code = contacts_filter_add_operator(
                  sub_filter, CONTACTS_FILTER_OPERATOR_AND);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_operator");

              error_code = contacts_filter_add_int(
                  sub_filter, property.propertyId,
                  CONTACTS_MATCH_LESS_THAN_OR_EQUAL, end_value_int);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_int");

              error_code =
                  contacts_filter_add_filter(contacts_filter, sub_filter);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_filter");
            } else if (initial_value_exists) {
              error_code = contacts_filter_add_int(
                  contacts_filter, property.propertyId,
                  CONTACTS_MATCH_GREATER_THAN_OR_EQUAL, initial_value_int);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_int");
            } else if (end_value_exists) {
              error_code = contacts_filter_add_int(
                  contacts_filter, property.propertyId,
                  CONTACTS_MATCH_LESS_THAN_OR_EQUAL, end_value_int);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_filter_add_int");
            }
          } else {
            throw UnknownException("Invalid primitive type!");
          }
          intermediate_filters[intermediate_filters.size() - 1]
              .push_back(std::move(contacts_filter_ptr));
        });

        visitor.SetOnCompositeFilterBegin([&](CompositeFilterType /*type*/) {
          intermediate_filters.push_back(
              std::vector<ContactUtil::ContactsFilterPtr>());
        });

        visitor.SetOnCompositeFilterEnd([&](CompositeFilterType type) {
          if (intermediate_filters.size() == 0) {
            throw UnknownException("Reached stack size equal to 0!");
          }
          // TODO what is this supposed to do? Removing it fixes filtering
          // without any
          // apparent consequences.
          //                    if (intermediate_filters.back().size()) {
          //                        intermediate_filters.pop_back();
          //                        return;
          //                    }

          contacts_filter_h merged_filter = nullptr;
          int error_code = 0;
          error_code =
              contacts_filter_create(_contacts_person._uri, &merged_filter);
          ContactUtil::ErrorChecker(error_code,
                                    "Failed contacts_query_set_filter");
          ContactUtil::ContactsFilterPtr merged_filter_ptr(
              merged_filter, ContactUtil::ContactsFilterDeleter);

          for (size_t i = 0; i < intermediate_filters.back().size(); ++i) {
            error_code = contacts_filter_add_filter(
                merged_filter, intermediate_filters.back().at(i).get());
            ContactUtil::ErrorChecker(error_code,
                                      "Failed contacts_query_set_filter");
            if (CompositeFilterType::kIntersection == type) {
              error_code = contacts_filter_add_operator(
                  merged_filter, CONTACTS_FILTER_OPERATOR_AND);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_query_set_filter");
            } else if (CompositeFilterType::kUnion == type) {
              error_code = contacts_filter_add_operator(
                  merged_filter, CONTACTS_FILTER_OPERATOR_OR);
              ContactUtil::ErrorChecker(error_code,
                                        "Failed contacts_query_set_filter");
            } else {
              throw InvalidValuesException("Invalid union type!");
            }
          }

          intermediate_filters.pop_back();
          intermediate_filters.back().push_back(std::move(merged_filter_ptr));
        });

        visitor.Visit(FromJson<JsonObject>(args, "filter"));
        // Should compute only one filter always.
        if ((intermediate_filters.size() != 1) ||
            (intermediate_filters[0].size() != 1)) {
          LoggerE("Bad filter evaluation!");
          throw UnknownException("Bad filter evaluation!");
        }
        // Filter is generated
        error_code = contacts_query_set_filter(
            contacts_query, intermediate_filters[0][0].get());
        ContactUtil::ErrorChecker(error_code,
                                  "Failed contacts_query_set_filter");
      }

      contacts_list_h person_list = nullptr;
      error_code = contacts_db_get_records_with_query(contacts_query, 0, 0,
                                                      &person_list);

      ContactUtil::ErrorChecker(error_code,
                                "Failed contacts_db_get_records_with_query");

      ContactUtil::ContactsListHPtr person_list_ptr(
          &person_list, ContactUtil::ContactsListDeleter);

      unsigned int record_count = 0;
      error_code = contacts_list_get_count(person_list, &record_count);
      ContactUtil::ErrorChecker(error_code, "Failed contacts_list_get_count");

      contacts_list_first(person_list);
      JsonValue result{JsonArray{}};
      JsonArray& persons = result.get<JsonArray>();

      for (unsigned int i = 0; i < record_count; i++) {
        contacts_record_h contacts_record;
        error_code =
            contacts_list_get_current_record_p(person_list, &contacts_record);
        if (error_code != CONTACTS_ERROR_NONE || contacts_record == NULL) {
          LoggerW("Failed group record (ret:%d)", error_code);
          continue;
        }

        int id_value = 0;
        error_code = contacts_record_get_int(contacts_record,
                                             _contacts_person.id, &id_value);

        ContactUtil::ErrorChecker(error_code, "Failed contacts_record_get_int");

        persons.push_back(JsonValue(static_cast<double>(id_value)));

        contacts_list_next(person_list);
      }

      NativePlugin::ReportSuccess(result, response->get<JsonObject>());
    }
    catch (const BasePlatformException& e) {
      LoggerE("error: %s: %s", e.name().c_str(), e.message().c_str());
      NativePlugin::ReportError(e, response->get<JsonObject>());
    }
  };

  auto get_response = [callback_handle](const std::shared_ptr<JsonValue> &
                                        response)->void {
    wrt::common::NativeContext::GetInstance()->InvokeCallback(
        callback_handle, response->serialize());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));

  NativePlugin::ReportSuccess(out);
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

  unsigned int record_count = 0;
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
    LOGW("changes is NULL");
    return;
  }
  if (strlen(changes) == 0) {
    LOGW("changes is empty");
    return;
  }

  JsonValue result{JsonObject{}};
  JsonObject& result_obj = result.get<JsonObject>();
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
    catch (common::BasePlatformException& ex) {
      LoggerE("Caught exception \"" << ex.name() << "\" in listener callback: "
                                    << ex.message());
    }

    token = strtok(nullptr, kTokenDelimiter);
  }

  NativeContext::GetInstance()->FireEvent(kContactPersonListenerId,
                                          result.serialize());
}
}

void ContactManager_startListening(/*const JsonObject&, JsonObject& out*/) {
  ContactUtil::CheckDBConnection();
  int error_code = contacts_db_add_changed_cb_with_info(
      _contacts_person._uri, ContactManager_listenerCallback, nullptr);

  if (CONTACTS_ERROR_NONE != error_code) {
    LOGE("contacts_db_add_changed_cb(_contacts_person._uri) error: %d",
         error_code);
    throw UnknownException("Failed to start listening");
  }

  //NativePlugin::ReportSuccess(out);
}

void ContactManager_stopListening(/*const JsonObject&, JsonObject& out*/) {
  ContactUtil::CheckDBConnection();
  int error_code = contacts_db_remove_changed_cb_with_info(
      _contacts_person._uri, ContactManager_listenerCallback, nullptr);

  if (CONTACTS_ERROR_NONE != error_code) {
    LOGE("contacts_db_remove_changed_cb(_contacts_person._uri) error: %d",
         error_code);
    throw UnknownException("Failed to stop listening");
  }

  //NativePlugin::ReportSuccess(out);
}

}  // namespace ContactManager
}  // namespace contact
}  // namespace extension
