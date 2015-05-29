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
 
#include "keymanager/keymanager_observers.h"
#include "common/extension.h"
#include "common/logger.h"
#include "common/task-queue.h"

namespace extension {
namespace keymanager {

using common::PlatformResult;
using common::ErrorCode;

CommonObserver::CommonObserver(KeyManagerListener* _listener, double _callbackId):
  listener(_listener), callbackId(_callbackId) {

}

SaveKeyObserver::SaveKeyObserver(KeyManagerListener* listener, double callbackId):
  CommonObserver(listener, callbackId) {

}

void SaveKeyObserver::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  std::string message =  "Failed to save key";
  switch (error) {
    case CKM_API_ERROR_INPUT_PARAM:
      code = ErrorCode::INVALID_VALUES_ERR;
      break;
    case CKM_API_ERROR_DB_ALIAS_EXISTS:
      code = ErrorCode::INVALID_VALUES_ERR;
      message = "Key alias already exists";
      break;
  }
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnSaveKey, listener, callbackId,
    PlatformResult(code, message
  )));
}

void SaveKeyObserver::ReceivedSaveKey() {
  LoggerD("Enter");
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnSaveKey, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR)));
}

CreateKeyObserver::CreateKeyObserver(KeyManagerListener* listener, double callbackId):
    CommonObserver(listener, callbackId) {}

void CreateKeyObserver::ReceivedCreateKeyPairDSA() {
  LoggerD("Enter");
  CallSuccess();
}

void CreateKeyObserver::ReceivedCreateKeyPairECDSA() {
  LoggerD("Enter");
  CallSuccess();
}

void CreateKeyObserver::ReceivedCreateKeyPairRSA() {
  LoggerD("Enter");
  CallSuccess();
}

void CreateKeyObserver::CallSuccess() {
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnCreateKeyPair, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR)));
}

void CreateKeyObserver::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  std::string message = "Failed to create key pair";
  switch (error) {
    case CKM_API_ERROR_INPUT_PARAM:
      code = ErrorCode::INVALID_VALUES_ERR;
      break;
    case CKM_API_ERROR_DB_ALIAS_EXISTS:
      code = ErrorCode::INVALID_VALUES_ERR;
      message = "Key alias already exists";
      break;
  }
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnCreateKeyPair, listener, callbackId,
    PlatformResult(code, message)));
}

SaveCertObserver::SaveCertObserver(KeyManagerListener* listener, double callbackId):
    CommonObserver(listener, callbackId) {
}

void SaveCertObserver::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  if (error == CKM_API_ERROR_INPUT_PARAM) {
    code = ErrorCode::INVALID_VALUES_ERR;
  }
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnSaveCert, listener, callbackId,
    PlatformResult(code, "Failed to save certificate")));
}

void SaveCertObserver::ReceivedSaveCertificate() {
  LoggerD("Enter");
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnSaveCert, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR)));
}

LoadFileCert::LoadFileCert(KeyManagerListener* _listener,
    double callbackId,
    const std::string &_password,
    const std::string &_alias,
    bool _extractable):
    callback_id(callbackId),
    password(_password),
    alias(_alias),
    extractable(_extractable),
    file_content(""),
    listener(_listener) {
}

void LoadFileCert::AppendBuffer(guint8* buffer, gssize size) {
  file_content.append(buffer, buffer + size);
}

void LoadFileCert::OnError(const common::PlatformResult& result) {
  listener->OnCertFileLoaded(this, result);
}

void LoadFileCert::OnFileLoaded() {
  listener->OnCertFileLoaded(this, PlatformResult(ErrorCode::NO_ERROR));
}

SaveDataObserver::SaveDataObserver(KeyManagerListener* listener, double callbackId):
    CommonObserver(listener, callbackId) {
}

void SaveDataObserver::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  if (error == CKM_API_ERROR_INPUT_PARAM) {
    code = ErrorCode::INVALID_VALUES_ERR;
  }
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnSaveData, listener, callbackId,
    PlatformResult(code, "Failed to save data")));
}

void SaveDataObserver::ReceivedSaveData() {
  LoggerD("Enter");
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnSaveData, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR)));
}

CreateSignatureObserver::CreateSignatureObserver(KeyManagerListener* listener, double callbackId):
    CommonObserver(listener, callbackId) {
}

