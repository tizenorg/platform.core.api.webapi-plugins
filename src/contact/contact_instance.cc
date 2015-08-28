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

#include "contact/contact_instance.h"

#include "common/converter.h"
#include "common/task-queue.h"
#include "common/logger.h"
#include "common/platform_exception.h"

#include "contact/addressbook.h"
#include "contact/contact_manager.h"
#include "contact/person.h"

namespace extension {
namespace contact {

using namespace common;

ContactInstance::ContactInstance()
    : current_state_(0),
      is_listening_(false) {
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&ContactInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&ContactInstance::x, this, _1, _2));

  // Contact Manager
  REGISTER_ASYNC("ContactManager_getAddressBooks",
                 ContactManagerGetAddressBooks);
  REGISTER_SYNC("ContactManager_getAddressBook", ContactManagerGetAddressBook);
  REGISTER_SYNC("ContactManager_addAddressBook", ContactManagerAddAddressBook);
  REGISTER_SYNC("ContactManager_removeAddressBook",
                ContactManagerRemoveAddressBook);
  REGISTER_SYNC("ContactManager_get", ContactManagerGet);
  REGISTER_SYNC("ContactManager_update", ContactManagerUpdate);
  REGISTER_ASYNC("ContactManager_updateBatch", ContactManagerUpdateBatch);
  REGISTER_SYNC("ContactManager_remove", ContactManagerRemove);
  REGISTER_ASYNC("ContactManager_removeBatch", ContactManagerRemoveBatch);
  REGISTER_ASYNC("ContactManager_find", ContactManagerFind);
  REGISTER_SYNC("ContactManager_importFromVCard",
                ContactManagerImportFromVCard);
  REGISTER_SYNC("ContactManager_startListening", ContactManagerStartListening);
  REGISTER_SYNC("ContactManager_stopListening", ContactManagerStopListening);

  // AddressBook
  REGISTER_ASYNC("AddressBook_addBatch", AddressBookAddBatch);
  REGISTER_ASYNC("AddressBook_updateBatch", AddressBookUpdateBatch);
  REGISTER_ASYNC("AddressBook_removeBatch", AddressBookRemoveBatch);
  REGISTER_SYNC("AddressBook_get", AddressBookGet);
  REGISTER_SYNC("AddressBook_add", AddressBookAdd);
  REGISTER_SYNC("AddressBook_update", AddressBookUpdate);
  REGISTER_SYNC("AddressBook_remove", AddressBookRemove);
  REGISTER_ASYNC("AddressBook_find", AddressBookFind);
  REGISTER_SYNC("AddressBook_addGroup", AddressBookAddGroup);
  REGISTER_SYNC("AddressBook_getGroup", AddressBookGetGroup);
  REGISTER_SYNC("AddressBook_updateGroup", AddressBookUpdateGroup);
  REGISTER_SYNC("AddressBook_removeGroup", AddressBookRemoveGroup);
  REGISTER_SYNC("AddressBook_getGroups", AddressBookGetGroups);
  REGISTER_SYNC("AddressBook_startListening", AddressBookStartListening);
  REGISTER_SYNC("AddressBook_stopListening", AddressBookStopListening);

  // Person
  REGISTER_SYNC("Person_link", PersonLink);
  REGISTER_SYNC("Person_unlink", PersonUnlink);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
}

ContactInstance::~ContactInstance() {
  if (is_listening_) {
    AddressBook::AddressBookStopListening(*this);
    set_is_listening(false);
  }
}

