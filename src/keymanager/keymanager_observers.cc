// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

} // namespace keymanager
} // namespace extension