void CreateSignatureObserver::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  if (error == CKM_API_ERROR_INPUT_PARAM) {
    code = ErrorCode::INVALID_VALUES_ERR;
  }
  CKM::RawBuffer empty;
 common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnCreateSignature, listener, callbackId,
    PlatformResult(code, "Failed to create signature"), empty));
}

void CreateSignatureObserver::ReceivedCreateSignature(CKM::RawBuffer&& buffer) {
  LoggerD("Enter");
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnCreateSignature, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR), buffer));
}

VerifySignatureObserver::VerifySignatureObserver(KeyManagerListener* listener, double callbackId):
    CommonObserver(listener, callbackId) {
}

void VerifySignatureObserver::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  if (error == CKM_API_ERROR_INPUT_PARAM) {
    code = ErrorCode::INVALID_VALUES_ERR;
  } else if (error == CKM_API_ERROR_VERIFICATION_FAILED) {
    code = ErrorCode::VALIDATION_ERR;
  }
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnVerifySignature, listener, callbackId,
    PlatformResult(code, "Siganture verification failed")));
}

void VerifySignatureObserver::ReceivedVerifySignature() {
  LoggerD("Enter");
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnVerifySignature, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR)));
}

LoadFilePKCS12::LoadFilePKCS12(KeyManagerListener* listener,
    double callback_id,
    const std::string &password,
    const std::string &key_alias,
    const std::string &cert_alias):
    callback_id(callback_id),
    password(password),
    key_alias(key_alias),
    cert_alias(cert_alias),
    listener(listener) {
}

void LoadFilePKCS12::AppendBuffer(guint8* buffer, gssize size) {
  file_content.insert(file_content.end(), buffer, buffer + size);
}

void LoadFilePKCS12::OnError(const common::PlatformResult& result) {
  listener->OnPKCS12FileLoaded(this, result);
}

void LoadFilePKCS12::OnFileLoaded() {
  listener->OnPKCS12FileLoaded(this, PlatformResult(ErrorCode::NO_ERROR));
}

SavePKCS12Observer::SavePKCS12Observer(KeyManagerListener* listener, double callback_id):
    CommonObserver(listener, callback_id),
    cert_saved(false), key_saved(false) {}

void SavePKCS12Observer::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  if (error == CKM_API_ERROR_INPUT_PARAM) {
    code = ErrorCode::INVALID_VALUES_ERR;
  }
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnSavePKCS12, listener, callbackId,
    PlatformResult(code, "Failed to save pkcs12 file")));
}

void SavePKCS12Observer::ReceivedSaveKey() {
  LoggerD("Enter");
  key_saved = true;
  if (cert_saved) {
   common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnSavePKCS12, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR)));
  }
}

void SavePKCS12Observer::ReceivedSaveCertificate() {
  LoggerD("Enter");
  cert_saved = true;
  if (key_saved) {
    common::TaskQueue::GetInstance().Async(std::bind(
      &KeyManagerListener::OnSavePKCS12, listener, callbackId,
      PlatformResult(ErrorCode::NO_ERROR)));
  }
}

AllowAccessObserver::AllowAccessObserver(KeyManagerListener* listener, double callbackId):
    CommonObserver(listener, callbackId) {
}

void AllowAccessObserver::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  if (error == CKM_API_ERROR_DB_ALIAS_UNKNOWN) {
    code = ErrorCode::NOT_FOUND_ERR;
  }
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnAllowAccess, listener, callbackId,
    PlatformResult(code, "Failed to grant access")));
}

void AllowAccessObserver::ReceivedSetPermission() {
  LoggerD("Enter");
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnAllowAccess, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR)));
}

DenyAccessObserver::DenyAccessObserver(KeyManagerListener* listener, double callbackId):
    CommonObserver(listener, callbackId) {
}

void DenyAccessObserver::ReceivedError(int error) {
  LoggerD("Enter, error: %d", error);
  ErrorCode code = ErrorCode::UNKNOWN_ERR;
  if (error == CKM_API_ERROR_DB_ALIAS_UNKNOWN) {
    code = ErrorCode::NOT_FOUND_ERR;
  }
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnDenyAccess, listener, callbackId,
    PlatformResult(code, "Failed to deny access")));
}

void DenyAccessObserver::ReceivedSetPermission() {
  LoggerD("Enter");
  common::TaskQueue::GetInstance().Async(std::bind(
    &KeyManagerListener::OnDenyAccess, listener, callbackId,
    PlatformResult(ErrorCode::NO_ERROR)));
}

} // namespace keymanager
} // namespace extension
