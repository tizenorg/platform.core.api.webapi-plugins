// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc_instance.h"
#include "nfc_util.h"
#include "nfc_message_utils.h"

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

// platform header
#include <nfc.h>


namespace extension {
namespace nfc {

using namespace common;
using namespace extension::nfc;

NFCInstance& NFCInstance::getInstance() {
  static NFCInstance instance;
  return instance;
}

NFCInstance::NFCInstance() {
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&NFCInstance::x, this, _1, _2));
  REGISTER_SYNC("NFCManager_getDefaultAdapter", GetDefaultAdapter);
  REGISTER_SYNC("NFCManager_setExclusiveMode", SetExclusiveMode);
  REGISTER_SYNC("NFCAdapter_getPowered", GetPowered);
  REGISTER_SYNC("NFCAdapter_cardEmulationModeSetter", CardEmulationModeSetter);
  REGISTER_SYNC("NFCAdapter_cardEmulationModeGetter", CardEmulationModeGetter);
  REGISTER_SYNC("NFCAdapter_activeSecureElementSetter", ActiveSecureElementSetter);
  REGISTER_SYNC("NFCAdapter_activeSecureElementGetter", ActiveSecureElementGetter);
  REGISTER_SYNC("NFCAdapter_setPeerListener", SetPeerListener);
  REGISTER_SYNC("NFCAdapter_setTagListener", SetTagListener);
  REGISTER_SYNC("NFCAdapter_PeerIsConnectedGetter", PeerIsConnectedGetter);
  REGISTER_SYNC("NFCAdapter_unsetTagListener", UnsetTagListener);
  REGISTER_SYNC("NFCAdapter_unsetPeerListener", UnsetPeerListener);
  REGISTER_SYNC("NFCAdapter_addCardEmulationModeChangeListener",
                AddCardEmulationModeChangeListener);
  REGISTER_SYNC("NFCAdapter_removeCardEmulationModeChangeListener",
                RemoveCardEmulationModeChangeListener);
  REGISTER_SYNC("NFCAdapter_addTransactionEventListener",
                AddTransactionEventListener);
  REGISTER_SYNC("NFCAdapter_removeTransactionEventListener",
                RemoveTransactionEventListener);
  REGISTER_SYNC("NFCAdapter_addActiveSecureElementChangeListener",
                AddActiveSecureElementChangeListener);
  REGISTER_SYNC("NFCAdapter_removeActiveSecureElementChangeListener",
                RemoveActiveSecureElementChangeListener);
  REGISTER_SYNC("NFCAdapter_getCachedMessage", GetCachedMessage);
  REGISTER_SYNC("NFCAdapter_setExclusiveModeForTransaction",
                SetExclusiveModeForTransaction);
  REGISTER_SYNC("NFCPeer_setReceiveNDEFListener", SetReceiveNDEFListener);
  REGISTER_SYNC("NFCPeer_unsetReceiveNDEFListener", UnsetReceiveNDEFListener);
  REGISTER_SYNC("NDEFMessage_toByte", ToByte);
  //Message related methods
  REGISTER_SYNC("NDEFMessage_constructor", NDEFMessageContructor);
  REGISTER_SYNC("NDEFRecord_constructor", NDEFRecordContructor);
  REGISTER_SYNC("NDEFRecordText_constructor", NDEFRecordTextContructor);
  REGISTER_SYNC("NDEFRecordURI_constructor", NDEFRecordURIContructor);
  REGISTER_SYNC("NDEFRecordMedia_constructor", NDEFRecordMediaContructor);

