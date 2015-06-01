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

#ifndef CONTACT_ADDRESSBOOK_H_
#define CONTACT_ADDRESSBOOK_H_

#include "common/picojson.h"
#include "contact/contact_util.h"
#include "functional"
#include "common/platform_result.h"

namespace extension {
namespace contact {

class ContactInstance;

namespace AddressBook {

typedef std::function<common::PlatformResult(const JsonObject&, JsonObject&)>
    NativeFunction;

common::PlatformResult AddressBookGet(const JsonObject& args, JsonObject& out);
common::PlatformResult AddressBookAdd(const JsonObject& args, JsonObject& out);
common::PlatformResult AddressBookUpdate(const JsonObject& args,
                                         JsonObject& out);
common::PlatformResult AddressBookRemove(const JsonObject& args, JsonObject&);
common::PlatformResult AddressBookFind(const JsonObject& args,
                                       JsonArray& array);
common::PlatformResult AddressBookAddGroup(const JsonObject& args,
                                           JsonObject& out);
common::PlatformResult AddressBookGetGroup(const JsonObject& args,
                                           JsonObject& out);
common::PlatformResult AddressBookUpdateGroup(const JsonObject& args,
                                              JsonObject&);
common::PlatformResult AddressBookRemoveGroup(const JsonObject& args,
                                              JsonObject&);
common::PlatformResult AddressBookGetGroups(const JsonObject& args,
                                            JsonArray& out);
common::PlatformResult AddressBookStartListening(ContactInstance& instance, const JsonObject& args,
                                                 JsonObject& out);
common::PlatformResult AddressBookStopListening(ContactInstance& instance);

common::PlatformResult AddressBookBatchFunc(NativeFunction impl,
                                            const char* single_arg_name,
                                            const JsonObject& args,
                                            JsonArray& out);

// TODO all batch operations should be implemented using CAPI batch functions
common::PlatformResult AddressBookAddBatch(const JsonObject& args,
                                           JsonArray& out);

}  // AddressBook
}  // contact
}  // extension

#endif  // CONTACT_ADDRESSBOOK_H_
