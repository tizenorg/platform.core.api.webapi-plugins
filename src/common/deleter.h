// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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