// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

