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
#include "common/native-plugin.h"
#include "contact/contact_util.h"

namespace extension {
namespace contact {
namespace AddressBook {

void AddressBook_get(const JsonObject& args, JsonObject& out);
void AddressBook_add(const JsonObject& args, JsonObject& out);
void AddressBook_update(const JsonObject& args, JsonObject& out);
void AddressBook_remove(const JsonObject& args, JsonObject& out);
void AddressBook_find(const JsonObject& args, JsonObject& out);
void AddressBook_addGroup(const JsonObject& args, JsonObject& out);
void AddressBook_getGroup(const JsonObject& args, JsonObject& out);
void AddressBook_updateGroup(const JsonObject& args, JsonObject& out);
void AddressBook_removeGroup(const JsonObject& args, JsonObject& out);
void AddressBook_getGroups(const JsonObject& args, JsonObject& out);
void AddressBook_startListening(int* current_state, const JsonObject& args, JsonObject& out);
void AddressBook_stopListening(int* current_state, const JsonObject& args, JsonObject& out);

void AddressBook_batchFunc(common::NativeFunction impl, const char* single_arg_name,
                           const JsonObject& args, JsonObject& out);

void AddressBook_addBatch(const JsonObject& args, JsonObject& out);

}  // AddressBook
}  // contact
}  // extension

#endif  // CONTACT_ADDRESSBOOK_H_
