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

#include "secureelement/secureelement_seservice.h"

#include "common/logger.h"
#include "common/task-queue.h"
#include "common/platform_result.h"


#include "secureelement/secureelement_instance.h"

using namespace smartcard_service_api;
using namespace common;
using namespace tools;

namespace extension {
namespace secureelement {

namespace {
const int SE_INSERTED = 1;
const int SE_REMOVED = 2;
}

class SEServiceEventHandler : public SEServiceListener {
  void serviceConnected(SEServiceHelper *service, void *context) {
    LoggerD("Entered");
    if (context) {
      (static_cast<SEService*>(context))->ServiceConnected();
    } else {
      LoggerE("Context is null");
    }
  }

  void eventHandler(SEServiceHelper *service, char *se_name, int event, void *context) {
    LoggerD("Entered");
    if (context) {
      (static_cast<SEService*>(context))->EventHandler(se_name, event);
    } else {
      LoggerE("Context is null");
    }
  }

  void errorHandler(SEServiceHelper *service, int error, void *context) {
    LoggerE("Error handler called, code: %d", error);
    if (context) {
      (static_cast<SEService*>(context))->ErrorHandler(error);
    } else {
      LoggerE("Context is null");
    }
  }
};

static SEServiceEventHandler se_event_handler;

SEService::SEService(SecureElementInstance& instance)
    : is_initialized_(false),
      is_listener_set_(false),
      is_error_(false),
      instance_(instance) {
  LoggerD("Entered");

  se_service_ = new smartcard_service_api::SEService((void *)this, &se_event_handler);
}

SEService::~SEService() {
  LoggerD("Entered");

  if (is_initialized_) {
    se_service_->shutdownSync();
    is_initialized_ = false;
  }

  delete se_service_;
  se_service_ = nullptr;
}

void SEService::GetReaders(double callback_id) {
  LoggerD("Entered");

  auto get_readers = [this](
      const std::shared_ptr<picojson::value>& response) -> void {
    picojson::value result = picojson::value(picojson::array());
    picojson::array& result_array = result.get<picojson::array>();

    std::vector<smartcard_service_api::ReaderHelper *> readers = se_service_->getReaders();
    for (std::size_t i = 0; i < readers.size(); i++) {
      if (readers[i]->isSecureElementPresent()) {
        result_array.push_back(picojson::value((double) (long) readers[i]));
      }
    }

    ReportSuccess(result, response->get<picojson::object>());
  };

  auto get_readers_response = [this, callback_id](
      const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    Instance::PostMessage(&instance_, response->serialize().c_str());
  };

  if (is_error_) {
    // there has been an error, report it asynchronously
    LoggerE("Failed: is_error_");
    std::shared_ptr<picojson::value> response{new picojson::value{picojson::object{}}};
    ReportError(
        PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR,
                       "Unable to connect to service."),
        &response->get<picojson::object>());
    TaskQueue::GetInstance().Async<picojson::value>(get_readers_response, response);
    return;
  }

  if (!is_initialized_) {
    // not yet ready, wait for the platform callback, send the response then
    get_readers_callbacks_.push_back(callback_id);
    return;
  }

  // everything's fine, get the readers, send the response
  TaskQueue::GetInstance().Queue<picojson::value>(
      get_readers,
      get_readers_response,
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

void SEService::RegisterSEListener() {
  LoggerD("Entered");

  is_listener_set_ = true;
}

void SEService::UnregisterSEListener() {
  LoggerD("Entered");

  is_listener_set_ = false;
}

void SEService::Shutdown() {
  LoggerD("Entered");

  if (is_initialized_) {
      se_service_->shutdownSync();
      is_initialized_ = false;
  }
}

void SEService::ServiceConnected() {
  LoggerD("Entered");

  is_initialized_ = true;

  // send the response to pending GetReaders callbacks
  for (const auto& callback_id : get_readers_callbacks_) {
    GetReaders(callback_id);
  }
  get_readers_callbacks_.clear();

  // notify the listeners
  if (!is_listener_set_) {
    LoggerE("SE listener is not set.");
    return;
  }

  std::vector<smartcard_service_api::ReaderHelper *> readers = se_service_->getReaders();
  for (std::size_t i = 0; i < readers.size(); i++) {
    if (readers[i]->isSecureElementPresent()) {
      picojson::value result = picojson::value(picojson::object());
      picojson::object& obj = result.get<picojson::object>();

      obj.insert(std::make_pair("action", picojson::value("onSEReady")));
      obj.insert(std::make_pair("handle", picojson::value((double) (long) readers[i])));

      Instance::PostMessage(&instance_, result.serialize().c_str());
    }
  }
}

void SEService::EventHandler(char *se_name, int event) {
  LoggerD("Entered");

  if (SE_INSERTED != event && SE_REMOVED != event) {
    LoggerD("Incorrect event type");
    return;
  }

  if (is_initialized_ && is_listener_set_) {
    bool is_present = event == SE_INSERTED;

    std::vector<smartcard_service_api::ReaderHelper *> readers = se_service_->getReaders();
    for (std::size_t i = 0; i < readers.size(); i++) {
      if (!strcmp(se_name, readers[i]->getName()) &&
          readers[i]->isSecureElementPresent() != is_present) {
        picojson::value result = picojson::value(picojson::object());
        picojson::object& obj = result.get<picojson::object>();

        if (is_present) {
          obj.insert(std::make_pair("action", picojson::value("onSEReady")));
        } else {
          obj.insert(std::make_pair("action", picojson::value("onSENotReady")));
        }

        obj.insert(std::make_pair("handle", picojson::value((double) (long) readers[i])));
        Instance::PostMessage(&instance_, result.serialize().c_str());
        return;
      }
    }
  }
}

void SEService::ErrorHandler(int error) {
  LoggerD("Entered");
  is_error_ = true;

  for (const auto& callback_id : get_readers_callbacks_) {
    GetReaders(callback_id);
  }
  get_readers_callbacks_.clear();
}

} // secureelement
} // extension
