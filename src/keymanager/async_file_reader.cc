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
 
#include "keymanager/async_file_reader.h"
#include "common/logger.h"

namespace extension {
namespace keymanager {

const gsize kBufferSize = 4096;

using common::PlatformResult;
using common::ErrorCode;

AsyncFileReader::AsyncFileReader(): buffer_(nullptr) {}

void AsyncFileReader::LoadFileAsync(const std::string& file_uri) {
  LoggerD("Enter");
  GFile* file = g_file_new_for_uri(file_uri.c_str());
  g_file_read_async(file, G_PRIORITY_DEFAULT, nullptr, OnFileRead, this);
}

void AsyncFileReader::OnFileRead(GObject* source_object,
  GAsyncResult* res, gpointer user_data) {
  LoggerD("Enter");
  AsyncFileReader* This = static_cast<AsyncFileReader*>(user_data);
  GError* err = nullptr;
  GFileInputStream* stream = g_file_read_finish(G_FILE(source_object),
    res, &err);
  g_object_unref(source_object);
  if (stream == nullptr) {
      LoggerE("Failed to read file: %d", err->code);
      if (err->code == G_FILE_ERROR_NOENT) {
        This->OnError(PlatformResult(ErrorCode::NOT_FOUND_ERR,
            "File not found"));
      } else {
        This->OnError(PlatformResult(ErrorCode::IO_ERR,
            "Failed to load file"));
      }
      return;
  }

  This->buffer_ = new guint8[kBufferSize];
  g_input_stream_read_async(G_INPUT_STREAM(stream), This->buffer_, kBufferSize,
    G_PRIORITY_DEFAULT, nullptr, OnStreamRead, This);
}

void AsyncFileReader::OnStreamRead(GObject* source_object,
  GAsyncResult* res, gpointer user_data) {
  LoggerD("Enter");

  AsyncFileReader* This = static_cast<AsyncFileReader*>(user_data);
  gssize size = g_input_stream_read_finish(G_INPUT_STREAM(source_object),
    res, nullptr);
  switch (size){
    case -1:
      LoggerE("Error occured");
      This->OnError(PlatformResult(ErrorCode::IO_ERR,
          "Failed to load file"));
      g_object_unref(source_object);
      break;
    case 0:
      LoggerD("End of file");
      This->OnFileLoaded();
      g_object_unref(source_object);
      break;
    default:
      This->AppendBuffer(This->buffer_, size);
      g_input_stream_read_async(G_INPUT_STREAM(source_object), This->buffer_,
        kBufferSize, G_PRIORITY_DEFAULT, nullptr, OnStreamRead, This);
  }
}

AsyncFileReader::~AsyncFileReader() {
  delete[] buffer_;
}

} // namespace keymanager
} // namespace extension