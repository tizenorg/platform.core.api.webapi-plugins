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

#ifndef CONTACT_CONTACT_SEARCH_ENGINE_H_
#define CONTACT_CONTACT_SEARCH_ENGINE_H_

#include <memory>
#include <set>
#include <stack>
#include <vector>

#include <contacts.h>

#include "common/filter-utils.h"
#include "common/picojson.h"
#include "common/platform_result.h"
#include "contact/contact_util.h"

namespace extension {
namespace contact {

class ContactSearchEngine {
 public:
  ContactSearchEngine();

  void SetAddressBookId(long id);
  common::PlatformResult ApplyFilter(const picojson::value& filter, int depth = 0);
  common::PlatformResult SetSortMode(const picojson::value& sort_mode);

  common::PlatformResult Find(picojson::array* out);
  static common::PlatformResult GetPersonUsage(int person_id, JsonObject* out_ptr);

 private:
  typedef std::vector<long> LongVector;
  typedef std::shared_ptr<LongVector> LongVectorPtr;

  typedef std::set<long> LongSet;
  typedef std::shared_ptr<LongSet> LongSetPtr;

  typedef std::vector<LongSetPtr> LongSetPtrVector;
  typedef std::shared_ptr<LongSetPtrVector> LongSetPtrVectorPtr;

  enum PrimitiveType {
    PrimitiveType_Boolean,
    PrimitiveType_Long,
    PrimitiveType_String,
  };

  struct FilterPropertyStruct {
    const char* view_uri;
    const unsigned int property_contact_id;
    const unsigned int property_id;
    const PrimitiveType type;
  };

  typedef std::map<std::string, FilterPropertyStruct> PropertiesMap;

  common::PlatformResult ApplyAttributeFilter(
      const std::string& name, common::AttributeMatchFlag match_flag,
      const picojson::value& match_value, int depth);
  common::PlatformResult ApplyAttributeRangeFilter(
      const std::string& name, const picojson::value& initial_value,
      const picojson::value& end_value, int depth);

  common::PlatformResult GetQueryResults(contacts_query_h query,
                                         contacts_filter_h filter,
                                         unsigned int property_id,
                                         LongSetPtr result);

  common::PlatformResult GetAllContactsSorted(
      const FilterPropertyStruct& attribute_properties, bool is_ascending,
      picojson::array* out);
  common::PlatformResult GetAllContacts(picojson::array* out);
  template<typename Iterator>
  common::PlatformResult GetContacts(Iterator begin, Iterator end,
                                     picojson::array* out);
  common::PlatformResult QueryAttributeBool(
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      bool match_value);
  common::PlatformResult QueryAttributeInt(
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      contacts_match_int_flag_e match, int match_value);
  common::PlatformResult QueryAttributeString(
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      contacts_match_str_flag_e match, const char* match_value);
  common::PlatformResult QueryAttributeDate(
      const std::string& attr_name,
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      contacts_match_int_flag_e match, int match_value);
  common::PlatformResult QueryAttributeRangeBool(
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      bool initial_value_is_set, bool initial_value, bool end_value_is_set,
      bool end_value);
  common::PlatformResult QueryAttributeRangeInt(
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      bool initial_value_is_set, int initial_value, bool end_value_is_set,
      int end_value);
  common::PlatformResult QueryAttributeRangeString(
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      const char* initial_value, const char* end_value);
  common::PlatformResult QueryAttributeRangeDate(
      const std::string& attrName,
      const FilterPropertyStruct& attribute_properties, LongSetPtr result,
      bool initial_value_is_set, int initial_value, bool end_value_is_set,
      int end_value);
  common::PlatformResult SortContacts(
      const FilterPropertyStruct& attribute_properties, LongVectorPtr result,
      bool is_ascending, LongSetPtr ids);
  common::PlatformResult SortContacts(
      const FilterPropertyStruct& attribute_properties, LongVectorPtr result,
      bool is_ascending);

  void GetIntersection(LongSetPtrVectorPtr id_sets, LongSetPtr result);
  void GetUnion(LongSetPtrVectorPtr id_sets, LongSetPtr result);


  static PropertiesMap s_properties_map_;

  long addressbook_id_;
  bool is_addressbook_id_is_set_;
  bool is_filter_set_;
  bool is_sort_mode_set_;

  std::stack<LongSetPtrVectorPtr> contact_id_set_array_stack_;
  LongSetPtr filtered_contact_ids_;
  LongVectorPtr sorted_contact_ids_;

  std::string sort_mode_attribute_;
  bool is_sort_mode_asc_;
};

}  // namespace contact
}  // namespace extension

#endif  // CONTACT_CONTACT_SEARCH_ENGINE_H_