  // NFCTag attributes getters
  REGISTER_SYNC("NFCTag_typeGetter", TagTypeGetter);
  REGISTER_SYNC("NFCTag_isSupportedNDEFGetter", TagIsSupportedNDEFGetter);
  REGISTER_SYNC("NFCTag_NDEFSizeGetter", TagNDEFSizeGetter);
  REGISTER_SYNC("NFCTag_propertiesGetter", TagPropertiesGetter);
  REGISTER_SYNC("NFCTag_isConnectedGetter", TagIsConnectedGetter);
#undef REGISTER_SYNC
#define REGISTER(c,x) \
    RegisterSyncHandler(c, std::bind(&NFCInstance::x, this, _1, _2));
  REGISTER("NFCAdapter_setPowered", SetPowered);
  REGISTER("NFCTag_readNDEF", ReadNDEF);
  REGISTER("NFCTag_writeNDEF", WriteNDEF);
  REGISTER("NFCTag_transceive", Transceive );
  REGISTER("NFCPeer_sendNDEF", SendNDEF);
#undef REGISTER
  // NFC library initialization
  int result = nfc_manager_initialize();
  if (NFC_ERROR_NONE != result) {
    LoggerE("Could not initialize NFC Manager.");
  }
}

#define CHECK_EXIST(args, name, out) \
  if (!args.contains(name)) {\
    ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

NFCInstance::~NFCInstance() {
  int result = nfc_manager_deinitialize();
  if (NFC_ERROR_NONE != result) {
    LoggerE("NFC Manager deinitialization failed.");
  }
}

void NFCInstance::GetDefaultAdapter(
    const picojson::value& args, picojson::object& out) {

  // Default NFC adapter is created at JS level
  // Here there's only check for NFC support
  LoggerD("Entered");
  if(!nfc_manager_is_supported()) {
    LoggerE("NFC manager is not supported");
    // According to API reference only Security and Unknown
    // exceptions are allowed here
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR,
                               "NFC manager not supported"), &out);
  }
  else {
    ReportSuccess(out);
  }
}

void NFCInstance::SetExclusiveMode(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "exclusiveMode", out);
  bool exmode = args.get("exclusiveMode").get<bool>();
  int ret = NFC_ERROR_NONE;

  ret = nfc_manager_set_system_handler_enable(!exmode);
  if (NFC_ERROR_NONE != ret) {
    PlatformResult result = NFCUtil::CodeToResult(ret,
                                                  "Failed to set exclusive mode.");
    ReportError(result, &out);
  } else {
    ReportSuccess(out);
  }
}

