// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTACT_CONTACT_INSTANCE_H
#define CONTACT_CONTACT_INSTANCE_H

#include "common/extension.h"
#include "common/picojson.h"

#include "contact/contact_util.h"

namespace extension {
namespace contact {

class ContactInstance : public common::ParsedInstance {
 public:
  ContactInstance();
  static ContactInstance& GetInstance();
  static int current_state;
  virtual ~ContactInstance();

 private:
  void AddressBook_get(const JsonValue& args, JsonObject& out);
  void AddressBook_add(const JsonValue& args, JsonObject& out);
  void AddressBook_addBatch(const JsonValue& args, JsonObject& out);
  void AddressBook_update(const JsonValue& args, JsonObject& out);
  void AddressBook_updateBatch(const JsonValue& args, JsonObject& out);
  void AddressBook_remove(const JsonValue& args, JsonObject& out);
  void AddressBook_removeBatch(const JsonValue& args, JsonObject& out);
  void AddressBook_find(const JsonValue& args, JsonObject& out);

  void AddressBook_getGroup(const JsonValue& args, JsonObject& out);
  void AddressBook_getGroups(const JsonValue& args, JsonObject& out);
  void AddressBook_addGroup(const JsonValue& args, JsonObject& out);
  void AddressBook_updateGroup(const JsonValue& args, JsonObject& out);
  void AddressBook_removeGroup(const JsonValue& args, JsonObject& out);

  void AddressBook_startListening(const JsonValue& args, JsonObject& out);
  void AddressBook_stopListening(const JsonValue& args,JsonObject& out);

  /**
   * Signature: @code void getAddressBooks(successCallback, errorCallback);
   * @endcode
   * JSON: @code data: {method: 'ContactManager_getAddressBook', args: {}}
   * @endcode
   * Invocation: @code native.call(request, result_callback) @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {addressbooks}}
   * @endcode
   */
  void ContactManager_getAddressBooks(const JsonValue& args, JsonObject& out);

  /**
   * Signature: @code AddressBook getAddressBook(addressBookId); @endcode
   * JSON: @code data: {method: 'ContactManager_getAddressBook',
   *                    args: {addressBookID: addressBookId}} @endcode
   * Invocation: @code native.callSync(request) @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void ContactManager_getAddressBook(const JsonValue& args, JsonObject& out);

  /**
   * Signature: @code Person get(personId); @endcode
   * JSON: @code data: {method: 'ContactManager_get',
   *                    args: {personID: personId}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void ContactManager_get(const JsonValue& args, JsonObject& out);

  /**
   * Signature: @code void update(person); @endcode
   * JSON: @code data: {method: 'ContactManager_update',
   *                    args: {person: person}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void ContactManager_update(const JsonValue& args, JsonObject& out);

  /**
   * Signature: @code void updateBatch(persons, successCallback, errorCallback);
   * @endcode
   * JSON: @code data: {method: 'ContactManager_updateBatch',
   *                    args: {persons: persons}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {persons}}
   * @endcode
   */
  void ContactManager_updateBatch(const JsonValue &args, JsonObject &out);

  /**
   * Signature: @code void remove(personId); @endcode
   * JSON: @code data: {method: 'ContactManager_remove',
   *                    args: {personID: personId}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void ContactManager_remove(const JsonValue& args, JsonObject& out);

  /**
   * Signature: @code void removeBatch(personIds, successCallback, errorCallback);
   * @endcode
   * JSON: @code data: {method: 'ContactManager_removeBatch',
   *                    args: {personsIDs: personIds}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   */
  void ContactManager_removeBatch(const JsonValue &args, JsonObject &out);

  /**
   * Signature: @code void find(successCallback, errorCallback, filter, sortMode);
   * @endcode
   * JSON: @code data: {method: 'ContactManager_find',
   *                    args: {filter: filter, sortMode: sortMode}} @endcode
   * Invocation: @code native.call(request, result_callback); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success'}
   * @endcode
   * Result callback:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {persons}}
   * @endcode
   */
  void ContactManager_find(const JsonValue &args, JsonObject &out);

  /**
   * Signature: @code void getAddressBook(contactString); @endcode
   * JSON: @code data: {method: 'ContactManager_importFromVCard',
   *                    args: {contact: contactString}} @endcode
   * Invocation: @code native.callSync(request); @endcode
   * Return:
   * @code
   * {status: 'error', error: {name, message}}
   * {status: 'success', result: {contact}}
   * @endcode
   */
  void ContactManager_importFromVCard(const JsonValue& args, JsonObject& out);

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
  void ContactManager_startListening(const JsonValue& args, JsonObject& out);

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
  void ContactManager_stopListening(const JsonValue& args, JsonObject& out);


  void Person_link(const JsonValue& args, JsonObject& out);
  void Person_unlink(const JsonValue& args, JsonObject& out);
};
}  // namespace contact
}  // namespace extension

#endif  // CONTACT_CONTACT_INSTANCE_H
