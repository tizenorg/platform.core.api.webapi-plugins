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

#ifndef CONTACT_ADDRESSBOOK_H_
#define CONTACT_ADDRESSBOOK_H_

#include "common/picojson.h"
#include "contact/contact_util.h"
#include "functional"

namespace extension {
namespace contact {
namespace AddressBook {

typedef std::function<void(const JsonObject&, JsonObject&)> NativeFunction;

void AddressBookGet(const JsonObject& args, JsonObject& out);
void AddressBookAdd(const JsonObject& args, JsonObject& out);
void AddressBookUpdate(const JsonObject& args, JsonObject& out);
void AddressBookRemove(const JsonObject& args, JsonObject&);
void AddressBookFind(const JsonObject& args, JsonArray& array);
void AddressBookAddGroup(const JsonObject& args, JsonObject& out);
void AddressBookGetGroup(const JsonObject& args, JsonObject& out);
void AddressBookUpdateGroup(const JsonObject& args, JsonObject&);
void AddressBookRemoveGroup(const JsonObject& args, JsonObject&);
void AddressBookGetGroups(const JsonObject& args, JsonArray& out);
void AddressBookStartListening(const JsonObject& args, JsonObject& out);
void AddressBookStopListening(const JsonObject& args, JsonObject& out);

void AddressBookBatchFunc(NativeFunction impl, const char* single_arg_name,
                          const JsonObject& args, JsonArray& out);

// TODO all batch operations should be implemented using CAPI batch functions
void AddressBookAddBatch(const JsonObject& args, JsonArray& out);

}  // AddressBook
}  // contact
}  // extension

#endif  // CONTACT_ADDRESSBOOK_H_
