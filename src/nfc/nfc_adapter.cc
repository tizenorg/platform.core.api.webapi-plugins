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

#include "nfc/nfc_adapter.h"

#include <glib.h>

#include <memory>
#include <string>

#include "common/assert.h"
#include "common/converter.h"
#include "common/extension.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "nfc/aid_data.h"
#include "nfc/defs.h"
#include "nfc/nfc_message_utils.h"
#include "nfc/nfc_util.h"

using namespace common;
using namespace std;

namespace extension {
namespace nfc {

namespace {

const std::string CARD_ELEMENT = "CardElement";
const std::string TRANSACTION = "Transaction";
const std::string HCE_EVENT_DATA = "HCEEventData";

const std::string ACTIVE_SECURE_ELEMENT_CHANGED = "ActiveSecureElementChanged";
const std::string CARD_EMULATION_MODE_CHANGED = "CardEmulationModeChanged";
const std::string TRANSACTION_EVENT_LISTENER_ESE = "TransactionEventListener_ESE";
const std::string TRANSACTION_EVENT_LISTENER_UICC = "TransactionEventListener_UICC";
const std::string PEER_LISTENER = "PeerListener";
const std::string HCE_EVENT_LISTENER = "HCEEventListener";

void HCEEventCallback(nfc_se_h handle,
                  nfc_hce_event_type_e event_type,
                  unsigned char* apdu,
                  unsigned int apdu_len,
                  void* /*user_data*/) {
  LoggerD("Entered");
  NFCAdapter::GetInstance()->SetSEHandle(handle);

  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();
  response_obj[JSON_LISTENER_ID] = picojson::value(HCE_EVENT_LISTENER);
  response_obj[JSON_TYPE] = picojson::value(HCE_EVENT_DATA);

  // HCE Event Data
  picojson::value event_data = picojson::value(picojson::object());
  picojson::object& event_data_obj = event_data.get<picojson::object>();
  event_data_obj[JSON_EVENT_TYPE] = picojson::value(NFCUtil::ToStr(event_type));
  event_data_obj[JSON_APDU] = picojson::value(
      NFCUtil::FromUCharArray(apdu, apdu_len));
  event_data_obj[JSON_LENGTH] = picojson::value(static_cast<double>(apdu_len));
  tools::ReportSuccess(event_data, response_obj);

  NFCAdapter::GetInstance()->RespondAsync(response.serialize().c_str());
}

}  // anonymous namespace

NFCAdapter::NFCAdapter():
            m_last_tag_handle(nullptr),
            m_is_tag_listener_set(false),
            m_latest_tag_id(0),
            m_is_listener_set(false),
            m_is_transaction_ese_listener_set(false),
            m_is_transaction_uicc_listener_set(false),
            m_is_transaction_hce_listener_set(false),
            m_is_peer_listener_set(false),
            m_latest_peer_id(0),
            m_peer_handle(nullptr),
            m_is_ndef_listener_set(false),
            m_se_handle(nullptr),
            m_is_hce_listener_set(false),
            responder_(nullptr)
{
  LoggerD("Entered");
  // NFC library initialization
  int ret = nfc_manager_initialize();
  if(ret != NFC_ERROR_NONE) {
    LoggerE("Could not initialize NFC Manager, error: %d", ret);
  }
}

NFCAdapter::~NFCAdapter() {
  LoggerD("Entered");
  if (m_is_listener_set) {
    nfc_manager_unset_se_event_cb();
  }
  if (m_is_peer_listener_set) {
    nfc_manager_unset_p2p_target_discovered_cb();
  }
  if (m_is_transaction_ese_listener_set) {
    nfc_manager_unset_se_transaction_event_cb(NFC_SE_TYPE_ESE);
  }
  if (m_is_transaction_uicc_listener_set) {
    nfc_manager_unset_se_transaction_event_cb(NFC_SE_TYPE_UICC);
  }
  if (m_is_ndef_listener_set) {
    nfc_p2p_unset_data_received_cb(m_peer_handle);
  }
  if (m_is_tag_listener_set) {
    nfc_manager_unset_tag_discovered_cb();
  }
  if (m_is_hce_listener_set) {
    nfc_manager_unset_hce_event_cb();
  }

  // NFC library deinitialization
  int ret = nfc_manager_deinitialize();
  if(ret != NFC_ERROR_NONE) {
    LoggerE("Could not deinitialize NFC Manager, error: %d", ret);
  }
}

void NFCAdapter::SetResponder(IResponder* responder) {
  LoggerD("Entered");
  responder_ = responder;
}

void NFCAdapter::RespondAsync(const char* msg) {
  LoggerD("Entered");
  AssertMsg(GetInstance()->responder_, "Handler variable should be set");
  GetInstance()->responder_->RespondAsync(msg);
}

static picojson::value CreateEventError(double callbackId, const PlatformResult& ret) {
  LoggerD("Entered");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  tools::ReportError(ret, &obj);
  obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

  return event;
}

static picojson::value createEventSuccess(double callbackId) {
  LoggerD("Entered");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  tools::ReportSuccess(obj);
  obj[JSON_CALLBACK_ID] = picojson::value(callbackId);

  return event;
}

static gboolean setPoweredCompleteCB(void* user_data) {
  LoggerD("Entered");
  double* callbackId = static_cast<double*>(user_data);
  picojson::value event = createEventSuccess(*callbackId);
  NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());

