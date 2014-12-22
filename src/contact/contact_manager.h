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

#ifndef CONTACT_CONTACT_MANAGER_H__
#define CONTACT_CONTACT_MANAGER_H__

#include "common/picojson.h"
#include "contact/contact_util.h"

namespace extension {
namespace contact {
namespace ContactManager {

void ContactManager_getAddressBooks(const JsonObject& args, JsonArray& out);

void ContactManager_getAddressBook(const JsonObject& args, JsonObject& out);

void ContactManager_get(const JsonObject& args, JsonObject& out);

void ContactManager_update(const JsonObject& args, JsonObject&);

void ContactManager_updateBatch(const JsonObject& /*args*/,
                                JsonObject& /*out*/);

void ContactManager_remove(const JsonObject& args, JsonObject&);

void ContactManager_removeBatch(const JsonObject& /*args*/,
                                JsonObject& /*out*/);

void ContactManager_find(const JsonObject& /*args*/, JsonObject& /*out*/);

void ContactManager_importFromVCard(const JsonObject& args, JsonObject& out);

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
void ContactManager_startListening(/*const JsonObject& args, JsonObject& out*/);

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
void ContactManager_stopListening(/*const JsonObject& args, JsonObject& out*/);

}  // namespace ContactManager
}  // namespace contact
}  // namespace extension

#endif  // CONTACT_CONTACT_MANAGER_H__
