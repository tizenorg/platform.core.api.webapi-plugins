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
 
#ifndef COMMON_DELETER_H_
#define COMMON_DELETER_H_

#include <cstdio>

namespace common {

template<typename T>
struct Deleter
{
  typedef std::function<void(T*)> Function;

  Deleter() : function_(NoDelete) {}
  explicit Deleter(Function const& function) : function_(function) {}

  void operator()(T* ptr) const {
    function_(ptr);
  }

  static void CallArrayDelete(T *ptr) { delete[] ptr; };
  static void CallDelete(T *ptr) { delete ptr; }
  static void CallFree(T *ptr) { free(ptr); }
  static void NoDelete(T *) {}

private:
  Function function_;
};

struct FileDeleter : public Deleter<FILE>
{
  FileDeleter() : Deleter(CloseFile) {}
  explicit FileDeleter(Function const& function) : Deleter(function) {};

  static void CloseFile(FILE *file) { fclose(file); }
};

}  // namespace common

#endif  // COMMON_DELETER_H_