  delete callbackId;
  callbackId = NULL;
  return false;
}

static void targetDetectedCallback(nfc_discovered_type_e type,
                                   nfc_p2p_target_h target,
                                   void* /*user_data*/) {
  LoggerD("Entered");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj[JSON_LISTENER_ID] = picojson::value(PEER_LISTENER);

  NFCAdapter* adapter = NFCAdapter::GetInstance();

  //unregister previous NDEF listener
  if (adapter->IsNDEFListenerSet()) {
    adapter->UnsetReceiveNDEFListener(adapter->GetPeerId());
  }

  if (NFC_DISCOVERED_TYPE_ATTACHED == type) {
    adapter->SetPeerHandle(target);
    obj["action"] = picojson::value("onattach");
    adapter->IncreasePeerId();
    obj["id"] = picojson::value(static_cast<double>(adapter->GetPeerId()));
  } else {
    adapter->SetPeerHandle(NULL);
    obj["action"] = picojson::value("ondetach");
  }
  NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
}

NFCAdapter* NFCAdapter::GetInstance() {
  LoggerD("Entered");
  static NFCAdapter instance;
  return &instance;
}

bool NFCAdapter::GetPowered() {
  LoggerD("Entered");
  return nfc_manager_is_activated();
}

#ifndef APP_CONTROL_SETTINGS_SUPPORT

static void NFCSetActivationCompletedCallback(nfc_error_e error,
                                              void* user_data) {
  LoggerD("Entered");
  double* callbackId = static_cast<double*>(user_data);

  if (NFC_ERROR_NONE != error) {
    PlatformResult result = NFCUtil::CodeToResult(error, NFCUtil::getNFCErrorMessage(error).c_str());
    picojson::value event = CreateEventError(*callbackId, result);

    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  } else {
    picojson::value event = createEventSuccess(*callbackId);

    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  }
  delete callbackId;
  callbackId = NULL;
}

#endif

static void se_event_callback(nfc_se_event_e se_event, void* /*user_data*/) {
  LoggerD("Entered");
  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  tools::ReportSuccess(obj);
  LoggerD("se_event_occured: %d", se_event);

  string result = "";
  switch (se_event) {
    case NFC_SE_EVENT_SE_TYPE_CHANGED:
      NFCAdapter::GetInstance()->GetActiveSecureElement(&result);
      obj.insert(make_pair(JSON_LISTENER_ID,
          picojson::value(ACTIVE_SECURE_ELEMENT_CHANGED)));
      break;
    case NFC_SE_EVENT_CARD_EMULATION_CHANGED:
      NFCAdapter::GetInstance()->GetCardEmulationMode(&result);
      obj.insert(make_pair(JSON_LISTENER_ID,
          picojson::value(CARD_EMULATION_MODE_CHANGED)));
      break;
    default:
      LoggerE("Unsupported se_event: %d", se_event);
      return;
  }

  obj.insert(make_pair(JSON_TYPE, picojson::value(CARD_ELEMENT)));
  obj.insert(make_pair(JSON_MODE, picojson::value(result)));
  NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
}

static void transaction_event_callback(nfc_se_type_e type,
                                       unsigned char* _aid,
                                       int aid_size,
                                       unsigned char* param,
                                       int param_size,
                                       void* /*user_data*/) {
  LoggerD("Entered");
  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();
  tools::ReportSuccess(response_obj);
  picojson::array& aid_array = response_obj.insert(std::make_pair(JSON_AID,
                                                                  picojson::value(picojson::array()))).first->second.get<picojson::array>();
  picojson::array& data_array = response_obj.insert(std::make_pair(JSON_DATA,
                                                                   picojson::value(picojson::array()))).first->second.get<picojson::array>();

  for (unsigned int i = 0; i < aid_size; i++) {
    aid_array.push_back(picojson::value(static_cast<double>(_aid[i])));
  }

  for (unsigned int i = 0; i < param_size; i++) {
    aid_array.push_back(picojson::value(static_cast<double>(param[i])));
  }

  if (NFC_SE_TYPE_ESE == type) {
    response_obj.insert(make_pair(JSON_LISTENER_ID,
        picojson::value(TRANSACTION_EVENT_LISTENER_ESE)));
  } else {
    response_obj.insert(make_pair(JSON_LISTENER_ID,
        picojson::value(TRANSACTION_EVENT_LISTENER_UICC)));
  }

  response_obj.insert(make_pair(JSON_TYPE, picojson::value(TRANSACTION)));
  NFCAdapter::GetInstance()->RespondAsync(response.serialize().c_str());
}

#ifdef APP_CONTROL_SETTINGS_SUPPORT
static void PostMessage(double* callbackId) {
  picojson::value event = CreateEventError(*callbackId,
                                           PlatformResult(ErrorCode::UNKNOWN_ERR,
                                                          "SetPowered failed."));
  NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  delete callbackId;
  callbackId = NULL;
}
#endif

PlatformResult NFCAdapter::SetPowered(const picojson::value& args) {
  LoggerD("Entered");
  double* callbackId = new double(args.get(JSON_CALLBACK_ID).get<double>());

  bool powered = args.get("powered").get<bool>();

  if (nfc_manager_is_activated() == powered) {
    if (!g_idle_add(setPoweredCompleteCB, static_cast<void*>(callbackId))) {
      LoggerE("g_idle addition failed");
      delete callbackId;
      callbackId = NULL;
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "SetPowered failed.");
    }
    return PlatformResult(ErrorCode::NO_ERROR);
  }

#ifdef APP_CONTROL_SETTINGS_SUPPORT
  app_control_h service = NULL;

  int ret = app_control_create(&service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("app_control_create failed: %d", ret);
    delete callbackId;
    callbackId = NULL;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "SetPowered failed.");
  }

  ret = app_control_set_operation(service, "http://tizen.org/appcontrol/operation/setting/nfc");
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("app_control_set_operation failed: %d", ret);
    app_control_destroy(service);
    delete callbackId;
    callbackId = NULL;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "SetPowered failed.");
  }

  ret = app_control_add_extra_data(service, "type", "nfc");
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("app_control_add_extra_data failed: %d", ret);
    app_control_destroy(service);
    delete callbackId;
    callbackId = NULL;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "SetPowered failed.");
  }

  ret = app_control_send_launch_request(service, [](app_control_h request,
      app_control_h reply, app_control_result_e result, void* user_data) {
    double* callbackId = static_cast<double*>(user_data);
    if (result == APP_CONTROL_RESULT_SUCCEEDED) {
      char* type = NULL;
      int ret = app_control_get_extra_data(reply, "nfc_status",
                                           &type);
      if (ret != APP_CONTROL_ERROR_NONE) {
        LoggerE("app_control_get_extra_data failed: %d", ret);
        PostMessage(callbackId);
        return;
      }
      LoggerD("app_control result: %s", type);
    } else {
      LoggerE("NFC enable app control failed : %d", result);
      PostMessage(callbackId);
      return;
    }

    if (!g_idle_add(setPoweredCompleteCB, static_cast<void*>(callbackId))) {
      LoggerE("g_idle addition failed");
      PostMessage(callbackId);
      return;
    }
  }, static_cast<void*>(callbackId));

  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("app_control_send_launch_request failed: %d", ret);
    app_control_destroy(service);
    delete callbackId;
    callbackId = NULL;
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "SetPowered failed.");
  }

  ret = app_control_destroy(service);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LoggerE("app_control_destroy failed: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "SetPowered failed.");
  }
