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

#include "contact/contact_search_engine.h"

#include <iomanip>

#include "common/converter.h"
#include "common/logger.h"

#include "contact/contact_util.h"

namespace extension {
namespace contact {

using common::ErrorCode;
using common::IsNull;
using common::JsonCast;
using common::PlatformResult;

namespace {

std::string ToString(const picojson::value& value) {
  if (value.is<std::string>()) {
    return value.get<std::string>();
  } else {
    return "";
  }
}

std::tm* ToDateTm(const picojson::value& value) {
  static std::tm t;
  memset(&t, 0, sizeof(std::tm));
  strptime(ToString(value).c_str(), "%Y-%m-%dT%H:%M:%S.%zZ", &t);
  return &t;
}

std::string ToDateDbStr(const tm& date) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setiosflags(std::ios::right) << std::setw(4)
      << (date.tm_year + 1900);
  ss << std::setfill('0') << std::setiosflags(std::ios::right) << std::setw(2)
      << (date.tm_mon + 1);
  ss << std::setfill('0') << std::setiosflags(std::ios::right) << std::setw(2)
      << date.tm_mday;

  return ss.str();
}

int StrToInt(const std::string& str) {
  int result = 0;

  std::istringstream iss(str);
  iss >> result;

  return result;
}

int ToDateDbInt(const tm& date) {
  return StrToInt(ToDateDbStr(date));
}

}  // namespace

ContactSearchEngine::PropertiesMap ContactSearchEngine::s_properties_map_ = {
  {"id",                      { _contacts_simple_contact._uri, _contacts_simple_contact.id, _contacts_simple_contact.id, PrimitiveType_Long } },
  {"personId",                { _contacts_simple_contact._uri, _contacts_simple_contact.id, _contacts_simple_contact.person_id, PrimitiveType_Long } },
  {"addressBookId",           { _contacts_simple_contact._uri, _contacts_simple_contact.id, _contacts_simple_contact.address_book_id, PrimitiveType_Long } },
  {"lastUpdated",             { _contacts_simple_contact._uri, _contacts_simple_contact.id, _contacts_simple_contact.changed_time, PrimitiveType_Long } },
  {"isFavorite",              { _contacts_simple_contact._uri, _contacts_simple_contact.id, _contacts_simple_contact.is_favorite, PrimitiveType_Boolean } },
  {"name.prefix",             { _contacts_name._uri, _contacts_name.contact_id, _contacts_name.prefix, PrimitiveType_String } },
  {"name.suffix",             { _contacts_name._uri, _contacts_name.contact_id, _contacts_name.suffix, PrimitiveType_String } },
  {"name.firstName",          { _contacts_name._uri, _contacts_name.contact_id, _contacts_name.first, PrimitiveType_String } },
  {"name.middleName",         { _contacts_name._uri, _contacts_name.contact_id, _contacts_name.addition, PrimitiveType_String } },
  {"name.lastName",           { _contacts_name._uri, _contacts_name.contact_id, _contacts_name.last, PrimitiveType_String } },
  {"name.nicknames",          { _contacts_nickname._uri, _contacts_nickname.contact_id, _contacts_nickname.name, PrimitiveType_String } },
  {"name.phoneticFirstName",  { _contacts_name._uri, _contacts_name.contact_id, _contacts_name.phonetic_first, PrimitiveType_String } },
  {"name.phoneticMiddleName",  { _contacts_name._uri, _contacts_name.contact_id, _contacts_name.phonetic_middle, PrimitiveType_String } },
  {"name.phoneticLastName",   { _contacts_name._uri, _contacts_name.contact_id, _contacts_name.phonetic_last, PrimitiveType_String } },
  {"name.displayName",        { _contacts_simple_contact._uri, _contacts_simple_contact.id, _contacts_simple_contact.display_name, PrimitiveType_String } },
  {"addresses.country",       { _contacts_address._uri, _contacts_address.contact_id, _contacts_address.country, PrimitiveType_String } },
  {"addresses.region",        { _contacts_address._uri, _contacts_address.contact_id, _contacts_address.region, PrimitiveType_String } },
  {"addresses.city",          { _contacts_address._uri, _contacts_address.contact_id, _contacts_address.locality, PrimitiveType_String } },
  {"addresses.streetAddress", { _contacts_address._uri, _contacts_address.contact_id, _contacts_address.street, PrimitiveType_String } },
  {"addresses.additionalInformation", { _contacts_address._uri, _contacts_address.contact_id, _contacts_address.extended, PrimitiveType_String } },
  {"addresses.postalCode",    { _contacts_address._uri, _contacts_address.contact_id, _contacts_address.postal_code, PrimitiveType_String } },
  {"addresses.isDefault",     { _contacts_address._uri, _contacts_address.contact_id, _contacts_address.is_default, PrimitiveType_Boolean } },
  {"addresses.types",         { _contacts_address._uri, _contacts_address.contact_id, _contacts_address.type, PrimitiveType_Long } },
  {"photoURI",                { _contacts_simple_contact._uri, _contacts_simple_contact.id, _contacts_simple_contact.image_thumbnail_path, PrimitiveType_String } },
  {"phoneNumbers.number",     { _contacts_number._uri, _contacts_number.contact_id, _contacts_number.number, PrimitiveType_String } },
  {"phoneNumbers.isDefault",  { _contacts_number._uri, _contacts_number.contact_id, _contacts_number.is_default, PrimitiveType_Boolean } },
  {"phoneNumbers.types",      { _contacts_number._uri, _contacts_number.contact_id, _contacts_number.type, PrimitiveType_Long } },
  {"emails.email",            { _contacts_email._uri, _contacts_email.contact_id, _contacts_email.email, PrimitiveType_String } },
  {"emails.isDefault",        { _contacts_email._uri, _contacts_email.contact_id, _contacts_email.is_default, PrimitiveType_Boolean } },
  {"emails.types",            { _contacts_email._uri, _contacts_email.contact_id, _contacts_email.type, PrimitiveType_Long } },
  {"birthday",                { _contacts_event._uri, _contacts_event.contact_id, _contacts_event.date, PrimitiveType_Long } },
  {"anniversaries.date",      { _contacts_event._uri, _contacts_event.contact_id, _contacts_event.date, PrimitiveType_Long } },
  {"anniversaries.label",     { _contacts_event._uri, _contacts_event.contact_id, _contacts_event.label, PrimitiveType_String } },
  {"organizations.name",      { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.name, PrimitiveType_String } },
  {"organizations.department",{ _contacts_company._uri, _contacts_company.contact_id, _contacts_company.department, PrimitiveType_String } },
  {"organizations.title",     { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.job_title, PrimitiveType_String } },
  {"organizations.role",      { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.role, PrimitiveType_String } },
  {"organizations.logoURI",   { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.logo, PrimitiveType_String } },
  {"organizations.assistant", { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.assistant_name, PrimitiveType_String } },
  {"organizations.location",  { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.location, PrimitiveType_String } },
  {"organizations.description",  { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.description, PrimitiveType_String } },
  {"organizations.phoneticName", { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.phonetic_name, PrimitiveType_String } },
  {"organizations.type",      { _contacts_company._uri, _contacts_company.contact_id, _contacts_company.type, PrimitiveType_Long } },
  {"notes",                   { _contacts_note._uri, _contacts_note.contact_id, _contacts_note.note, PrimitiveType_String } },
  {"urls.url",                { _contacts_url._uri, _contacts_url.contact_id, _contacts_url.url, PrimitiveType_String } },
  {"urls.type",               { _contacts_url._uri, _contacts_url.contact_id, _contacts_url.type, PrimitiveType_Long } },
  {"ringtoneURI",             { _contacts_simple_contact._uri, _contacts_simple_contact.id, _contacts_simple_contact.ringtone_path, PrimitiveType_String } },
  {"groupIds",                { _contacts_group_relation._uri, _contacts_group_relation.contact_id, _contacts_group_relation.group_id, PrimitiveType_Long } }
};

// implementation ported from wrt-plugins-tizen

ContactSearchEngine::ContactSearchEngine()
    : addressbook_id_(0),
      is_addressbook_id_is_set_(false),
      is_filter_set_(false),
      is_sort_mode_set_(false),
      is_sort_mode_asc_(false) {
  LoggerD("Entered");
}

void ContactSearchEngine::SetAddressBookId(long id) {
  LoggerD("Entered");
  addressbook_id_ = id;
  is_addressbook_id_is_set_ = true;
}

common::PlatformResult ContactSearchEngine::ApplyFilter(const picojson::value& filter, int depth) {
  LoggerD("Entered");

  if (!filter.is<picojson::object>()) {
    return LogAndCreateResult(ErrorCode::TYPE_MISMATCH_ERR, "Failed to set filter");
  }

  common::FilterVisitor visitor;

  visitor.SetOnAttributeFilter([&depth, this](const std::string& name,
                                              common::AttributeMatchFlag match_flag,
                                              const picojson::value& match_value) {
    return ApplyAttributeFilter(name, match_flag, match_value, depth);
  });

  visitor.SetOnAttributeRangeFilter([&depth, this](const std::string& name,
                                                   const picojson::value& initial_value,
                                                   const picojson::value& end_value) {
    return ApplyAttributeRangeFilter(name, initial_value, end_value, depth);
  });

  visitor.SetOnCompositeFilterBegin([&depth, this](common::CompositeFilterType type) {
    LongSetPtrVectorPtr id_sets = LongSetPtrVectorPtr(new LongSetPtrVector());
    contact_id_set_array_stack_.push(id_sets);
    ++depth;

    return PlatformResult(ErrorCode::NO_ERROR);
  });

  visitor.SetOnCompositeFilterEnd([&depth, this](common::CompositeFilterType type) {
    --depth;
    auto id_sets = contact_id_set_array_stack_.top();
    contact_id_set_array_stack_.pop();

    LongSetPtr new_id_set = LongSetPtr(new LongSet());
    if (common::CompositeFilterType::kUnion == type) {
      GetUnion(id_sets, new_id_set);
    } else if (common::CompositeFilterType::kIntersection == type) {
      GetIntersection(id_sets, new_id_set);
    }

    if (!depth) {
      filtered_contact_ids_ = new_id_set;
    } else if (new_id_set) {
      contact_id_set_array_stack_.top()->push_back(new_id_set);
    }

    return PlatformResult(ErrorCode::NO_ERROR);
  });

  auto status = visitor.Visit(filter.get<picojson::object>());
  if (!status) return status;

  if (filtered_contact_ids_) {
    is_filter_set_ = true;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactSearchEngine::SetSortMode(const picojson::value& sort_mode) {
  LoggerD("Entered");

  if (!sort_mode.is<picojson::object>()) {
    return LogAndCreateResult(ErrorCode::TYPE_MISMATCH_ERR, "Failed to set sort mode");
  }

  std::string attribute = sort_mode.get("attributeName").to_str();

  const auto iter = s_properties_map_.find(attribute);
  if (s_properties_map_.end() == iter) {
    std::string msg = "SortMode doesn't support attribute: " + attribute;
    return LogAndCreateResult(ErrorCode::TYPE_MISMATCH_ERR, msg);
  }

  is_sort_mode_set_ = true;
  sort_mode_attribute_ = attribute;
  is_sort_mode_asc_ = sort_mode.get("order").to_str() == "ASC";

  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult ContactSearchEngine::Find(picojson::array* out) {
  LoggerD("Entered");

  if (is_filter_set_) {
    if (is_sort_mode_set_) {
      LongVectorPtr ids = LongVectorPtr(new LongVector());

      const auto iter = s_properties_map_.find(sort_mode_attribute_);
      const FilterPropertyStruct& property = iter->second;

      if (filtered_contact_ids_->size()) {
        SortContacts(property, ids, is_sort_mode_asc_, filtered_contact_ids_);
        return GetContacts(ids->begin(), ids->end(), out);
      } else {
        return PlatformResult(ErrorCode::NO_ERROR);
      }
    } else {
      if (filtered_contact_ids_->size()) {
        return GetContacts(filtered_contact_ids_->begin(), filtered_contact_ids_->end(), out);
      } else {
        return PlatformResult(ErrorCode::NO_ERROR);
      }
    }
  } else {
    if (is_sort_mode_set_) {
      const auto iter = s_properties_map_.find(sort_mode_attribute_);
      const FilterPropertyStruct& property = iter->second;

      return GetAllContactsSorted(property, is_sort_mode_asc_, out);
    } else {
      return GetAllContacts(out);
    }
  }
}

PlatformResult ContactSearchEngine::ApplyAttributeFilter(
    const std::string& name, common::AttributeMatchFlag match_flag,
    const picojson::value& match_value, int depth) {
  LoggerD("Entered");

  LongSetPtr id_set = LongSetPtr(new LongSet());

  if ("id" == name) {
    if (common::AttributeMatchFlag::kExists != match_flag) {
      id_set->insert(static_cast<long>(JsonCast<double>(match_value)));

      if (!depth) {
        filtered_contact_ids_ = id_set;
      } else {
        contact_id_set_array_stack_.top()->push_back(id_set);
      }
    }
    return PlatformResult(ErrorCode::NO_ERROR);
  } else if ("addresses.types" == name ||
             "emails.types" == name ||
             "phoneNumbers.types" == name ||
             "urls.type" == name) {
    if (!depth) {
      filtered_contact_ids_ = LongSetPtr();
    }
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  const auto iter =  s_properties_map_.find(name);
  if (s_properties_map_.end() == iter) {
    std::string msg = "Unknown attribute name: " + name;
    return LogAndCreateResult(ErrorCode::TYPE_MISMATCH_ERR, msg);
  }

  const auto& properties = iter->second;

  if (PrimitiveType_Boolean == properties.type) {
    bool value = true;
    if (common::AttributeMatchFlag::kExists != match_flag) {
      value = JsonCast<bool>(match_value);
    }

    auto status = QueryAttributeBool(properties, id_set, value);
    if (!status) return status;
  } else if (PrimitiveType_String == properties.type) {
    std::string value = "";

    if (common::AttributeMatchFlag::kExists != match_flag) {
      if ("photoURI" == name ||
          "ringtoneURI" == name ||
          "organizations.logoURI" == name) {
        value = ContactUtil::ConvertUriToPath(JsonCast<std::string>(match_value));
      } else {
        value = JsonCast<std::string>(match_value);
      }
    }

    contacts_match_str_flag_e flag = CONTACTS_MATCH_EXISTS;

    if (common::AttributeMatchFlag::kExactly == match_flag) {
      flag = CONTACTS_MATCH_EXACTLY;
    } else if (common::AttributeMatchFlag::kFullString == match_flag) {
      flag = CONTACTS_MATCH_FULLSTRING;
    } else if (common::AttributeMatchFlag::kContains == match_flag) {
      flag = CONTACTS_MATCH_CONTAINS;
    } else if (common::AttributeMatchFlag::kStartsWith == match_flag) {
      flag = CONTACTS_MATCH_STARTSWITH;
    } else if (common::AttributeMatchFlag::kEndsWith == match_flag) {
      flag = CONTACTS_MATCH_ENDSWITH;
    } else if (common::AttributeMatchFlag::kExists == match_flag) {
      flag = CONTACTS_MATCH_EXISTS;
    }

    auto status = QueryAttributeString(properties, id_set, flag, value.c_str());
    if (!status) return status;
  } else if (PrimitiveType_Long == properties.type) {
    int value = 0;

    if (common::AttributeMatchFlag::kExists != match_flag) {
      if ("lastUpdated" == name) {
        value = static_cast<int>(mktime(ToDateTm(match_value)));
      } else if ("birthday" == name || "anniversaries.date" == name) {
        value = ToDateDbInt(*ToDateTm(match_value));
      } else {
        value = static_cast<long>(JsonCast<double>(match_value));
      }
    }

    contacts_match_int_flag_e flag;
    if (common::AttributeMatchFlag::kExists == match_flag) {
      flag = CONTACTS_MATCH_GREATER_THAN_OR_EQUAL;
      value = 0;
    } else if (common::AttributeMatchFlag::kStartsWith == match_flag ||
               common::AttributeMatchFlag::kContains == match_flag) {
      flag = CONTACTS_MATCH_GREATER_THAN_OR_EQUAL;
    } else if (common::AttributeMatchFlag::kEndsWith == match_flag) {
      flag = CONTACTS_MATCH_LESS_THAN_OR_EQUAL;
    } else {
      flag = CONTACTS_MATCH_EQUAL;
    }

    if ("birthday" == name || "anniversaries.date" == name) {
      auto status = QueryAttributeDate(name, properties, id_set, flag, value);
      if (!status) return status;
    } else {
      auto status = QueryAttributeInt(properties, id_set, flag, value);
      if (!status) return status;
    }
  }

  if (!depth) {
    filtered_contact_ids_ = id_set;
  } else {
    if (id_set) {
      contact_id_set_array_stack_.top()->push_back(id_set);
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactSearchEngine::ApplyAttributeRangeFilter(
    const std::string& name, const picojson::value& initial_value,
    const picojson::value& end_value, int depth) {
  LoggerD("Entered");

  bool initial_value_set = (!IsNull(initial_value));
  bool end_value_set = (!IsNull(end_value));

  if (!initial_value_set && !end_value_set) {
    if (!depth) {
      filtered_contact_ids_ = LongSetPtr();
    }
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  if ("addresses.types" == name ||
      "emails.types" == name ||
      "phoneNumbers.types" == name ||
      "urls.type" == name) {
    if (!depth) {
      filtered_contact_ids_ = LongSetPtr();
    }
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  const auto iter =  s_properties_map_.find(name);
  if (s_properties_map_.end() == iter) {
    std::string msg = "Unknown attribute name: " + name;
    return LogAndCreateResult(ErrorCode::TYPE_MISMATCH_ERR, msg);
  }

  LongSetPtr id_set = LongSetPtr(new LongSet());
  const FilterPropertyStruct& properties = iter->second;

  if (PrimitiveType_Boolean == properties.type) {
    bool initial_value_bool = false;
    bool end_value_bool = false;

    if (initial_value_set) {
      initial_value_bool = JsonCast<bool>(initial_value);
    }
    if (end_value_set) {
      end_value_bool = JsonCast<bool>(end_value);
    }

    auto status = QueryAttributeRangeBool(properties, id_set, initial_value_set,
                                          initial_value_bool, end_value_set,
                                          end_value_bool);
    if (!status) return status;
  } else if (PrimitiveType_String == properties.type) {
    const char* initial_value_str = nullptr;
    const char* end_value_str = nullptr;

    if (initial_value_set) {
      initial_value_str = JsonCast<std::string>(initial_value).c_str();
    }
    if (end_value_set) {
      end_value_str = JsonCast<std::string>(end_value).c_str();
    }

    auto status = QueryAttributeRangeString(properties, id_set,
                                            initial_value_str, end_value_str);
    if (!status) return status;
  } else if (PrimitiveType_Long == properties.type) {
    int initial_value_int = 0;
    int end_value_int = 0;

    if ("lastUpdated" == name) {
      if (initial_value_set) {
        initial_value_int = static_cast<int>(mktime(ToDateTm(initial_value)));
      }
      if (end_value_set) {
        end_value_int = static_cast<int>(mktime(ToDateTm(end_value)));
      }
    } else if ("birthday" == name || "anniversaries.date" == name) {
      if (initial_value_set) {
        initial_value_int = ToDateDbInt(*ToDateTm(initial_value));
      }
      if (end_value_set) {
        end_value_int = ToDateDbInt(*ToDateTm(end_value));
      }
    } else {
      if (initial_value_set) {
        initial_value_int = static_cast<long>(JsonCast<double>(initial_value));
      }
      if (end_value_set) {
        end_value_int = static_cast<long>(JsonCast<double>(end_value));
      }
    }

    if ("birthday" == name || "anniversaries.date" == name) {
      auto status = QueryAttributeRangeDate(name, properties, id_set,
                                            initial_value_set,
                                            initial_value_int, end_value_set,
                                            end_value_int);
      if (!status) return status;
    } else {
      auto status = QueryAttributeRangeInt(properties, id_set,
                                           initial_value_set, initial_value_int,
                                           end_value_set, end_value_int);
      if (!status) return status;
    }
  }

  if (!depth) {
    filtered_contact_ids_ = id_set;
  } else {
    contact_id_set_array_stack_.top()->push_back(id_set);
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactSearchEngine::GetAllContactsSorted(
    const FilterPropertyStruct& attribute_properties, bool is_ascending,
    picojson::array* out) {
  LoggerD("Entered");

  LongVectorPtr sorted_ids = LongVectorPtr(new LongVector());
  SortContacts(attribute_properties, sorted_ids, is_ascending);

  return GetContacts(sorted_ids->begin(), sorted_ids->end(), out);
}

PlatformResult ContactSearchEngine::GetAllContacts(picojson::array* out) {
  LoggerD("Entered");

  contacts_list_h list = nullptr;
  int error_code = 0;
  PlatformResult status(ErrorCode::NO_ERROR);

  if (!is_addressbook_id_is_set_) {
    error_code = contacts_db_get_all_records(_contacts_contact._uri, 0, 0,
                                             &list);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_db_get_all_records");
    if (!status) return status;
  } else {
    contacts_query_h query = nullptr;
    contacts_filter_h filter = nullptr;

    error_code = contacts_query_create(_contacts_contact._uri, &query);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_query_create");
    if (!status) return status;

    ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                             ContactUtil::ContactsQueryDeleter);

    error_code = contacts_filter_create(_contacts_contact._uri, &filter);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_create");
    if (!status) return status;

    ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                              ContactUtil::ContactsFilterDeleter);

    error_code = contacts_filter_add_int(filter,
                                         _contacts_contact.address_book_id,
                                         CONTACTS_MATCH_EQUAL,
                                         addressbook_id_);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;

    error_code = contacts_query_set_filter(query, filter);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_query_set_filter");
    if (!status) return status;

    error_code = contacts_db_get_records_with_query(query, 0, 0, &list);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_db_get_records_with_query");
    if (!status) return status;
  }

  ContactUtil::ContactsListHPtr list_ptr(&list,
                                         ContactUtil::ContactsListDeleter);

  int record_count = 0;
  error_code = contacts_list_get_count(list, &record_count);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_list_get_count");
  if (!status) return status;

  contacts_list_first(list);
  for (int i = 0; i < record_count; ++i) {
    contacts_record_h record = nullptr;

    error_code = contacts_list_get_current_record_p(list, &record);
    if (CONTACTS_ERROR_NONE != error_code || !record) {
      LoggerW("contacts_list_get_current_record_p() failed: %d", error_code);
      continue;
    }

    int id_value = 0;
    error_code = contacts_record_get_int(record, _contacts_contact.id, &id_value);
    status = ContactUtil::ErrorChecker(error_code, "Failed contacts_record_get_int");
    if (!status) return status;

    out->push_back(picojson::value(static_cast<double>(id_value)));

    error_code = contacts_list_next(list);

    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("contacts_list_next() failed: %d", error_code);
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

template<typename Iterator>
PlatformResult ContactSearchEngine::GetContacts(Iterator begin, Iterator end,
                                                picojson::array* out) {
  LoggerD("Entered");

  for (auto iter = begin; iter != end; ++iter) {
    const auto id = *iter;

    if (is_addressbook_id_is_set_) {
      contacts_record_h record = nullptr;
      int error_code = contacts_db_get_record(_contacts_contact._uri, id, &record);
      if (CONTACTS_ERROR_NONE != error_code) {
        LoggerE("Failed to get contact with ID: %d", id);
        continue;
      }

      ContactUtil::ContactsRecordHPtr record_ptr(&record,
                                                 ContactUtil::ContactsDeleter);
      int address_book_id = 0;

      error_code = contacts_record_get_int(record,
                                           _contacts_contact.address_book_id,
                                           &address_book_id);
      if (CONTACTS_ERROR_NONE != error_code) {
        LoggerE("Failed to get address book ID of contact with ID: %d", id);
        continue;
      }

      if (address_book_id != addressbook_id_) {
        LoggerE("Wrong address book");
        continue;
      }
    }

    out->push_back(picojson::value(static_cast<double>(id)));
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

common::PlatformResult ContactSearchEngine::GetQueryResults(
    contacts_query_h query, contacts_filter_h filter, unsigned int property_id,
    LongSetPtr result) {
  LoggerD("Entered");

  int error_code = contacts_query_set_filter(query, filter);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_set_filter");
  if (!status) return status;

  contacts_list_h list = nullptr;
  error_code = contacts_db_get_records_with_query(query, 0, 0, &list);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_db_get_records_with_query");
  if (!status) return status;

  ContactUtil::ContactsListHPtr list_ptr(&list,
                                         ContactUtil::ContactsListDeleter);

  int record_count = 0;
  error_code = contacts_list_get_count(list, &record_count);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_list_get_count");
  if (!status) return status;

  contacts_list_first(*list_ptr);
  for (int i = 0; i < record_count; ++i) {
    contacts_record_h record = nullptr;
    error_code = contacts_list_get_current_record_p(list, &record);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_list_get_current_record_p");
    if (!status) return status;

    int value = 0;
    error_code = contacts_record_get_int(record, property_id, &value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_record_get_int");
    if (!status) return status;

    result->insert(value);
    error_code = contacts_list_next(*list_ptr);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("Failed to get next contact list: %d", error_code);
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactSearchEngine::QueryAttributeBool(
    const FilterPropertyStruct& attribute_properties, LongSetPtr result,
    bool match_value) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  error_code = contacts_filter_add_bool(filter, property_id, match_value);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_add_bool");
  if (!status) return status;

  return GetQueryResults(query, filter, property_contact_id, result);
}

PlatformResult ContactSearchEngine::QueryAttributeInt(
    const FilterPropertyStruct& attribute_properties, LongSetPtr result,
    contacts_match_int_flag_e match, int match_value) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  error_code = contacts_filter_add_int(filter, property_id, match, match_value);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_add_int");
  if (!status) return status;

  return GetQueryResults(query, filter, property_contact_id, result);
}

PlatformResult ContactSearchEngine::QueryAttributeString(
    const FilterPropertyStruct& attribute_properties, LongSetPtr result,
    contacts_match_str_flag_e match, const char* match_value) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  if (_contacts_number.number == property_id
      && CONTACTS_MATCH_CONTAINS == match) {
    property_id = _contacts_number.normalized_number;
  }

  error_code = contacts_filter_add_str(filter, property_id, match, match_value);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_add_str");
  if (!status) return status;

  return GetQueryResults(query, filter, property_contact_id, result);
}

PlatformResult ContactSearchEngine::QueryAttributeDate(
    const std::string& attr_name,
    const FilterPropertyStruct& attribute_properties, LongSetPtr result,
    contacts_match_int_flag_e match, int match_value) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  error_code = contacts_filter_add_int(filter, property_id, match,
                                       match_value);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_add_int");
  if (!status) return status;

  if ("birthday" == attr_name) {
    error_code = contacts_filter_add_operator(filter,
                                              CONTACTS_FILTER_OPERATOR_AND);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_operator");
    if (!status) return status;

    error_code = contacts_filter_add_int(filter, _contacts_event.type,
                                         CONTACTS_MATCH_EQUAL,
                                         CONTACTS_EVENT_TYPE_BIRTH);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;
  } else if ("anniversaries.date" == attr_name) {
    error_code = contacts_filter_add_operator(filter,
                                              CONTACTS_FILTER_OPERATOR_AND);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_operator");
    if (!status) return status;

    error_code = contacts_filter_add_int(filter, _contacts_event.type,
                                         CONTACTS_MATCH_EQUAL,
                                         CONTACTS_EVENT_TYPE_ANNIVERSARY);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_operator");
    if (!status) return status;
  }

  return GetQueryResults(query, filter, property_contact_id, result);
}

PlatformResult ContactSearchEngine::QueryAttributeRangeBool(
    const FilterPropertyStruct& attribute_properties, LongSetPtr result,
    bool initial_value_is_set,
    bool initial_value, bool end_value_is_set,
    bool end_value) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  if (initial_value_is_set && end_value_is_set) {
    if (initial_value == end_value) {
      if (initial_value) {
        error_code = contacts_filter_add_bool(filter, property_id, true);
        status = ContactUtil::ErrorChecker(error_code,
                                           "Failed contacts_filter_add_bool");
        if (!status) return status;
      } else if (!end_value) {
        error_code = contacts_filter_add_bool(filter, property_id, false);
        status = ContactUtil::ErrorChecker(error_code,
                                           "Failed contacts_filter_add_bool");
        if (!status) return status;
      }
    }
  } else if (initial_value_is_set) {
    if (initial_value) {
      error_code = contacts_filter_add_bool(filter, property_id, true);
      status = ContactUtil::ErrorChecker(error_code,
                                         "Failed contacts_filter_add_bool");
      if (!status) return status;
    }
  } else if (end_value_is_set) {
    if (!end_value) {
      error_code = contacts_filter_add_bool(filter, property_id, false);
      status = ContactUtil::ErrorChecker(error_code,
                                         "Failed contacts_filter_add_bool");
      if (!status) return status;
    }
  }

  return GetQueryResults(query, filter, property_contact_id, result);
}

PlatformResult ContactSearchEngine::QueryAttributeRangeInt(
    const FilterPropertyStruct& attribute_properties, LongSetPtr result,
    bool initial_value_is_set,
    int initial_value, bool end_value_is_set, int end_value) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  if (initial_value_is_set && end_value_is_set) {
    contacts_filter_h sub_filter = nullptr;

    error_code = contacts_filter_create(view_uri, &sub_filter);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_create");
    if (!status) return status;

    ContactUtil::ContactsFilterPtr sub_filter_ptr(sub_filter,
                                                  ContactUtil::ContactsFilterDeleter);

    error_code = contacts_filter_add_int(sub_filter, property_id,
                                         CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,
                                         initial_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;

    error_code = contacts_filter_add_operator(sub_filter,
                                              CONTACTS_FILTER_OPERATOR_AND);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_operator");
    if (!status) return status;

    error_code = contacts_filter_add_int(sub_filter, property_id,
                                         CONTACTS_MATCH_LESS_THAN_OR_EQUAL,
                                         end_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;

    error_code = contacts_filter_add_filter(filter, sub_filter);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_filter");
    if (!status) return status;
  } else if (initial_value_is_set) {
    error_code = contacts_filter_add_int(filter, property_id,
                                         CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,
                                         initial_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;
  } else if (end_value_is_set) {
    error_code = contacts_filter_add_int(filter, property_id,
                                         CONTACTS_MATCH_LESS_THAN_OR_EQUAL,
                                         end_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;
  }

  return GetQueryResults(query, filter, property_contact_id, result);
}

PlatformResult ContactSearchEngine::QueryAttributeRangeString(
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      const char* initial_value, const char* end_value) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  if (initial_value && end_value) {
    contacts_filter_h sub_filter = nullptr;

    error_code = contacts_filter_create(view_uri, &sub_filter);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_create");
    if (!status) return status;

    ContactUtil::ContactsFilterPtr sub_filter_ptr(sub_filter,
                                                  ContactUtil::ContactsFilterDeleter);

    error_code = contacts_filter_add_str(sub_filter, property_id,
                                         CONTACTS_MATCH_STARTSWITH,
                                         initial_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_str");
    if (!status) return status;

    error_code = contacts_filter_add_operator(sub_filter,
                                              CONTACTS_FILTER_OPERATOR_AND);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_operator");
    if (!status) return status;

    error_code = contacts_filter_add_str(sub_filter, property_id,
                                         CONTACTS_MATCH_ENDSWITH, end_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_str");
    if (!status) return status;

    error_code = contacts_filter_add_filter(filter, sub_filter);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_filter");
    if (!status) return status;
  } else if (initial_value) {
    error_code = contacts_filter_add_str(filter, property_id,
                                         CONTACTS_MATCH_STARTSWITH,
                                         initial_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_str");
    if (!status) return status;
  } else if (end_value) {
    error_code = contacts_filter_add_str(filter, property_id,
                                         CONTACTS_MATCH_ENDSWITH, end_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_str");
    if (!status) return status;
  }

  return GetQueryResults(query, filter, property_contact_id, result);
}

PlatformResult ContactSearchEngine::QueryAttributeRangeDate(
      const std::string& attr_name,
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      bool initial_value_is_set, int initial_value, bool end_value_is_set,
      int end_value) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  if (initial_value_is_set && end_value_is_set) {
    error_code = contacts_filter_add_int(filter, property_id,
                                         CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,
                                         initial_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;

    error_code = contacts_filter_add_operator(filter,
                                              CONTACTS_FILTER_OPERATOR_AND);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_operator");
    if (!status) return status;

    error_code = contacts_filter_add_int(filter, property_id,
                                         CONTACTS_MATCH_LESS_THAN_OR_EQUAL,
                                         end_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;
  } else if (initial_value_is_set) {
    error_code = contacts_filter_add_int(filter, property_id,
                                         CONTACTS_MATCH_GREATER_THAN_OR_EQUAL,
                                         initial_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;
  } else if (end_value_is_set) {
    error_code = contacts_filter_add_int(filter, property_id,
                                         CONTACTS_MATCH_LESS_THAN_OR_EQUAL,
                                         end_value);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;
  }

  if ("birthday" == attr_name) {
    error_code = contacts_filter_add_operator(filter,
                                              CONTACTS_FILTER_OPERATOR_AND);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_operator");
    if (!status) return status;

    error_code = contacts_filter_add_int(filter, _contacts_event.type,
                                         CONTACTS_MATCH_EQUAL,
                                         CONTACTS_EVENT_TYPE_BIRTH);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;
  } else if ("anniversaries.date" == attr_name) {
    error_code = contacts_filter_add_operator(filter,
                                              CONTACTS_FILTER_OPERATOR_AND);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_operator");
    if (!status) return status;

    error_code = contacts_filter_add_int(filter, _contacts_event.type,
                                         CONTACTS_MATCH_EQUAL,
                                         CONTACTS_EVENT_TYPE_ANNIVERSARY);
    status = ContactUtil::ErrorChecker(error_code,
                                       "Failed contacts_filter_add_int");
    if (!status) return status;
  }

  return GetQueryResults(query, filter, property_contact_id, result);
}

PlatformResult ContactSearchEngine::SortContacts(
      const FilterPropertyStruct& attribute_properties, LongVectorPtr result,
      bool is_ascending, LongSetPtr ids) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_filter_h filter = nullptr;
  contacts_list_h list = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_filter_create(view_uri, &filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_filter_create");
  if (!status) return status;

  ContactUtil::ContactsFilterPtr filter_ptr(filter,
                                            ContactUtil::ContactsFilterDeleter);

  auto iter = ids->begin();
  if (ids->end() != iter) {
    error_code = contacts_filter_add_int(filter, property_contact_id,
                                         CONTACTS_MATCH_EQUAL, *iter);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("contacts_filter_add_int() failed: %d", error_code);
    }
    for (; iter != ids->end(); ++iter) {
      error_code = contacts_filter_add_operator(filter,
                                                CONTACTS_FILTER_OPERATOR_OR);
      status = ContactUtil::ErrorChecker(error_code,
                                         "Failed contacts_filter_add_operator");
      if (!status) return status;

      error_code = contacts_filter_add_int(filter, property_contact_id,
                                           CONTACTS_MATCH_EQUAL, *iter);
      status = ContactUtil::ErrorChecker(error_code,
                                         "Failed contacts_filter_add_int");
      if (!status) return status;
    }
  }

  error_code = contacts_query_set_sort(query, property_id, is_ascending);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_query_set_sort");
  if (!status) return status;

  error_code = contacts_query_set_filter(query, filter);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_query_set_filter");
  if (!status) return status;

  error_code = contacts_db_get_records_with_query(query, 0, 0, &list);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_db_get_records_with_query");
  if (!status) return status;

  ContactUtil::ContactsListHPtr list_ptr(&list,
                                         ContactUtil::ContactsListDeleter);

  int record_count = 0;
  error_code = contacts_list_get_count(list, &record_count);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_list_get_count");
  if (!status) return status;

  LongSet overlapping_ids;

  contacts_list_first(*list_ptr);
  for (int i = 0; i < record_count; ++i) {
    contacts_record_h record;
    error_code = contacts_list_get_current_record_p(list, &record);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("contacts_list_get_current_record_p() failed: %d", error_code);
      continue;
    }

    int value = 0;
    error_code = contacts_record_get_int(record, property_contact_id, &value);
    LoggerW("contacts_record_get_int() failed: %d", error_code);
    if (CONTACTS_ERROR_NONE != error_code) {
      continue;
    }

    if (overlapping_ids.find(value) == overlapping_ids.end()) {
      result->push_back(value);
      overlapping_ids.insert(value);
    }

    error_code = contacts_list_next(list);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("contacts_list_next() failed: %d", error_code);
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult ContactSearchEngine::SortContacts(
      const FilterPropertyStruct& attribute_properties, LongVectorPtr result,
      bool is_ascending) {
  LoggerD("Entered");

  const char* view_uri = attribute_properties.view_uri;
  unsigned int property_contact_id = attribute_properties.property_contact_id;
  unsigned int property_id = attribute_properties.property_id;

  contacts_query_h query = nullptr;
  contacts_list_h list = nullptr;

  int error_code = contacts_query_create(view_uri, &query);
  auto status = ContactUtil::ErrorChecker(error_code,
                                          "Failed contacts_query_create");
  if (!status) return status;

  ContactUtil::ContactsQueryHPtr query_ptr(&query,
                                           ContactUtil::ContactsQueryDeleter);

  error_code = contacts_query_set_sort(query, property_id, is_ascending);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_query_set_sort");
  if (!status) return status;

  error_code = contacts_db_get_records_with_query(query, 0, 0, &list);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_db_get_records_with_query");
  if (!status) return status;

  ContactUtil::ContactsListHPtr list_ptr(&list,
                                         ContactUtil::ContactsListDeleter);

  int record_count = 0;
  error_code = contacts_list_get_count(list, &record_count);
  status = ContactUtil::ErrorChecker(error_code,
                                     "Failed contacts_list_get_count");
  if (!status) return status;

  LongSet overlapping_ids;

  contacts_list_first(*list_ptr);
  for (int i = 0; i < record_count; ++i) {
    contacts_record_h record;
    error_code = contacts_list_get_current_record_p(list, &record);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("contacts_list_get_current_record_p() failed: %d", error_code);
      continue;
    }

    int value = 0;
    error_code = contacts_record_get_int(record, property_contact_id, &value);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("contacts_record_get_int() failed: %d", error_code);
      continue;
    }

    if (overlapping_ids.find(value) == overlapping_ids.end()) {
      result->push_back(value);
      overlapping_ids.insert(value);
    }

    error_code = contacts_list_next(list);
    if (CONTACTS_ERROR_NONE != error_code) {
      LoggerW("contacts_list_next() failed: %d", error_code);
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void ContactSearchEngine::GetIntersection(LongSetPtrVectorPtr idSets,
                                          LongSetPtr result) {
  LongSetPtrVector::iterator i;

  if (idSets->size() == 0) {
    result = LongSetPtr();
    return;
  } else if (idSets->size() == 1) {
    *result = **(idSets->begin());
    return;
  }

  LongSetPtrVector::iterator minIter;
  LongSetPtrVector::size_type minSize = std::numeric_limits<
      LongSetPtrVector::size_type>::max();

  for (i = idSets->begin(); i != idSets->end(); i++) {
    LongSetPtr idSet = *i;
    LongSetPtrVector::size_type size = idSet->size();
    if (minSize > size) {
      minSize = size;
      minIter = i;
    }
  }

  LongSetPtr p = *minIter;
  LongSetPtrVectorPtr sa = LongSetPtrVectorPtr(new LongSetPtrVector());
  for (i = idSets->begin(); i != idSets->end(); i++) {
    if (minIter != i) {
      sa->push_back(*i);
    }
  }

  for (auto iter = p->begin(); iter != p->end(); ++iter) {
    bool excluded = false;
    int value = *iter;

    for (i = sa->begin(); i != sa->end(); i++) {
      LongSetPtr id_set = *i;
      if (id_set->find(value) == id_set->end()) {
        excluded = true;
        break;
      }
    }

    if (!excluded) {
      result->insert(value);
    }
  }
}

void ContactSearchEngine::GetUnion(LongSetPtrVectorPtr id_sets,
                                   LongSetPtr result) {
  if (!id_sets->size()) {
    result = LongSetPtr();
    return;
  } else if (1 == id_sets->size()) {
    *result = **(id_sets->begin());
    return;
  }

  for (auto i = id_sets->begin(); i != id_sets->end(); ++i) {
    LongSetPtr ids = *i;
    for (auto j = ids->begin(); j != ids->end(); ++j) {
      result->insert(*j);
    }
  }
}

}  // namespace contact
}  // namespace extension
