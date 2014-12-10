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

#include <string>
#include <contacts.h>
#include "common/native-plugin.h"
#include "common/logger.h"
#include "contact/contact_util.h"
#include "contact/addressbook.h"
#include "contact/contact_manager.h"
#include "contact/person.h"

namespace extension {
namespace contact {

namespace {
void CheckReadPrivileges(const JsonObject& /*args*/, JsonObject& /*out*/) {
  common::NativePlugin::CheckAccess(ContactUtil::kContactReadPrivileges);
}
}

class ContactPlugin : public webapi::common::NativePlugin {
 public:
  ContactPlugin();
  ~ContactPlugin();
  virtual void OnLoad();

 private:
  bool connected_;
  int contact_listener_current_state_;
};

EXPORT_NATIVE_PLUGIN(extension::contact::ContactPlugin);

using namespace extension::common;

ContactPlugin::ContactPlugin()
    : connected_{false}, contact_listener_current_state_{0} {}

ContactPlugin::~ContactPlugin() {
  if (connected_) {
    contacts_disconnect();
  }
}

void ContactPlugin::OnLoad() {

  using namespace AddressBook;
  using namespace Person;
  using namespace ContactManager;

#define DISPATCHER_ADDFUNCTION(N) dispatcher_.AddFunction(#N, N);
  DISPATCHER_ADDFUNCTION(AddressBook_get);
  DISPATCHER_ADDFUNCTION(AddressBook_add);
  DISPATCHER_ADDFUNCTION(AddressBook_update);
  DISPATCHER_ADDFUNCTION(AddressBook_remove);
  DISPATCHER_ADDFUNCTION(AddressBook_find);
  DISPATCHER_ADDFUNCTION(AddressBook_getGroup);
  DISPATCHER_ADDFUNCTION(AddressBook_addGroup);
  DISPATCHER_ADDFUNCTION(AddressBook_updateGroup);
  DISPATCHER_ADDFUNCTION(AddressBook_removeGroup);
  DISPATCHER_ADDFUNCTION(AddressBook_getGroups);
  DISPATCHER_ADDFUNCTION(AddressBook_addBatch);

  DISPATCHER_ADDFUNCTION(ContactManager_getAddressBooks);
  DISPATCHER_ADDFUNCTION(ContactManager_find);
  DISPATCHER_ADDFUNCTION(ContactManager_get);
  DISPATCHER_ADDFUNCTION(ContactManager_getAddressBook);
  DISPATCHER_ADDFUNCTION(ContactManager_importFromVCard);
  DISPATCHER_ADDFUNCTION(ContactManager_remove);
  DISPATCHER_ADDFUNCTION(ContactManager_update);
  DISPATCHER_ADDFUNCTION(ContactManager_startListening);
  DISPATCHER_ADDFUNCTION(ContactManager_stopListening);

  DISPATCHER_ADDFUNCTION(CheckReadPrivileges);

  DISPATCHER_ADDFUNCTION(Person_link);
  DISPATCHER_ADDFUNCTION(Person_unlink);
#undef DISPATCHER_ADDFUNCTION

  using namespace std::placeholders;
  dispatcher_.AddFunction(
      "AddressBook_updateBatch",
      std::bind(AddressBook_batchFunc, AddressBook_update, "contact", _1, _2));
  dispatcher_.AddFunction(
      "AddressBook_removeBatch",
      std::bind(AddressBook_batchFunc, AddressBook_remove, "id", _1, _2));

  dispatcher_.AddFunction("ContactManager_updateBatch",
                          std::bind(AddressBook_batchFunc,
                                    ContactManager_update, "person", _1, _2));
  dispatcher_.AddFunction("ContactManager_removeBatch",
                          std::bind(AddressBook_batchFunc,
                                    ContactManager_remove, "personId", _1, _2));

  dispatcher_.AddFunction("AddressBook_startListening",
                          std::bind(AddressBook_startListening,
                                    &contact_listener_current_state_, _1, _2));
  dispatcher_.AddFunction("AddressBook_stopListening",
                          std::bind(AddressBook_stopListening,
                                    &contact_listener_current_state_, _1, _2));

  int err = contacts_connect();
  // TODO maybe do something if it fails
  connected_ = err == CONTACTS_ERROR_NONE;
  if (connected_) {
    LoggerI("Connection established!");
  } else {
    LoggerW("Connection error occured: " << err);
  }
}

}  // namespace contact
}  // namespace extension