#else
  int ret = nfc_manager_set_activation(powered,
      NFCSetActivationCompletedCallback, static_cast<void*>(callbackId));

  if (NFC_ERROR_NONE != ret) {
    LoggerE("setPowered failed %d",ret);
    delete callbackId;
    callbackId = NULL;
    return NFCUtil::CodeToResult(ret, "setPowered failed.");
  }
#endif
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::GetCardEmulationMode(std::string* mode) {
  LoggerD("Entered");

  nfc_se_card_emulation_mode_type_e card_mode;
  int ret = nfc_se_get_card_emulation_mode(&card_mode);

  if (NFC_ERROR_NONE != ret) {
    LoggerE("Failed to get card emulation mode %d", ret);
    return NFCUtil::CodeToResult(ret, "Failed to get card emulation mode");
  }

  return NFCUtil::ToStringCardEmulationMode(card_mode, mode);
}

PlatformResult NFCAdapter::SetCardEmulationMode(const std::string& mode) {
  LoggerD("Entered");

  nfc_se_card_emulation_mode_type_e new_mode;
  PlatformResult result = NFCUtil::ToCardEmulationMode(mode, &new_mode);
  if (result.IsError()) {
    return result;
  }
  LoggerD("Card emulation mode value: %x", (int)new_mode);

  std::string current_mode = "";
  result = GetCardEmulationMode(&current_mode);

  if (result.IsError()) {
    LoggerD("Error: %s", result.message().c_str());
    return result;
  }

  if (mode.compare(current_mode) == 0) {
    LoggerD("Card emulation mode already set to given value (%s)",
            mode.c_str());
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  int ret = NFC_ERROR_NONE;
  switch (new_mode) {
    case NFC_SE_CARD_EMULATION_MODE_OFF:
      ret = nfc_se_disable_card_emulation();
      break;
    case NFC_SE_CARD_EMULATION_MODE_ON:
      ret = nfc_se_enable_card_emulation();
      break;
    default:
      // Should never go here - in case of invalid mode
      // platformResult is returned from convertert few lines above
      LoggerE("Invalid card emulation mode: %s", mode.c_str());
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                            "Invalid card emulation mode given.");
  }

  if (NFC_ERROR_NONE != ret) {
    LoggerE("Failed to set card emulation mode %d", ret);
    return NFCUtil::CodeToResult(ret, "Failed to set card emulation mode");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::GetActiveSecureElement(std::string* type) {
  LoggerD("Entered");

  nfc_se_type_e se_type = NFC_SE_TYPE_DISABLE;
  int ret = nfc_manager_get_se_type(&se_type);
  if (NFC_ERROR_NONE != ret) {
    LoggerE("Failed to get active secure element type: %d", ret);
    return NFCUtil::CodeToResult(ret,
        "Unable to get active secure element type");
  }

  return NFCUtil::ToStringSecureElementType(se_type, type);
}

PlatformResult NFCAdapter::SetActiveSecureElement(std::string element) {
  LoggerD("Entered");

  // if given value is not correct secure element type then
  // there's no sense to get current value for comparison
  nfc_se_type_e new_type = NFC_SE_TYPE_DISABLE;
  PlatformResult result = NFCUtil::ToSecureElementType(element, &new_type);

  if (result.IsError()) {
    LoggerD("Error: %s", result.message().c_str());
    return result;
  }
  LoggerD("Secure element type value: %x", (int)new_type);

  std::string current_type = "";
  result = GetActiveSecureElement(&current_type);

  if (result.IsError()) {
    LoggerD("Error: %s", result.message().c_str());
    return result;
  }

  if (element == current_type) {
    LoggerD("Active secure element type already set to: %s", element.c_str());
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  int ret = nfc_manager_set_se_type(new_type);
  if (NFC_ERROR_NONE != ret) {
    LoggerE("Failed to set active secure element type: %d", ret);
    return NFCUtil::CodeToResult(ret,
        "Unable to set active secure element type");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::SetExclusiveModeForTransaction(bool exmode) {
  LoggerD("Entered");

  int ret = NFC_ERROR_NONE;
  if (exmode) {
    ret = nfc_manager_enable_transaction_fg_dispatch();
  } else {
    ret = nfc_manager_disable_transaction_fg_dispatch();
  }

  if (NFC_ERROR_NONE != ret) {
    LoggerE("Failed to set exclusive mode for transaction: %d", ret);
    return NFCUtil::CodeToResult(ret,
        "Setting exclusive mode for transaction failed.");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::AddCardEmulationModeChangeListener() {
  LoggerD("Entered");
  if (!m_is_listener_set) {
    int ret = nfc_manager_set_se_event_cb(se_event_callback, nullptr);
    if (NFC_ERROR_NONE != ret) {
      LoggerE("AddCardEmulationModeChangeListener failed: %d", ret);
      return NFCUtil::CodeToResult(ret,
                                 NFCUtil::getNFCErrorMessage(ret).c_str());
    }
    m_is_listener_set = true;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::RemoveCardEmulationModeChangeListener() {
  LoggerD("Entered");
  if (!nfc_manager_is_supported()) {
    LoggerE("NFC Not Supported");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "NFC Not Supported");
  }

  if (m_is_listener_set) {
    nfc_manager_unset_se_event_cb();
    m_is_listener_set = false;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}


PlatformResult NFCAdapter::AddTransactionEventListener(
    const picojson::value& args) {
  LoggerD("Entered");

  nfc_se_type_e se_type = NFC_SE_TYPE_DISABLE;
  PlatformResult result = NFCUtil::ToSecureElementType(
      args.get(JSON_TYPE).get<string>(), &se_type);
  if (result.IsError()) {
    LoggerD("Error: %s", result.message().c_str());
    return result;
  }

  int ret = NFC_ERROR_NONE;
  if (NFC_SE_TYPE_ESE == se_type) {
    if (!m_is_transaction_ese_listener_set) {
      ret = nfc_manager_set_se_transaction_event_cb(se_type,
          transaction_event_callback, NULL);
      m_is_transaction_ese_listener_set = true;
    }
  } else if (NFC_SE_TYPE_UICC == se_type) {
    if (!m_is_transaction_uicc_listener_set) {
      ret = nfc_manager_set_se_transaction_event_cb(se_type,
          transaction_event_callback, NULL);
      m_is_transaction_uicc_listener_set = true;
    }
  } else if (NFC_SE_TYPE_HCE == se_type) {
    if (!m_is_transaction_hce_listener_set) {
      ret = nfc_manager_set_se_transaction_event_cb(se_type,
          transaction_event_callback, NULL);
      m_is_transaction_hce_listener_set = true;
    }
  }

  if (NFC_ERROR_NONE != ret) {
    LoggerE("AddTransactionEventListener failed: %d", ret);
    return NFCUtil::CodeToResult(ret, NFCUtil::getNFCErrorMessage(ret).c_str());
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::RemoveTransactionEventListener(
    const picojson::value& args) {
  LoggerD("Entered");

  nfc_se_type_e se_type = NFC_SE_TYPE_DISABLE;
  PlatformResult result =
      NFCUtil::ToSecureElementType(args.get(JSON_TYPE).get<string>(), &se_type);
  if (result.IsError()) {
    return result;
  }

  if (se_type == NFC_SE_TYPE_ESE) {
    nfc_manager_unset_se_transaction_event_cb(se_type);
    m_is_transaction_ese_listener_set = false;
  } else if (se_type == NFC_SE_TYPE_UICC) {
    nfc_manager_unset_se_transaction_event_cb(se_type);
    m_is_transaction_uicc_listener_set = false;
  } else if (se_type == NFC_SE_TYPE_HCE) {
    nfc_manager_unset_se_transaction_event_cb(se_type);
    m_is_transaction_hce_listener_set = false;
  } else {
    AssertMsg(false, "Invalid NFC SecureElement type");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::AddActiveSecureElementChangeListener() {
  LoggerD("Entered");
  if (!m_is_listener_set) {
    int ret = nfc_manager_set_se_event_cb(se_event_callback, nullptr);
    if (NFC_ERROR_NONE != ret) {
      LoggerE("AddActiveSecureElementChangeListener failed: %d", ret);
      return NFCUtil::CodeToResult(ret,
                                 NFCUtil::getNFCErrorMessage(ret).c_str());
    }
    m_is_listener_set = true;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::RemoveActiveSecureElementChangeListener() {
  LoggerD("Entered");
  if (!nfc_manager_is_supported()) {
    LoggerE("NFC Not Supported");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "NFC Not Supported");
  }

  if (m_is_listener_set) {
    nfc_manager_unset_se_event_cb();
    m_is_listener_set = false;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void NFCAdapter::SetPeerHandle(nfc_p2p_target_h handle) {
  LoggerD("Entered");
  m_peer_handle = handle;
}

nfc_p2p_target_h NFCAdapter::GetPeerHandle() {
  LoggerD("Entered");
  return m_peer_handle;
}

int NFCAdapter::GetPeerId() {
  LoggerD("Entered");
  return m_latest_peer_id;
}

void NFCAdapter::IncreasePeerId() {
  LoggerD("Entered");
  m_latest_peer_id++;
}

PlatformResult NFCAdapter::PeerIsConnectedGetter(int peer_id, bool* state) {
  LoggerD("Entered");
  if (m_latest_peer_id != peer_id || !m_peer_handle) {
    *state = false;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  nfc_p2p_target_h handle = NULL;
  int ret = nfc_manager_get_connected_target(&handle);
  if (NFC_ERROR_NONE != ret) {
    LoggerE("Failed to get connected target handle: %d", ret);
    return NFCUtil::CodeToResult(ret, "Failed to get connected target.");
  }

  *state = (m_peer_handle == handle);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::SetPeerListener() {
  LoggerD("Entered");
  if (!nfc_manager_is_supported()) {
    LoggerE("NFC Not Supported");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "NFC Not Supported");
  }

  if (!m_is_peer_listener_set) {
    int ret = nfc_manager_set_p2p_target_discovered_cb (targetDetectedCallback, NULL);
    if (NFC_ERROR_NONE != ret) {
      LoggerE("Failed to set listener: %d", ret);
      return NFCUtil::CodeToResult(ret, "setPeerListener failed");
    }
    m_is_peer_listener_set = true;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::UnsetPeerListener() {
  LoggerD("Entered");
  if (!nfc_manager_is_supported()) {
    LoggerE("NFC Not Supported");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "NFC Not Supported");
  }

  if (m_is_peer_listener_set) {
    nfc_manager_unset_p2p_target_discovered_cb();
    m_is_peer_listener_set = false;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

static void targetReceivedCallback(nfc_p2p_target_h /*target*/,
                                   nfc_ndef_message_h message,
                                   void* /*data*/) {
  LoggerD("Entered");
  unsigned char* raw_data = NULL;
  unsigned int size;
  if (NFC_ERROR_NONE != nfc_ndef_message_get_rawdata(message, &raw_data, &size)) {
    LoggerE("Unknown error while getting raw data of message.");
    free(raw_data);
    return;
  }

  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj.insert(make_pair("listenerId", picojson::value("ReceiveNDEFListener")));
  obj.insert(make_pair("id", picojson::value(static_cast<double>(NFCAdapter::GetInstance()->GetPeerId()))));

  NFCMessageUtils::ReportNdefMessageFromData(raw_data, size, obj);

  NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  free(raw_data);
}

PlatformResult NFCAdapter::SetReceiveNDEFListener(int peer_id) {
  LoggerD("Entered");
  //unregister previous NDEF listener
  if (m_is_ndef_listener_set) {
    int ret = nfc_p2p_unset_data_received_cb(m_peer_handle);
    if (NFC_ERROR_NONE != ret) {
      LoggerW("Unregister ReceiveNDEFListener error: %d", ret);
    }
    m_is_ndef_listener_set = false;
  }

  //check if peer object is still connected
  bool is_connected = false;
  PlatformResult result = PeerIsConnectedGetter(peer_id, &is_connected);
  if (result.IsError()) {
    LoggerD("Error: %s", result.message().c_str());
    return result;
  }

  if (!is_connected) {
    LoggerE("Target is not connected");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Target is not connected.");
  }

  int ret = nfc_p2p_set_data_received_cb(m_peer_handle, targetReceivedCallback, NULL);
  if (NFC_ERROR_NONE != ret) {
    LoggerE("Failed to set NDEF listener: %d", ret);
    return NFCUtil::CodeToResult(ret, "Failed to set NDEF listener");
  }

  m_is_ndef_listener_set = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::UnsetReceiveNDEFListener(int peer_id) {
  LoggerD("Entered");
  if (m_is_ndef_listener_set) {
    //check if peer object is still connected

    bool is_connected = false;
    PlatformResult result = PeerIsConnectedGetter(peer_id, &is_connected);
    if (result.IsError()) {
      LoggerD("Error: %s", result.message().c_str());
      return result;
    }

    if (!is_connected) {
      LoggerE("Target is not connected");
    }

    int ret = nfc_p2p_unset_data_received_cb(m_peer_handle);
    if (NFC_ERROR_NONE != ret) {
      LoggerE("Unregister ReceiveNDEFListener error: %d", ret);
      return NFCUtil::CodeToResult(ret, "Failed to set NDEF listener");
    }

    m_is_ndef_listener_set = false;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

bool NFCAdapter::IsNDEFListenerSet() {
  LoggerD("Entered");
  return m_is_ndef_listener_set;
}


// NFCTag related functions
PlatformResult NFCAdapter::TagTypeGetter(int /*tag_id*/, std::string* type) {
  LoggerD("Entered");

  nfc_tag_type_e nfc_type = NFC_UNKNOWN_TARGET;

  int err = nfc_tag_get_type(m_last_tag_handle, &nfc_type);
  if(NFC_ERROR_NONE != err) {
    LoggerE("Failed to get tag type: %d", err);
    return NFCUtil::CodeToResult(err, "Failed to get tag type");
  }

  *type = NFCUtil::ToStringNFCTag(nfc_type);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::TagIsSupportedNDEFGetter(int /*tag_id*/,
                                                    bool* is_supported) {
  LoggerD("Entered");

  int err = nfc_tag_is_support_ndef(m_last_tag_handle, is_supported);
  if(NFC_ERROR_NONE != err) {
    LoggerE("Failed to check if NDEF is supported %d", err);
    return NFCUtil::CodeToResult(err,
                               "Failed to check if NDEF is supported");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::TagNDEFSizeGetter(int /*tag_id*/,
                                             unsigned int* size) {
  LoggerD("Entered");

  int err = nfc_tag_get_ndef_size(m_last_tag_handle, size);
  if(NFC_ERROR_NONE != err) {
    LoggerE("Failed to get tag NDEF size: %d, %s", err);
    return NFCUtil::CodeToResult(err,
                               "Failed to get tag NDEF size");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

static bool tagPropertiesGetterCb(const char* key,
                                  const unsigned char* value,
                                  int value_size,
                                  void* user_data) {
  LoggerD("Entered");
  if (user_data) {
    UCharVector tag_info = NFCUtil::ToVector(value, value_size);
    (static_cast<NFCTagPropertiesT*>(user_data))->push_back(
        std::make_pair(key, tag_info));
    return true;
  }
  return false;
}

PlatformResult NFCAdapter::TagPropertiesGetter(int /*tag_id*/,
                                               NFCTagPropertiesT* properties) {
  LoggerD("Entered");

  int err = nfc_tag_foreach_information(m_last_tag_handle,
                                        tagPropertiesGetterCb, (void*)properties);
  if(NFC_ERROR_NONE != err) {
    LoggerE("Error occured while getting NFC properties: %d", err);
    return NFCUtil::CodeToResult(err,
                               "Error occured while getting NFC properties");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::TagIsConnectedGetter(int tag_id, bool* state) {
  LoggerD("Entered");

  PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

  if(tag_id != m_latest_tag_id || NULL == m_last_tag_handle) {
    // internaly stored tag id changed -> new tag has been already connected
    // internaly stored tag handle NULL -> tag has been disconnected
    LoggerD("NFCTag () not connected (id differs or invalid handle)");
    *state = false;
    return result;
  }

  nfc_tag_h handle = NULL;
  int ret = nfc_manager_get_connected_tag(&handle);
  if(NFC_ERROR_NONE != ret) {
    LoggerE("Failed to get connected tag: %s",
            NFCUtil::getNFCErrorMessage(ret).c_str());
    // exception is thrown here to return undefined in JS layer
    // instead of false
    return NFCUtil::CodeToResult(ret, "Failed to get connected tag");
  }

  if(m_last_tag_handle != handle) {
    LoggerD("Last known handle and current handle differs");
    *state = false;
  } else {
    *state = true;
  }

  return result;
}

int NFCAdapter::GetNextTagId() {
  LoggerD("Entered");
  return ++m_latest_tag_id;
}


static void tagEventCallback(nfc_discovered_type_e type,
                             nfc_tag_h tag,
                             void* /*data*/) {
  LoggerD("Entered");

  picojson::value event = picojson::value(picojson::object());
  picojson::object& obj = event.get<picojson::object>();
  obj.insert(make_pair("listenerId", picojson::value("TagListener")));

  NFCAdapter* adapter = NFCAdapter::GetInstance();
  // Tag detected event
  if (NFC_DISCOVERED_TYPE_ATTACHED == type) {
    nfc_tag_type_e tag_type;

    int result;
    result = nfc_tag_get_type(tag, &tag_type);
    if(NFC_ERROR_NONE != result) {
      LoggerE("setTagListener failed %d",result);
      return;
    }
    // Fetch new id and set detected tag handle in NFCAdapter
    int generated_id = adapter->GetNextTagId();
    adapter->SetTagHandle(tag);

    obj.insert(make_pair("action", picojson::value("onattach")));
    obj.insert(make_pair("id", picojson::value(static_cast<double>(generated_id))));
    obj.insert(make_pair("type", picojson::value(NFCUtil::ToStringNFCTag(tag_type))));

    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  }
  // Tag disconnected event
  else if (NFC_DISCOVERED_TYPE_DETACHED == type) {
    // Set stored tag handle to NULL
    adapter->SetTagHandle(NULL);

    obj.insert(make_pair("action", picojson::value("ondetach")));
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  }
  // ERROR - should never happen
  else {
    LoggerE("Invalid NFC discovered type: %d (%x)", type, type);
  }

}

PlatformResult  NFCAdapter::SetTagListener() {
  LoggerD("Entered");

  if (!m_is_tag_listener_set) {
    nfc_manager_set_tag_filter(NFC_TAG_FILTER_ALL_ENABLE);
    int result = nfc_manager_set_tag_discovered_cb (tagEventCallback, NULL);
    if (NFC_ERROR_NONE != result) {
      LoggerE("Failed to register tag listener: %d", result);
      return NFCUtil::CodeToResult(result, "Failed to register tag listener.");
    }
    m_is_tag_listener_set = true;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void NFCAdapter::UnsetTagListener() {
  LoggerD("Entered");

  if(m_is_tag_listener_set) {
    nfc_manager_unset_tag_discovered_cb();
    m_is_tag_listener_set = false;
  }
}

void NFCAdapter::SetTagHandle(nfc_tag_h tag) {
  LoggerD("Entered");
  m_last_tag_handle = tag;
}

static void tagReadNDEFCb(nfc_error_e result,
                          nfc_ndef_message_h message,
                          void* data) {
  LoggerD("Entered");

  if(!data) {
    // Can not continue if unable to get callbackId
    LoggerE("NULL callback id given");
    return;
  }

  double callbackId = *((double*)data);
  delete (double*)data;
  data = NULL;

  if(NFC_ERROR_NONE != result) {
    // create exception and post error message (call error callback)
    picojson::value event = CreateEventError(callbackId, PlatformResult(ErrorCode::UNKNOWN_ERR,
                                                                        "Failed to read NDEF message"));
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    return;
  }

  unsigned char* raw_data = NULL;
  unsigned int size = 0;

  int ret = nfc_ndef_message_get_rawdata(message, &raw_data, &size);
  if(NFC_ERROR_NONE != ret) {
    LoggerE("Failed to get message data: %d", ret);

    // release data if any allocated
    free(raw_data);

    // create exception and post error message (call error callback)
    picojson::value event = CreateEventError(callbackId, PlatformResult(ErrorCode::UNKNOWN_ERR,
                                                                        "Failed to retrieve NDEF message data"));
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    return;
  }

  // create success event, fill it with data and call success callback
  picojson::value event = createEventSuccess(callbackId);
  picojson::object& obj = event.get<picojson::object>();

  NFCMessageUtils::ReportNdefMessageFromData(raw_data, size, obj);

  NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  free(raw_data);
}

PlatformResult NFCAdapter::TagReadNDEF(int tag_id,
                                       const picojson::value& args) {
  LoggerD("Entered");

  bool is_connected = false;
  PlatformResult result = TagIsConnectedGetter(tag_id, &is_connected);
  if (result.IsError()) {
    return result;
  }

  double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
  LoggerD("Received callback id: %f", callbackId);

  if(!is_connected) {
    picojson::value event = CreateEventError(callbackId,
                                             PlatformResult(ErrorCode::UNKNOWN_ERR,
                                                            "Tag is no more connected."));
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    return PlatformResult(ErrorCode::NO_ERROR);;
  }

  double* callbackIdPointer = new double(callbackId);
  int ret = nfc_tag_read_ndef(m_last_tag_handle, tagReadNDEFCb,
                                 (void*)(callbackIdPointer));
  if(NFC_ERROR_NONE != ret) {
    LoggerE("Failed to read NDEF message from tag: %d", ret);
    delete callbackIdPointer;
    callbackIdPointer = NULL;

    // for permission related error throw exception ...
    if(NFC_ERROR_SECURITY_RESTRICTED == ret ||
        NFC_ERROR_PERMISSION_DENIED == ret) {
      LoggerD("Error: %d", ret);
      return PlatformResult(ErrorCode::SECURITY_ERR, "Failed to read NDEF - permission denied");
    }

    LoggerE("Preparing error callback to call");
    // ... otherwise call error callback
    std::string errMessage = NFCUtil::getNFCErrorMessage(ret);

    result = NFCUtil::CodeToResult(ret, errMessage.c_str());

    picojson::value event = CreateEventError(callbackId, result);
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  }
  delete callbackIdPointer;
  callbackIdPointer = NULL;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::TagWriteNDEF(int tag_id,
                                        const picojson::value& args) {
  LoggerD("Entered");

  bool is_connected = false;
  PlatformResult result = TagIsConnectedGetter(tag_id, &is_connected);
  if (result.IsError()) {
    LoggerD("Error: %s", result.message().c_str());
    return result;
  }

  double callbackId = args.get(JSON_CALLBACK_ID).get<double>();
  LoggerD("Received callback id: %f", callbackId);

  if(!is_connected) {
    picojson::value event = CreateEventError(callbackId,
                                             PlatformResult(ErrorCode::UNKNOWN_ERR,
                                                            "Tag is no more connected."));
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    return PlatformResult(ErrorCode::NO_ERROR);;
  }

  const picojson::array& records_array = FromJson<picojson::array>(
      args.get<picojson::object>(), "records");

  const int size = static_cast<int>(args.get("recordsSize").get<double>());

  nfc_ndef_message_h message = NULL;
  result = NFCMessageUtils::NDEFMessageToStruct(records_array, size, &message);

  if (result.IsError()) {
    LoggerD("Error: %s", result.message().c_str());
    return result;
  }

  if(message){
    int ret = nfc_tag_write_ndef(m_last_tag_handle, message, NULL, NULL);

    NFCMessageUtils::RemoveMessageHandle(message);
    if (NFC_ERROR_NONE != ret){

      // for permission related error throw exception
      if(NFC_ERROR_SECURITY_RESTRICTED == ret ||
          NFC_ERROR_PERMISSION_DENIED == ret) {
        LoggerE("Error: %d", ret);
        return PlatformResult(ErrorCode::SECURITY_ERR, "Failed to read NDEF - permission denied");
      }

      LoggerE("Error occured when sending message: %d", ret);
      std::string errMessage = NFCUtil::getNFCErrorMessage(ret);

      result = NFCUtil::CodeToResult(ret, errMessage.c_str());
      picojson::value event = CreateEventError(callbackId, result);
      // create and post error message
      NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    }
    else {
      // create and post success message
      picojson::value event = createEventSuccess(callbackId);
      NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    }
  } else {
    LoggerE("Invalid message handle");
    picojson::value event = CreateEventError(callbackId, PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                                                                        "Message is not valid"));
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

static void tagTransceiveCb(nfc_error_e err,
                            unsigned char* buffer,
                            int buffer_size,
                            void* data) {
  LoggerD("Entered");
  std::unique_ptr<unsigned char> buffer_ptr(buffer);
  buffer = nullptr;

  if (!data) {
    // no callback id - unable to report success, neither error
    LoggerE("Unable to launch transceive callback");
    return;
  }
  double callback_id = *(reinterpret_cast<double*>(data));
  delete reinterpret_cast<double*>(data);
  data = nullptr;

  if (NFC_ERROR_NONE != err) {

    LoggerE("NFCTag transceive failed: %d", err);

    // create exception and post error message (call error callback)
    std::string error_message = NFCUtil::getNFCErrorMessage(err);

    PlatformResult result = NFCUtil::CodeToResult(err, error_message.c_str());
    picojson::value event = CreateEventError(callback_id, result);
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    return;
  }

  // create result and push to JS layer
  picojson::value response = createEventSuccess(callback_id);
  picojson::object& response_obj = response.get<picojson::object>();
  tools::ReportSuccess(response_obj);
  response_obj[JSON_DATA] =
      picojson::value(NFCUtil::FromUCharArray(buffer_ptr.get(), buffer_size));

  NFCAdapter::GetInstance()->RespondAsync(response.serialize().c_str());
}

PlatformResult NFCAdapter::TagTransceive(int tag_id, const picojson::value& args) {

  LoggerD("Entered");
  bool is_connected = false;
  PlatformResult result = TagIsConnectedGetter(tag_id, &is_connected);
  if (result.IsError()) {
    return result;
  }

  double callback_id = args.get(JSON_CALLBACK_ID).get<double>();
  LoggerD("Received callback id: %f", callback_id);

  if(!is_connected) {
    picojson::value event = CreateEventError(callback_id,
        PlatformResult(ErrorCode::UNKNOWN_ERR, "Tag is no more connected."));
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    return PlatformResult(ErrorCode::NO_ERROR);;
 }

  const picojson::array& data_array = FromJson<picojson::array>(
      args.get<picojson::object>(), JSON_DATA);

  unsigned char* buffer = NFCUtil::DoubleArrayToUCharArray(data_array);
  double* callback_id_pointer = new double(callback_id);

  int ret = nfc_tag_transceive(m_last_tag_handle, buffer,
      data_array.size(), tagTransceiveCb, (void*) callback_id_pointer);

  if (NFC_ERROR_NONE != ret) {
    delete callback_id_pointer;
    callback_id_pointer = nullptr;
    delete[] buffer;
    buffer = nullptr;

    // for permission related error throw exception
    if(NFC_ERROR_SECURITY_RESTRICTED == ret ||
        NFC_ERROR_PERMISSION_DENIED == ret) {
      LoggerE("Error: %d", ret);
      return PlatformResult(ErrorCode::SECURITY_ERR,
          "Failed to read NDEF - permission denied");
    }

    std::string error_message = NFCUtil::getNFCErrorMessage(ret);

    LoggerE("NFCTag transceive failed: %d", ret);

    result = NFCUtil::CodeToResult(ret, error_message.c_str());
    picojson::value event = CreateEventError(callback_id, result);
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  }
  delete callback_id_pointer;
  callback_id_pointer = nullptr;
  delete[] buffer;
  buffer = nullptr;

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::GetCachedMessage(picojson::object& out) {
  LoggerD("Entered");
  nfc_ndef_message_h message_handle = NULL;
  int result = nfc_manager_get_cached_message(&message_handle);
  if (NFC_ERROR_INVALID_NDEF_MESSAGE == result ||
      NFC_ERROR_NO_NDEF_MESSAGE == result) {
    LoggerE("Error: %d", result);
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  if (NFC_ERROR_NONE != result) {
    LoggerE("Failed to get cached message: %d", result);
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Failed to get cached message");
  }
  unsigned char* raw_data = NULL;
  unsigned int size;
  if (NFC_ERROR_NONE != nfc_ndef_message_get_rawdata(message_handle,
                                                     &raw_data, &size)) {
    LoggerE("Unknown error while getting message.");
    free(raw_data);
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  PlatformResult ret = NFCMessageUtils::ReportNdefMessageFromData(raw_data,
      size, out);
  free(raw_data);
  if (ret.IsError()) {
    LoggerE("Error: %d", ret.message().c_str());
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return ret;
  }
  NFCMessageUtils::RemoveMessageHandle(message_handle);
  return PlatformResult(ErrorCode::NO_ERROR);
}

static void peerSentCallback(nfc_error_e result, void* user_data) {
  LoggerD("Entered");
  double* callbackId = static_cast<double*>(user_data);

  if (NFC_ERROR_NONE != result){
    LoggerE("Error: %d", result);
    std::string error_message = NFCUtil::getNFCErrorMessage(result);

    PlatformResult ret = NFCUtil::CodeToResult(result, error_message.c_str());
    picojson::value event = CreateEventError(*callbackId, ret);
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  } else {
    picojson::value event = createEventSuccess(*callbackId);
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
  }

  delete callbackId;
  callbackId = NULL;
}

static gboolean sendNDEFErrorCB(void* user_data) {
  LoggerD("Entered");
  peerSentCallback(NFC_ERROR_INVALID_NDEF_MESSAGE, user_data);
  return false;
}

PlatformResult NFCAdapter::sendNDEF(int peer_id, const picojson::value& args) {
  LoggerD("Entered");
  bool is_connected = false;
  PlatformResult result = PeerIsConnectedGetter(peer_id, &is_connected);
  if (result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    return result;
  }

  double* callbackId = new double(args.get(JSON_CALLBACK_ID).get<double>());

  if (!is_connected) {
    picojson::value event = CreateEventError(*callbackId,
        PlatformResult(ErrorCode::UNKNOWN_ERR, "Peer is no more connected"));
    NFCAdapter::GetInstance()->RespondAsync(event.serialize().c_str());
    delete callbackId;
    callbackId = NULL;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  const picojson::array& records_array =
      FromJson<picojson::array>(args.get<picojson::object>(), "records");
  const int size = static_cast<int>(args.get("recordsSize").get<double>());

  nfc_ndef_message_h message = NULL;
  result = NFCMessageUtils::NDEFMessageToStruct(
      records_array, size, &message);
  if (message) {
    int ret = nfc_p2p_send(m_peer_handle, message, peerSentCallback,
        static_cast<void*>(callbackId));
    NFCMessageUtils::RemoveMessageHandle(message);
    if (NFC_ERROR_NONE != ret) {
      LoggerE("sendNDEF failed %d",ret);
      delete callbackId;
      callbackId = NULL;
      return NFCUtil::CodeToResult(ret, "sendNDEF failed.");
    }
  } else {
    if (!g_idle_add(sendNDEFErrorCB, static_cast<void*>(callbackId))) {
      LoggerE("g_idle addition failed");
      delete callbackId;
      callbackId = NULL;
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void NFCAdapter::SetSEHandle(nfc_se_h handle) {
  LoggerD("Entered");
  m_se_handle = handle;
}

nfc_se_h NFCAdapter::GetSEHandle() {
  LoggerD("Entered");
  return m_se_handle;
}

PlatformResult NFCAdapter::AddHCEEventListener() {
  LoggerD("Entered");
  if (!m_is_hce_listener_set) {
    int ret = nfc_manager_set_hce_event_cb(HCEEventCallback, nullptr);
    if (NFC_ERROR_NONE != ret) {
      LoggerE("AddHCEEventListener failed: %d", ret);
      return NFCUtil::CodeToResult(ret,
          NFCUtil::getNFCErrorMessage(ret).c_str());
    }
    m_is_hce_listener_set = true;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::RemoveHCEEventListener() {
  LoggerD("Entered");
  if (m_is_hce_listener_set) {
    nfc_manager_unset_hce_event_cb();
    m_is_hce_listener_set = false;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void NFCAdapter::SendHostAPDUResponse(
    const UCharVector& apdu,
    const std::function<void()>& success_cb,
    const std::function<void(const PlatformResult&)>& error_cb) {
  LoggerD("Entered");

  if (!nfc_manager_is_supported()) {
    LoggerE("NFC Not Supported");
    error_cb(PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "NFC Not Supported"));
    return;
  }

  int ret = nfc_hce_send_apdu_response(m_se_handle,
      const_cast<unsigned char*>(apdu.data()),
      static_cast<unsigned int>(apdu.size()));
  if (NFC_ERROR_NONE == ret) {
    success_cb();
  } else {
    LoggerE("Error: %d", ret);
    error_cb(
        NFCUtil::CodeToResult(ret, NFCUtil::getNFCErrorMessage(ret).c_str()));
  }
}

PlatformResult NFCAdapter::IsActivatedHandlerForAID(
    const std::string& type,
    const std::string& aid,
    bool* is_activated_handler) {
  AssertMsg(is_activated_handler, "Poiner can not be null!");
  LoggerD("Entered");

  nfc_se_type_e se_type;
  PlatformResult result = NFCUtil::ToSecureElementType(type, &se_type);
  if (!result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    return result;
  }

  int ret = nfc_se_is_activated_handler_for_aid(se_type,
                                                aid.c_str(),
                                                is_activated_handler);
  if (NFC_ERROR_NONE != ret) {
    LoggerE("IsActivatedHandlerForAID failed: %d", ret);
    return NFCUtil::CodeToResult(ret,
        NFCUtil::getNFCErrorMessage(ret).c_str());
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::IsActivatedHandlerForCategory(
    const std::string& type,
    nfc_card_emulation_category_type_e category,
    bool* is_activated_handler) {
  LoggerD("Entered");
  AssertMsg(is_activated_handler, "Poiner can not be null!");

  nfc_se_type_e se_type;
  PlatformResult result = NFCUtil::ToSecureElementType(type, &se_type);
  if (!result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    return result;
  }

  int ret = nfc_se_is_activated_handler_for_category(se_type,
                                                     category,
                                                     is_activated_handler);
  if (NFC_ERROR_NONE != ret) {
    LoggerE("IsActivatedHandlerForCategory failed: %d", ret);
    return NFCUtil::CodeToResult(ret,
        NFCUtil::getNFCErrorMessage(ret).c_str());
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::RegisterAID(
    const std::string& type,
    const std::string& aid,
    nfc_card_emulation_category_type_e category) {
  LoggerD("Entered");
  nfc_se_type_e se_type;
  PlatformResult result = NFCUtil::ToSecureElementType(type, &se_type);
  if (!result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    return result;
  }

  int ret = nfc_se_register_aid(se_type, category, aid.c_str());
  if (NFC_ERROR_NONE != ret) {
    LoggerE("RegisterAID failed: %d", ret);
    return NFCUtil::CodeToResult(ret,
        NFCUtil::getNFCErrorMessage(ret).c_str());
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCAdapter::UnregisterAID(
    const std::string& type,
    const std::string& aid,
    nfc_card_emulation_category_type_e category) {
  LoggerD("Entered");
  nfc_se_type_e se_type;
  PlatformResult result = NFCUtil::ToSecureElementType(type, &se_type);
  if (!result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    return result;
  }

  int ret = nfc_se_unregister_aid(se_type, category, aid.c_str());
  if (NFC_ERROR_NONE != ret) {
    LoggerE("UnregisterAID failed: %d", ret);
    return NFCUtil::CodeToResult(ret,
        NFCUtil::getNFCErrorMessage(ret).c_str());
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

static void SaveRow(nfc_se_type_e se_type,
                   const char* aid,
                   bool read_only,
                   void* user_data) {
  LoggerD("Entered");
  AssertMsg(aid, "Poiner can not be null!");
  AssertMsg(user_data, "Poiner can not be null!");
  AIDDataVector* aids = static_cast<AIDDataVector*>(user_data);
  aids->push_back(AIDData(se_type, aid, read_only));
};

void NFCAdapter::GetAIDsForCategory(
    const std::string& type,
    nfc_card_emulation_category_type_e category,
    const std::function<void(const AIDDataVector&)>& success_cb,
    const std::function<void(const PlatformResult&)>& error_cb) {
  LoggerD("Entered");
  nfc_se_type_e se_type;
  PlatformResult result = NFCUtil::ToSecureElementType(type, &se_type);
  if (!result.IsError()) {
    LoggerE("Error: %s", result.message().c_str());
    error_cb(result);
    return;
  }

  AIDDataVector aids{};
  int ret = nfc_se_foreach_registered_aids(se_type, category, SaveRow, &aids);
  if (NFC_ERROR_NONE != ret) {
    LoggerE("GetAIDsForCategory failed: %d", ret);
    error_cb(NFCUtil::CodeToResult(ret,
        NFCUtil::getNFCErrorMessage(ret).c_str()));
  }
  success_cb(aids);
}

}// nfc
}// extension
