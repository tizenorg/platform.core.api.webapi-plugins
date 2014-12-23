// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

ContactInstance& ContactInstance::GetInstance() {
    static ContactInstance instance;
    return instance;
}

int ContactInstance::current_state = 0;

ContactInstance::ContactInstance() {
  using namespace std::placeholders;

#define REGISTER_SYNC(c, x) \
  RegisterSyncHandler(c, std::bind(&ContactInstance::x, this, _1, _2));
#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&ContactInstance::x, this, _1, _2));

  // Contact Manager
  REGISTER_ASYNC("ContactManager_getAddressBooks", ContactManager_getAddressBooks);
  REGISTER_SYNC("ContactManager_getAddressBook", ContactManager_getAddressBook);
  REGISTER_SYNC("ContactManager_get", ContactManager_get);
  REGISTER_SYNC("ContactManager_update", ContactManager_update);
  REGISTER_SYNC("ContactManager_updateBatch", ContactManager_updateBatch);
  REGISTER_SYNC("ContactManager_remove", ContactManager_remove);
  REGISTER_SYNC("ContactManager_removeBatch", ContactManager_removeBatch);
  REGISTER_SYNC("ContactManager_find", ContactManager_find);
  REGISTER_SYNC("ContactManager_importFromVCard", ContactManager_importFromVCard);
  REGISTER_SYNC("ContactManager_startListening", ContactManager_startListening);
  REGISTER_SYNC("ContactManager_stopListening", ContactManager_stopListening);

  // AddressBook
  REGISTER_ASYNC("AddressBook_addBatch", AddressBook_addBatch);
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
  REGISTER_SYNC("AddressBook_startListening", AddressBook_startListening);
  REGISTER_SYNC("AddressBook_stopListening", AddressBook_stopListening);

  // Person
  REGISTER_SYNC("Person_link", Person_link);
  REGISTER_SYNC("Person_unlink", Person_unlink);

#undef REGISTER_SYNC
#undef REGISTER_ASYNC
}

ContactInstance::~ContactInstance() {}

void ContactInstance::AddressBook_get(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_get(common::JsonCast<JsonObject>(args),
                               val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_add(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_add(common::JsonCast<JsonObject>(args),
                               val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_addBatch(const JsonValue &args, JsonObject &out) {
  LoggerD("entered");
  // TODO check privileges

  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue>& response)->void {
      try {
        JsonValue result = JsonValue(JsonArray());
        AddressBook::AddressBook_addBatch(common::JsonCast<JsonObject>(args),
            result.get<JsonArray>());
        ReportSuccess(result, response->get<JsonObject>());
      } catch (const PlatformException& e) {
        ReportError(e, response->get<JsonObject>());
      }
  };

  auto get_response = [this, callback_id](const std::shared_ptr<JsonValue>& response) {
      JsonObject& obj = response->get<JsonObject>();
      obj.insert(std::make_pair("callbackId", callback_id));
      PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::AddressBook_update(const JsonValue& args,
                                         JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_update(common::JsonCast<JsonObject>(args),
                                  val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_remove(const JsonValue& args,
                                         JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_remove(common::JsonCast<JsonObject>(args),
                                  val.get<JsonObject>());
  ReportSuccess(out);
}

void ContactInstance::AddressBook_find(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_find(common::JsonCast<JsonObject>(args),
                                val.get<JsonObject>());
  ReportSuccess(out);
}

void ContactInstance::AddressBook_addGroup(const JsonValue& args,
                                           JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_addGroup(common::JsonCast<JsonObject>(args),
                                    val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_getGroup(const JsonValue& args,
                                           JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_getGroup(common::JsonCast<JsonObject>(args),
                                    val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_updateGroup(const JsonValue& args,
                                              JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_updateGroup(common::JsonCast<JsonObject>(args),
                                       val.get<JsonObject>());
  ReportSuccess(out);
}

void ContactInstance::AddressBook_removeGroup(const JsonValue& args,
                                              JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_removeGroup(common::JsonCast<JsonObject>(args),
                                       val.get<JsonObject>());
  ReportSuccess(out);
}

void ContactInstance::AddressBook_getGroups(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonArray{}};
  AddressBook::AddressBook_getGroups(common::JsonCast<JsonObject>(args),
                                     val.get<JsonArray>());
  ReportSuccess(val, out);
}

void ContactInstance::ContactManager_getAddressBooks(const JsonValue& args, JsonObject& out) {
  LoggerD("entered");

  // TODO check privileges

  const double callback_id = args.get("callbackId").get<double>();

  auto get = [=](const std::shared_ptr<JsonValue> &response) -> void {
    try {
      JsonValue result = JsonValue(JsonArray());
      ContactManager::ContactManager_getAddressBooks(common::JsonCast<JsonObject>(args),
          result.get<JsonArray>());
      ReportSuccess(result, response->get<picojson::object>());
    } catch (const PlatformException &e) {
      ReportError(e, response->get<picojson::object>());
    }
  };

  auto get_response = [this, callback_id](const std::shared_ptr<JsonValue> &response) {
    picojson::object &obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", callback_id));
    PostMessage(response->serialize().c_str());
  };

  TaskQueue::GetInstance().Queue<JsonValue>(get, get_response,
      std::shared_ptr<JsonValue>(new JsonValue(JsonObject())));
}

void ContactInstance::ContactManager_getAddressBook(const JsonValue& args,
                                                    JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_getAddressBook(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_startListening(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_startListening(common::JsonCast<JsonObject>(args),
                               val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::AddressBook_stopListening(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  AddressBook::AddressBook_stopListening(common::JsonCast<JsonObject>(args),
                               val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::ContactManager_get(const JsonValue& args,
                                         JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_get(common::JsonCast<JsonObject>(args),
                                     val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::ContactManager_update(const JsonValue& args,
                                            JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_update(common::JsonCast<JsonObject>(args),
                                        val.get<JsonObject>());
  ReportSuccess(out);
}

void ContactInstance::ContactManager_updateBatch(const JsonValue& args,
                                                 JsonObject& out) {
  // @todo implement
}

void ContactInstance::ContactManager_remove(const JsonValue& args,
                                            JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_remove(common::JsonCast<JsonObject>(args),
                                        val.get<JsonObject>());
  ReportSuccess(out);
}

void ContactInstance::ContactManager_removeBatch(const JsonValue& args,
                                                 JsonObject& out) {
  // @todo implement
}

void ContactInstance::ContactManager_find(const JsonValue& args,
                                          JsonObject& out) {
  // @todo implement
}

void ContactInstance::ContactManager_importFromVCard(const JsonValue& args,
                                                     JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_importFromVCard(
      common::JsonCast<JsonObject>(args), val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::ContactManager_startListening(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_startListening(common::JsonCast<JsonObject>(args),
                               val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::ContactManager_stopListening(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  ContactManager::ContactManager_stopListening(common::JsonCast<JsonObject>(args),
                               val.get<JsonObject>());
  ReportSuccess(val, out);
}

void ContactInstance::Person_link(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Person::Person_link(common::JsonCast<JsonObject>(args),
                      val.get<JsonObject>());
  ReportSuccess(out);
}

void ContactInstance::Person_unlink(const JsonValue& args, JsonObject& out) {
  JsonValue val{JsonObject{}};
  Person::Person_unlink(common::JsonCast<JsonObject>(args),
                        val.get<JsonObject>());
  ReportSuccess(out);
}

}  // namespace contact
}  // namespace extension
