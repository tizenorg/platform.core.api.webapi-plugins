// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "contact/contact_instance.h"

#include "common/converter.h"

#include "contact/addressbook.h"
#include "contact/contact_manager.h"
#include "contact/person.h"

namespace extension {
namespace contact {

ContactInstance::ContactInstance() {
  using namespace std::placeholders;
#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&PowerInstance::x, this, _1, _2));

  // AddressBook
  REGISTER_SYNC("AddressBook_get", AddressBook_get);
  REGISTER_SYNC("AddressBook_add", AddressBook_add);
  REGISTER_SYNC("AddressBook_update", AddressBook_update);
  REGISTER_SYNC("AddressBook_remove", AddressBook_remove);
  REGISTER_SYNC("AddressBook_find", AddressBook_find);
  REGISTER_SYNC("AddressBook_addGroup", AddressBook_addGroup);
  REGISTER_SYNC("AddressBook_getGroup", AddressBook_getGroup);
  REGISTER_SYNC("AddressBook_updateGroup", AddressBook_updateGroup);
  REGISTER_SYNC("AddressBook_removeGroup", AddressBook_removeGroup);
  REGISTER_SYNC("AddressBook_getGroups", AddressBook_getGroups);

  // Contact Manager
  REGISTER_SYNC("ContactManager_getAddressBooks",
                ContactManager_getAddressBooks);
  REGISTER_SYNC("ContactManager_getAddressBook", ContactManager_getAddressBook);
  REGISTER_SYNC("ContactManager_get", ContactManager_get);
  REGISTER_SYNC("ContactManager_update", ContactManager_update);
  REGISTER_SYNC("ContactManager_updateBatch", ContactManager_updateBatch);
  REGISTER_SYNC("ContactManager_remove", ContactManager_remove);
  REGISTER_SYNC("ContactManager_removeBatch", ContactManager_removeBatch);
  REGISTER_SYNC("ContactManager_find", ContactManager_find);
  REGISTER_SYNC("ContactManager_importFromVCard",
                ContactManager_importFromVCard);

  // Person
  REGISTER_SYNC("Person_link", Person_link);
  REGISTER_SYNC("Person_unlink", Person_unlink);

#undef REGISTER_SYNC
}

ContactInstance::~ContactInstance() {}

void ContactInstance::AddressBook_get(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_get(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_add(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_add(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_update(const JsonValue& args,
                                         JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_update(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_remove(const JsonValue& args,
                                         JsonObject& out) {
  AddressBook::AddressBook_remove(args);
  ReportSuccess(out);
}

void ContactInstance::AddressBook_find(const JsonValue& args, JsonObject& out) {
  AddressBook::AddressBook_find(args);
  ReportSuccess(out);
}

void ContactInstance::AddressBook_addGroup(const JsonValue& args,
                                           JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_addGroup(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_getGroup(const JsonValue& args,
                                           JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_getGroup(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_updateGroup(const JsonValue& args,
                                              JsonObject& out) {
  AddressBook::AddressBook_updateGroup(args);
  ReportSuccess(out);
}

void ContactInstance::AddressBook_removeGroup(const JsonValue& args,
                                              JsonObject& out) {
  AddressBook::AddressBook_removeGroup(args);
  ReportSuccess(out);
}

void ContactInstance::AddressBook_getGroups(const JsonValue& args,
                                            JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_getGroups(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::ContactManager_getAddressBooks(const JsonValue& args,
                                                     JsonObject& out) {
  // @todo
}

void ContactInstance::ContactManager_getAddressBook(const JsonValue& args,
                                                    JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_getAddressBook(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::ContactManager_get(const JsonValue& args,
                                         JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_get(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::ContactManager_update(const JsonValue& args,
                                            JsonObject& out) {
  ContactManager::ContactManager_update(args);
  ReportSuccess(out);
}

void ContactInstance::ContactManager_updateBatch(const JsonValue& args,
                                                 JsonObject& out) {
  // @todo
}

void ContactInstance::ContactManager_remove(const JsonValue& args,
                                            JsonObject& out) {
  ContactManager::ContactManager_remove(args);
  ReportSuccess(out);
}

void ContactInstance::ContactManager_removeBatch(const JsonValue& args,
                                                 JsonObject& out) {
  // @todo
}

void ContactInstance::ContactManager_find(const JsonValue& args,
                                          JsonObject& out) {
  // @todo
}

void ContactInstance::ContactManager_importFromVCard(const JsonValue& args,
                                                     JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_importFromVCard(args, val);
  ReportSuccess(val, out);
}

void ContactInstance::Person_link(const JsonValue& args, JsonObject& out) {
  Person::Person_link(args);
  ReportSuccess(out);
}

void ContactInstance::Person_unlink(const JsonValue& args, JsonObject& out) {
  Person::Person_unlink(args);
  ReportSuccess(out);
}

}  // namespace contact
}  // namespace extension
