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
  virtual ~ContactInstance();

  int current_state() const { return current_state_; }
  void set_current_state(int state) { current_state_ = state; }

 private:
  void AddressBookGet(const JsonValue& args, JsonObject& out);
  void AddressBookAdd(const JsonValue& args, JsonObject& out);
  void AddressBookAddBatch(const JsonValue& args, JsonObject& out);
  void AddressBookUpdate(const JsonValue& args, JsonObject& out);
  void AddressBookUpdateBatch(const JsonValue& args, JsonObject& out);
  void AddressBookRemove(const JsonValue& args, JsonObject& out);
  void AddressBookRemoveBatch(const JsonValue& args, JsonObject& out);
  void AddressBookFind(const JsonValue& args, JsonObject& out);

  void AddressBookGetGroup(const JsonValue& args, JsonObject& out);
  void AddressBookGetGroups(const JsonValue& args, JsonObject& out);
  void AddressBookAddGroup(const JsonValue& args, JsonObject& out);
  void AddressBookUpdateGroup(const JsonValue& args, JsonObject& out);
  void AddressBookRemoveGroup(const JsonValue& args, JsonObject& out);

  void AddressBookStartListening(const JsonValue& args, JsonObject& out);
  void AddressBookStopListening(const JsonValue& args, JsonObject& out);

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
  void ContactManagerGetAddressBooks(const JsonValue& args, JsonObject& out);

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
  void ContactManagerGetAddressBook(const JsonValue& args, JsonObject& out);

  void ContactManagerAddAddressBook(const JsonValue& args, JsonObject& out);
  void ContactManagerRemoveAddressBook(const JsonValue& args, JsonObject& out);

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
  void ContactManagerGet(const JsonValue& args, JsonObject& out);

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
  void ContactManagerUpdate(const JsonValue& args, JsonObject& out);

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
  void ContactManagerUpdateBatch(const JsonValue& args, JsonObject& out);

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
  void ContactManagerRemove(const JsonValue& args, JsonObject& out);

  /**
   * Signature: @code void removeBatch(personIds, successCallback,
   * errorCallback);
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
  void ContactManagerRemoveBatch(const JsonValue& args, JsonObject& out);

  /**
   * Signature: @code void find(successCallback, errorCallback, filter,
   * sortMode);
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
  void ContactManagerFind(const JsonValue& args, JsonObject& out);

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
  void ContactManagerImportFromVCard(const JsonValue& args, JsonObject& out);

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
  void ContactManagerStartListening(const JsonValue& args, JsonObject& out);

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
  void ContactManagerStopListening(const JsonValue& args, JsonObject& out);

  void PersonLink(const JsonValue& args, JsonObject& out);
  void PersonUnlink(const JsonValue& args, JsonObject& out);

  int current_state_;
};
}  // namespace contact
}  // namespace extension

#endif  // CONTACT_CONTACT_INSTANCE_H