void NFCInstance::SetPowered(
    const picojson::value& args, picojson::object& out) {
  PlatformResult result = NFCAdapter::GetInstance()->SetPowered(args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::GetPowered(
    const picojson::value& args, picojson::object& out) {
  bool ret = NFCAdapter::GetInstance()->GetPowered();
  ReportSuccess(picojson::value(ret), out);
}

void NFCInstance::CardEmulationModeSetter(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "emulationMode", out);
  std::string mode = args.get("emulationMode").get<std::string>();
  PlatformResult result = NFCAdapter::GetInstance()->SetCardEmulationMode(mode);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::CardEmulationModeGetter(
    const picojson::value& args, picojson::object& out) {

  std::string mode = "";
  PlatformResult result = NFCAdapter::GetInstance()->GetCardEmulationMode(&mode);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(mode), out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::ActiveSecureElementSetter(
    const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "secureElement", out);
  std::string ase = args.get("secureElement").get<std::string>();
  PlatformResult result = NFCAdapter::GetInstance()->SetActiveSecureElement(ase);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::ActiveSecureElementGetter(
    const picojson::value& args, picojson::object& out) {

  std::string ase = "";
  PlatformResult result = NFCAdapter::GetInstance()->GetActiveSecureElement(&ase);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(ase), out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::SetTagListener(
    const picojson::value& args, picojson::object& out) {

  PlatformResult result = NFCAdapter::GetInstance()->SetTagListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::PeerIsConnectedGetter(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "id", out);

  int peer_id = (int)args.get("id").get<double>();
  bool ret = false;
  PlatformResult result = NFCAdapter::GetInstance()->PeerIsConnectedGetter(peer_id, &ret);

  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(ret), out);
  } else {
    ReportError(result, &out);
  }

}

void NFCInstance::SetPeerListener(
    const picojson::value& args, picojson::object& out) {

  PlatformResult result = NFCAdapter::GetInstance()->SetPeerListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::UnsetTagListener(
    const picojson::value& args, picojson::object& out) {

  NFCAdapter::GetInstance()->UnsetTagListener();
  ReportSuccess(out);
}

void NFCInstance::UnsetPeerListener(
    const picojson::value& args, picojson::object& out) {

  PlatformResult result = NFCAdapter::GetInstance()->UnsetPeerListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::AddCardEmulationModeChangeListener(
    const picojson::value& args, picojson::object& out) {

  PlatformResult result = NFCAdapter::GetInstance()->AddCardEmulationModeChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::RemoveCardEmulationModeChangeListener(
    const picojson::value& args, picojson::object& out) {
  PlatformResult result = NFCAdapter::GetInstance()->RemoveCardEmulationModeChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::AddTransactionEventListener(
    const picojson::value& args, picojson::object& out) {
  PlatformResult result = NFCAdapter::GetInstance()->AddTransactionEventListener(args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::RemoveTransactionEventListener(
    const picojson::value& args, picojson::object& out) {
  PlatformResult result = NFCAdapter::GetInstance()->RemoveTransactionEventListener(args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::AddActiveSecureElementChangeListener(
    const picojson::value& args, picojson::object& out) {
  PlatformResult result = NFCAdapter::GetInstance()->AddActiveSecureElementChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::RemoveActiveSecureElementChangeListener(
    const picojson::value& args, picojson::object& out) {
  PlatformResult result = NFCAdapter::GetInstance()->RemoveActiveSecureElementChangeListener();
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::GetCachedMessage(
    const picojson::value& args, picojson::object& out) {
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();

  PlatformResult ret = NFCAdapter::GetInstance()->GetCachedMessage(result_obj);
  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    ReportError(ret, &out);
  }
}

void NFCInstance::SetExclusiveModeForTransaction(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "transactionMode", out);

  bool transaction_mode = args.get("transactionMode").get<bool>();
  PlatformResult result = NFCAdapter::GetInstance()->SetExclusiveModeForTransaction(
        transaction_mode);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::ReadNDEF(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "id", out);

  int tag_id = static_cast<int>(args.get("id").get<double>());
  LoggerD("Tag id: %d", tag_id);

  PlatformResult result = NFCAdapter::GetInstance()->TagReadNDEF(tag_id, args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::WriteNDEF(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "id", out);

  int tag_id = static_cast<int>(args.get("id").get<double>());
  LoggerD("Tag id: %d", tag_id);

  PlatformResult result = NFCAdapter::GetInstance()->TagWriteNDEF(tag_id, args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::Transceive(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");
  CHECK_EXIST(args, "id", out);
  int tag_id = static_cast<int>(args.get("id").get<double>());
  LoggerD("Tag id: %d", tag_id);

  PlatformResult result = NFCAdapter::GetInstance()->TagTransceive(tag_id, args);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::SetReceiveNDEFListener(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "id", out);

  int peer_id = (int)args.get("id").get<double>();
  PlatformResult result = NFCAdapter::GetInstance()->SetReceiveNDEFListener(peer_id);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::UnsetReceiveNDEFListener(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "id", out);

  int peer_id = (int)args.get("id").get<double>();
  PlatformResult result = NFCAdapter::GetInstance()->UnsetReceiveNDEFListener(peer_id);
  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::SendNDEF(
    const picojson::value& args, picojson::object& out) {

  CHECK_EXIST(args, "id", out);

  int peer_id = static_cast<int>(args.get("id").get<double>());
  LoggerD("Peer id: %d", peer_id);

  PlatformResult result = NFCAdapter::GetInstance()->sendNDEF(peer_id, args);

  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::ToByte(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::NDEFMessageToByte(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    ReportError(ret, &out);
  }
}

//Message related methods
void NFCInstance::NDEFMessageContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFMessage(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    ReportError(ret, &out);
  }
}

void NFCInstance::NDEFRecordContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFRecord(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    ReportError(ret, &out);
  }
}

void NFCInstance::NDEFRecordTextContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFRecordText(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    ReportError(ret, &out);
  }
}

void NFCInstance::NDEFRecordURIContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFRecordURI(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    ReportError(ret, &out);
  }
}

void NFCInstance::NDEFRecordMediaContructor(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  picojson::value result = picojson::value(picojson::object());
  picojson::object& result_obj = result.get<picojson::object>();
  PlatformResult ret = NFCMessageUtils::ReportNDEFRecordMedia(args, result_obj);

  if (ret.IsSuccess()) {
    ReportSuccess(result, out);
  } else {
    ReportError(ret, &out);
  }
}

// NFCTag attributes getters
void NFCInstance::TagTypeGetter(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");

  CHECK_EXIST(args, "id", out);
  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  // Function below throws exception if core API call fails
  bool is_connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &is_connected);

  if (result.IsError()) {
    ReportError(result, &out);
    return;
  }

  if (!is_connected) {
    LoggerE("Tag with id %d is not connected anymore", tag_id);
    // If tag is not connected then attribute's value
    // should be undefined
    ReportError(out);
    return;
  }

  std::string tag_type = "";
  result = NFCAdapter::GetInstance()->TagTypeGetter(tag_id, &tag_type);

  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(tag_type), out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::TagIsSupportedNDEFGetter(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");

  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  // Function below throws exception if core API call fails
  bool is_connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &is_connected);

  if (result.IsError()) {
    ReportError(result, &out);
    return;
  }

  if (!is_connected) {
    LoggerE("Tag with id %d is not connected anymore", tag_id);
    // If tag is not connected then attribute's value
    // should be undefined
    ReportError(out);
    return;
  }

  bool is_supported = false;
  result = NFCAdapter::GetInstance()->TagIsSupportedNDEFGetter(tag_id, &is_supported);
  if (result.IsSuccess()) {
    ReportSuccess(picojson::value(is_supported), out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::TagNDEFSizeGetter(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");

  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  // Function below throws exception if core API call fails
  bool is_connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &is_connected);

  if (result.IsError()) {
    ReportError(result, &out);
    return;
  }

  if (!is_connected) {
    LoggerE("Tag with id %d is not connected anymore", tag_id);
    // If tag is not connected then attribute's value
    // should be undefined
    ReportError(out);
    return;
  }

  unsigned int ndef_size;
  result = NFCAdapter::GetInstance()->TagNDEFSizeGetter(tag_id, &ndef_size);

  if (result.IsSuccess()) {
    ReportSuccess(picojson::value((double)ndef_size), out);
  } else {
    ReportError(result, &out);
  }
}

void NFCInstance::TagPropertiesGetter(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");

  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  // Function below throws exception if core API call fails
  bool is_connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &is_connected);

  if (result.IsError()) {
    ReportError(result, &out);
    return;
  }

  if (!is_connected) {
    LoggerE("Tag with id %d is not connected anymore", tag_id);
    // If tag is not connected then attribute's value
    // should be undefined
    ReportError(out);
    return;
  }

  NFCTagPropertiesT prop;

  result = NFCAdapter::GetInstance()->TagPropertiesGetter(tag_id, &prop);
  if (result.IsSuccess()) {
    picojson::value properties = picojson::value(picojson::array());
    picojson::array& properties_array = properties.get<picojson::array>();
    for (auto it = prop.begin() ; it != prop.end(); it++) {
      picojson::value val = picojson::value(picojson::object());
      picojson::object& obj = val.get<picojson::object>();

      picojson::value value_vector = picojson::value(picojson::array());
      picojson::array& value_vector_obj = value_vector.get<picojson::array>();

      for (size_t i = 0 ; i < it->second.size(); i++) {
        value_vector_obj.push_back(picojson::value(
            std::to_string(it->second[i])));
      }

      obj.insert(std::make_pair(it->first, value_vector));
      properties_array.push_back(val);
    }
    ReportSuccess(properties, out);
  } else {
    ReportError(result, &out);
  }

}

void NFCInstance::TagIsConnectedGetter(
    const picojson::value& args, picojson::object& out) {

  LoggerD("Entered");

  CHECK_EXIST(args, "id", out);
  int tag_id = (int)args.get("id").get<double>();
  LoggerD("Tag id: %d", tag_id);

  bool connected = false;
  PlatformResult result = NFCAdapter::GetInstance()->TagIsConnectedGetter(tag_id, &connected);

  if (result.IsSuccess()) {
    ReportSuccess(out);
  } else {
    ReportError(result, &out);
  }
}

} // namespace nfc
} // namespace extension
