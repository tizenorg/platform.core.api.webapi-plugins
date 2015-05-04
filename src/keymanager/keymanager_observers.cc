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
    callbackId(callbackId),
    password(_password),
    alias(_alias),
    extractable(_extractable),
    fileContent(""),
    buffer(NULL),
    listener(_listener) {}

void LoadFileCert::LoadFileAsync(const std::string& fileUri) {
  LoggerD("Enter");
  GFile* file = g_file_new_for_uri(fileUri.c_str());
  g_file_read_async(file, G_PRIORITY_DEFAULT, NULL, OnFileRead, this);
}

void LoadFileCert::OnFileRead(GObject* source_object,
  GAsyncResult* res, gpointer user_data) {
  LoggerD("Enter");
  LoadFileCert* This = static_cast<LoadFileCert*>(user_data);
  GError* err = NULL;
  GFileInputStream* stream = g_file_read_finish(G_FILE(source_object),
    res, &err);
  g_object_unref(source_object);
  if (stream == NULL) {
      LoggerE("Failed to read file: %d", err->code);
      if (err->code == G_FILE_ERROR_NOENT) {
        This->listener->OnCertFileLoaded(This,
          PlatformResult(ErrorCode::NOT_FOUND_ERR, "Certificate file not found"));
      } else {
        This->listener->OnCertFileLoaded(This,
          PlatformResult(ErrorCode::IO_ERR, "Failed to load certificate file"));
      }
      return;
  }

  This->buffer = new guint8[4096];
  g_input_stream_read_async(G_INPUT_STREAM(stream), This->buffer, 4096,
    G_PRIORITY_DEFAULT, NULL, OnStreamRead, This);
}

void LoadFileCert::OnStreamRead(GObject* source_object,
  GAsyncResult* res, gpointer user_data) {
  LoggerD("Enter");

  LoadFileCert* This = static_cast<LoadFileCert*>(user_data);
  gssize size = g_input_stream_read_finish(G_INPUT_STREAM(source_object),
    res, NULL);
  switch (size){
    case -1:
      LoggerE("Error occured");
      This->listener->OnCertFileLoaded(This,
        PlatformResult(ErrorCode::IO_ERR, "Failed to load certificate file"));
      g_object_unref(source_object);
      break;
    case 0:
      LoggerD("End of file");
      This->listener->OnCertFileLoaded(This,
        PlatformResult(ErrorCode::NO_ERROR));
      g_object_unref(source_object);
      break;
    default:
      This->fileContent.append(This->buffer, This->buffer + size);
      g_input_stream_read_async(G_INPUT_STREAM(source_object), This->buffer,
        4096, G_PRIORITY_DEFAULT, NULL, OnStreamRead, This);
  }
}

LoadFileCert::~LoadFileCert() {
  delete[] buffer;
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

} // namespace keymanager
} // namespace extension
