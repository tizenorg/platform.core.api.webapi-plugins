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
 
#ifndef CONTACT_PERSON_H_
#define CONTACT_PERSON_H_

#include "common/picojson.h"
#include "contact/contact_util.h"

namespace extension {
namespace contact {

enum PrimitiveType {
  kPrimitiveTypeBoolean,
  kPrimitiveTypeString,
  kPrimitiveTypeLong,
  kPrimitiveTypeId
};

extern const std::string kUsageTypeOutgoingCall;
extern const std::string kUsageTypeOutgoingMsg;
extern const std::string kUsageTypeOutgoingEmail;
extern const std::string kUsageTypeIncomingCall;
extern const std::string kUsageTypeIncomingMsg;
extern const std::string kUsageTypeIncomingEmail;
extern const std::string kUsageTypeMissedCall;
extern const std::string kUsageTypeRejectedCall;
extern const std::string kUsageTypeBlockedCall;
extern const std::string kUsageTypeBlockedMsg;

namespace Person {

struct PersonProperty {
  unsigned int propertyId;
  PrimitiveType type;
};

typedef std::map<std::string, PersonProperty> PersonPropertyMap;

common::PlatformResult PersonPropertyFromString(const std::string& name,
                                                PersonProperty* person_prop);
contacts_usage_type_e UsageTypeFromString(const std::string& type_str);

common::PlatformResult PersonLink(const JsonObject& args, JsonObject&);
common::PlatformResult PersonUnlink(const JsonObject& args, JsonObject&);
common::PlatformResult GetUsageCount(const JsonObject& args, JsonObject&);
common::PlatformResult ResetUsageCount(const long& person_id, const std::string& type);
common::PlatformResult PersonResetUsageCount(const JsonObject& args);

}  // Person
}  // contact
}  // extension

#endif  // CONTACT_PERSON_H
