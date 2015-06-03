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

#ifndef CONTACT_CONTACT_MANAGER_H__
#define CONTACT_CONTACT_MANAGER_H__

#include "common/picojson.h"
#include "contact/contact_util.h"

namespace extension {
namespace contact {

class ContactInstance;

namespace ContactManager {

common::PlatformResult ContactManagerGetAddressBooks(const JsonObject& args,
                                                     JsonArray& out);

common::PlatformResult ContactManagerGetAddressBook(const JsonObject& args,
                                                    JsonObject& out);

common::PlatformResult ContactManagerAddAddressBook(const JsonObject& args,
                                                    JsonObject& out);

common::PlatformResult ContactManagerRemoveAddressBook(const JsonObject& args,
                                                       JsonObject& out);

common::PlatformResult ContactManagerGet(const JsonObject& args,
                                         JsonObject& out);

common::PlatformResult ContactManagerUpdate(const JsonObject& args,
                                            JsonObject&);

common::PlatformResult ContactManagerRemove(const JsonObject& args,
                                            JsonObject&);

common::PlatformResult ContactManagerFind(const JsonObject& args,
                                          JsonArray& out);

common::PlatformResult ContactManagerImportFromVCard(const JsonObject& args,
                                                     JsonObject& out);

/**
 * Signature: @code void getAddressBook(contactString); @endcode
 * JSON: @code data: {method: 'ContactManager_startListening',
 *                    args: {}} @endcode
 * Invocation: @code native.callSync(request); @endcode
 * Return:
 * @code
 * {status: 'error', error: {name, message}}
 * {status: 'success'}
 * @endcode
 */
common::PlatformResult ContactManagerStartListening(ContactInstance& instance, const JsonObject& args,
                                                    JsonObject& out);

/**
 * Signature: @code void getAddressBook(contactString); @endcode
 * JSON: @code data: {method: 'ContactManager_stopListening',
 *                    args: {}} @endcode
 * Invocation: @code native.callSync(request); @endcode
 * Return:
 * @code
 * {status: 'error', error: {name, message}}
 * {status: 'success'}
 * @endcode
 */
common::PlatformResult ContactManagerStopListening(ContactInstance& instance, const JsonObject& args,
                                                   JsonObject& out);

}  // namespace ContactManager
}  // namespace contact
}  // namespace extension

#endif  // CONTACT_CONTACT_MANAGER_H__
