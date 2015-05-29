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
#ifndef KEYMANAGER_ASYNC_FILE_READER_H_
#define KEYMANAGER_ASYNC_FILE_READER_H_

#include <glib.h>
#include <gio/gio.h>
#include "common/platform_result.h"

namespace extension {
namespace keymanager {

class AsyncFileReader {
public:
  AsyncFileReader();
  void LoadFileAsync(const std::string &file_uri);
  virtual ~AsyncFileReader();

protected:
  virtual void AppendBuffer(guint8* buffer, gssize size) = 0;
  virtual void OnError(const common::PlatformResult& result) = 0;
  virtual void OnFileLoaded() = 0;

private:
  guint8* buffer_;

  static void OnFileRead(GObject *source_object,
    GAsyncResult *res,
    gpointer user_data);
  static void OnStreamRead(GObject *source_object,
    GAsyncResult *res,
    gpointer user_data);
};

} // namespace keymanager
} // namespace extension

#endif // KEYMANAGER_ASYNC_FILE_READER_H_

