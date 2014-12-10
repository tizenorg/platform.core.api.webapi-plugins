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

#ifndef CONTACT_PERSON_H_
#define CONTACT_PERSON_H_

#include "common/picojson.h"
#include "common/filter-utils.h"

namespace extension {
namespace contact {
namespace Person {

struct PersonProperty {
    const unsigned int propertyId;
    const webapi::common::PrimitiveType type;
};

typedef std::map<std::string, PersonProperty> PersonPropertyMap;

const PersonProperty& PersonProperty_fromString(const std::string& name);

void Person_link(const JsonObject& args, JsonObject& out);
void Person_unlink(const JsonObject& args, JsonObject& out);

}  // Person
}  // contact
}  // extension

#endif  // CONTACT_PERSON_H
