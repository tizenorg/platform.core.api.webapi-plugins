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
 
#ifndef KEYMANAGER_KEYMANAGER_INSTANCE_H_
#define KEYMANAGER_KEYMANAGER_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace keymanager {

class KeyManagerInstance : public common::ParsedInstance {
 public:
  KeyManagerInstance();
  virtual ~KeyManagerInstance();

 private:
  void GetKeyAliasList(picojson::value const& args, picojson::object& out);
  void GetCertificateAliasList(picojson::value const& args,
                               picojson::object& out);
  void GetDataAliasList(picojson::value const& args, picojson::object& out);
  void GetKey(const picojson::value& args, picojson::object& out);
  void SaveKey(const picojson::value& args, picojson::object& out);
  void RemoveAlias(const picojson::value& args, picojson::object& out);
  void GenerateKeyPair(const picojson::value& args, picojson::object& out);
  void GetCertificate(const picojson::value& args, picojson::object& out);
  void SaveCertificate(const picojson::value& args, picojson::object& out);
  void LoadCertificateFromFile(const picojson::value& args,
                               picojson::object& out);
  void SaveData(const picojson::value& args, picojson::object& out);
  void GetData(const picojson::value& args, picojson::object& out);
  void CreateSignature(const picojson::value& args, picojson::object& out);
  void VerifySignature(const picojson::value& args, picojson::object& out);
  void LoadFromPKCS12File(const picojson::value& args, picojson::object& out);
  void AllowAccessControl(const picojson::value& args, picojson::object& out);
  void DenyAccessControl(const picojson::value& args, picojson::object& out);

  void IsDataNameFound(const picojson::value& args, picojson::object& out);
};

} // namespace keymanager
} // namespace extension

#endif // KEYMANAGER_KEYMANAGER_INSTANCE_H_
