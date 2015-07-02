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

#ifndef DOWNLOAD_DOWNLOAD_INSTANCE_H_
#define DOWNLOAD_DOWNLOAD_INSTANCE_H_

#include <glib.h>
#include <download.h>
#include <system_info.h>

#include <memory>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <mutex>

#include "common/extension.h"

template <class T>
  inline std::string to_string(const T& t) {
      std::stringstream ss;
      ss << t;
      return ss.str();
  }

namespace extension {
namespace download {

class DownloadInstance : public common::ParsedInstance {
 public:
  DownloadInstance();
  virtual ~DownloadInstance();

 private:
  void DownloadManagerStart
    (const picojson::value& args, picojson::object& out);
  void DownloadManagerCancel
    (const picojson::value& args, picojson::object& out);
  void DownloadManagerPause
    (const picojson::value& args, picojson::object& out);
  void DownloadManagerResume
    (const picojson::value& args, picojson::object& out);
  void DownloadManagerGetstate
    (const picojson::value& args, picojson::object& out);
  void DownloadManagerGetdownloadrequest
    (const picojson::value& args, picojson::object& out);
  void DownloadManagerGetmimetype
    (const picojson::value& args, picojson::object& out);
  void DownloadManagerSetlistener
    (const picojson::value& args, picojson::object& out);

  bool GetDownloadID(const int callback_id, int& download_id);

  static void OnStateChanged
    (int download_id, download_state_e state, void* user_data);
  static void progress_changed_cb
    (int download_id, long long unsigned received, void* user_data);
  static void OnStart(int download_id, void* user_data);

  static gboolean OnProgressChanged(void* user_data);
  static gboolean OnFinished(void* user_data);
  static gboolean OnPaused(void* user_data);
  static gboolean OnCanceled(void* user_data);
  static gboolean OnFailed(void* user_data);
  static bool CheckInstance(DownloadInstance* instance);

  struct DownloadInfo {
    int callbackId;
    std::string url;
    std::string destination;
    std::string file_name;
    std::string http_header;
    download_network_type_e network_type;

    int download_id;
    long long unsigned file_size;
  };

  struct DownloadCallback {
    int callbackId;
    int downloadId;
    DownloadInstance* instance;
    unsigned long long received;
    download_state_e state;
  };

  typedef std::map<int, DownloadCallback*> DownloadCallbackMap;
  typedef std::shared_ptr<DownloadInfo> DownloadInfoPtr;
  typedef std::map<int, DownloadInfoPtr> DownloadInfoMap;

  static std::mutex instances_mutex_;
  static std::vector<DownloadInstance*> instances_;

  DownloadCallbackMap downCbMap;
  DownloadInfoMap diMap;
};

}  // namespace download
}  // namespace extension

#endif  // DOWNLOAD_DOWNLOAD_INSTANCE_H_