void ContactInstance::AddressBookGet(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookGet(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookAdd(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookAdd(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookAddBatch(const JsonValue& args,
                                          JsonObject& out) {
  LoggerD("entered");

  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    JsonValue result = JsonValue(JsonArray());
    PlatformResult status = AddressBook::AddressBookAddBatch(
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());
    if (status.IsSuccess())
      ReportSuccess(result, response->get<JsonObject>());
    else
      ReportError(status, &response->get<JsonObject>());
  };

  auto get_response =
      [this, callback_id](const std::shared_ptr<JsonValue>& response) {
    JsonObject& obj = response->get<JsonObject>();
    obj["callbackId"] = picojson::value(static_cast<double>(callback_id));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::AddressBookRemoveBatch(const JsonValue& args,
                                             JsonObject& out) {
  LoggerD("entered");

  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    JsonValue result = JsonValue(JsonArray());
    // TODO all batch operations should be implemented using CAPI batch
    // functions
    PlatformResult status = AddressBook::AddressBookBatchFunc(
        AddressBook::AddressBookRemove, "id",
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<JsonObject>());
    else
      ReportError(status, &response->get<JsonObject>());
  };

  auto get_response =
      [this, callback_id](const std::shared_ptr<JsonValue>& response) {
    JsonObject& obj = response->get<JsonObject>();
    obj["callbackId"] = picojson::value(static_cast<double>(callback_id));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::AddressBookUpdateBatch(const JsonValue& args,
                                             JsonObject& out) {
  LoggerD("entered");

  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    JsonValue result = JsonValue(JsonArray());
    // TODO all batch operations should be implemented using CAPI batch
    // functions
    PlatformResult status = AddressBook::AddressBookBatchFunc(
        AddressBook::AddressBookUpdate, "contact",
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<JsonObject>());
    else
      ReportError(status, &response->get<JsonObject>());
  };

  auto get_response =
      [this, callback_id](const std::shared_ptr<JsonValue>& response) {
    JsonObject& obj = response->get<JsonObject>();
    obj["callbackId"] = picojson::value(static_cast<double>(callback_id));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::AddressBookUpdate(const JsonValue& args,
                                        JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookUpdate(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookRemove(const JsonValue& args,
                                        JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookRemove(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookFind(const JsonValue& args, JsonObject& out) {
  LoggerD("entered");

  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    JsonValue result = JsonValue(JsonArray());
    PlatformResult status = AddressBook::AddressBookFind(
        JsonCast<JsonObject>(args), result.get<JsonArray>());
    if (status.IsSuccess())
      ReportSuccess(result, response->get<JsonObject>());
    else
      ReportError(status, &response->get<JsonObject>());
  };

  auto get_response =
      [this, callback_id](const std::shared_ptr<JsonValue>& response) {
    JsonObject& obj = response->get<JsonObject>();
    obj["callbackId"] = picojson::value(static_cast<double>(callback_id));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::AddressBookAddGroup(const JsonValue& args,
                                          JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookAddGroup(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookGetGroup(const JsonValue& args,
                                          JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookGetGroup(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookUpdateGroup(const JsonValue& args,
                                             JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookUpdateGroup(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookRemoveGroup(const JsonValue& args,
                                             JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookRemoveGroup(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookGetGroups(const JsonValue& args,
                                           JsonObject& out) {
  JsonValue val{JsonArray{}};
  PlatformResult status = AddressBook::AddressBookGetGroups(
      common::JsonCast<JsonObject>(args), val.get<JsonArray>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerGetAddressBooks(const JsonValue& args,
                                                    JsonObject& out) {
  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    JsonValue result = JsonValue(JsonArray());
    PlatformResult status = ContactManager::ContactManagerGetAddressBooks(
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<JsonObject>());
    else
      ReportError(status, &response->get<JsonObject>());
  };

  auto get_response =
      [this, callback_id](const std::shared_ptr<JsonValue>& response) {
    JsonObject& obj = response->get<JsonObject>();
    obj["callbackId"] = picojson::value(static_cast<double>(callback_id));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::ContactManagerGetAddressBook(const JsonValue& args,
                                                   JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerGetAddressBook(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerAddAddressBook(const JsonValue& args,
                                                   JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerAddAddressBook(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerRemoveAddressBook(const JsonValue& args,
                                                      JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerRemoveAddressBook(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookStartListening(const JsonValue& args,
                                                JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookStartListening(
      *this, common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::AddressBookStopListening(const JsonValue& args,
                                               JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = AddressBook::AddressBookStopListening(*this);
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerGet(const JsonValue& args,
                                        JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerGet(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerUpdate(const JsonValue& args,
                                           JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerUpdate(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerUpdateBatch(const JsonValue& args,
                                                JsonObject& out) {
  LoggerD("entered");

  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    JsonValue result = JsonValue(JsonArray());
    // TODO all batch operations should be implemented using CAPI batch
    // functions
    PlatformResult status = AddressBook::AddressBookBatchFunc(
        ContactManager::ContactManagerUpdate, "person",
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<JsonObject>());
    else
      ReportError(status, &response->get<JsonObject>());
  };

  auto get_response =
      [this, callback_id](const std::shared_ptr<JsonValue>& response) {
    JsonObject& obj = response->get<JsonObject>();
    obj["callbackId"] = picojson::value(static_cast<double>(callback_id));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::ContactManagerRemove(const JsonValue& args,
                                           JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerRemove(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerRemoveBatch(const JsonValue& args,
                                                JsonObject& out) {
  LoggerD("entered");

  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue>& response) -> void {
    JsonValue result = JsonValue(JsonArray());
    // TODO all batch operations should be implemented using CAPI batch
    // functions
    PlatformResult status = AddressBook::AddressBookBatchFunc(
        ContactManager::ContactManagerRemove, "personId",
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());

    if (status.IsSuccess())
      ReportSuccess(result, response->get<JsonObject>());
    else
      ReportError(status, &response->get<JsonObject>());
  };

  auto get_response =
      [this, callback_id](const std::shared_ptr<JsonValue>& response) {
    JsonObject& obj = response->get<JsonObject>();
    obj["callbackId"] = picojson::value(static_cast<double>(callback_id));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::ContactManagerFind(const JsonValue& args,
                                         JsonObject& out) {
  const double callback_id = args.get("callbackId").get<double>();

  auto get = [this, args](const std::shared_ptr<JsonValue>& response) -> void {
    JsonValue result = JsonValue(JsonArray());

    PlatformResult status = ContactManager::ContactManagerFind(
        common::JsonCast<JsonObject>(args), result.get<JsonArray>());
    if (status.IsSuccess()) {
      ReportSuccess(result, response->get<JsonObject>());
    } else {
      ReportError(status, &response->get<JsonObject>());
    }
  };

  auto get_response =
      [this, callback_id](const std::shared_ptr<JsonValue>& response) {
    JsonObject& obj = response->get<JsonObject>();
    obj["callbackId"] = picojson::value(static_cast<double>(callback_id));
    Instance::PostMessage(this, response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(
      get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::ContactManagerImportFromVCard(const JsonValue& args,
                                                    JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerImportFromVCard(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerStartListening(const JsonValue& args,
                                                   JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerStartListening(
      *this, common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::ContactManagerStopListening(const JsonValue& args,
                                                  JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = ContactManager::ContactManagerStopListening(
      *this, common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

void ContactInstance::PersonLink(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = Person::PersonLink(common::JsonCast<JsonObject>(args),
                                             val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(out);
  else
    ReportError(status, &out);
}

void ContactInstance::PersonUnlink(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  PlatformResult status = Person::PersonUnlink(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  if (status.IsSuccess())
    ReportSuccess(val, out);
  else
    ReportError(status, &out);
}

}  // namespace contact
}  // namespace extension
