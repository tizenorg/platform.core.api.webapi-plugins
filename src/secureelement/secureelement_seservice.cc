// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "secureelement_seservice.h"

#include "secureelement_instance.h"
#include "common/logger.h"
#include "common/task-queue.h"
#include "common/platform_result.h"

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
  }
};

static SEServiceEventHandler se_event_handler;

SEService& SEService::GetInstance() {
  static SEService instance;
  return instance;
}

SEService::SEService() : is_initialized_(false), is_listener_set_(false) {
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

void SEService::GetReaders(const picojson::value& args) {
  LoggerD("Entered");

  double callback_id = 0.0;
  if (args.contains("callbackId")) {
    callback_id = args.get("callbackId").get<double>();
  }

  auto get_readers = [this](
      const std::shared_ptr<picojson::value>& response) -> void {

    if (!is_initialized_) {
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "SE service not initialized."),
                  &response->get<picojson::object>());
      return;
    }

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

  auto get_readers_response = [callback_id](
      const std::shared_ptr<picojson::value>& response) -> void {
    picojson::object& obj = response->get<picojson::object>();
    obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
    SecureElementInstance::getInstance().PostMessage(response->serialize().c_str());
  };

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
  if (!is_listener_set_) {
    LoggerW("SE listener is not set.");
    return;
  }

  std::vector<smartcard_service_api::ReaderHelper *> readers = se_service_->getReaders();
  for (std::size_t i = 0; i < readers.size(); i++) {
    if (readers[i]->isSecureElementPresent()) {
      picojson::value result = picojson::value(picojson::object());
      picojson::object& obj = result.get<picojson::object>();

      obj.insert(std::make_pair("action", picojson::value("onSEReady")));
      obj.insert(std::make_pair("handle", picojson::value((double) (long) readers[i])));

      SecureElementInstance::getInstance().PostMessage(result.serialize().c_str());
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
        SecureElementInstance::getInstance().PostMessage(result.serialize().c_str());
        return;
      }
    }
  }
}

} // secureelement
} // extension
