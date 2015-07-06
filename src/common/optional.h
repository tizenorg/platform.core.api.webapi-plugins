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

#ifndef COMMON_OPTIONAL_H_
#define COMMON_OPTIONAL_H_

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 8
  #define ALIGNAS_TYPE(x) alignas(x)
  #define ALIGNAS(x) alignas(x)
#else
  #define ALIGNAS_TYPE(x) __attribute__((__aligned__(__alignof(x))))
  #define ALIGNAS(x) __attribute__((aligned(x)))
#endif


#include <new>
#include <utility>
#include <type_traits>
#include "common/assert.h"

namespace common {

template<typename T>
class optional {
  static_assert(!std::is_reference<T>::value, "reference is not supported");

 public:
  optional() : exist_(false) {}
  optional(std::nullptr_t) : exist_(false) {}
  optional(const optional& rhs): exist_(rhs.exist_) { create(rhs); }
  optional(optional&& rhs): exist_(rhs.exist_) { create(std::move(rhs)); }
  optional(const T& v): exist_(true) { create(v); }
  ~optional() { if (exist_) destroy(); }

  bool operator !() const { return !exist_; }
  explicit operator bool() const { return exist_; }

  optional& operator = (const optional& rhs) { return assign(rhs); }
  optional& operator = (optional&& rhs) { return assign(std::move(rhs)); }
  optional& operator = (std::nullptr_t) {if (exist_) cleanup(); return *this; }
  optional& operator = (const T& v) { return assign(v); }

  const T& operator * () const { return *get(); }
  T& operator * () { return *get(); }
  const T* operator -> () const { return get(); }
  T* operator -> () { return get(); }

 private:
  void create(const T& v) {
    new(value_) T(v);
  }
  void create(T&& v) {
    new(value_) T(std::move(v));
  }
  void create(const optional& rhs) {
    if (exist_) create(*rhs.get());
  }
  void create(optional&& rhs) {
    if (exist_) create(std::move(*rhs.get()));
  }
  void destroy() {
    get()->~T();
  }
  const T* get() const {
    Assert(exist_);
    return reinterpret_cast<const T*>(value_);
  }

  T* get() {
    Assert(exist_);
    return reinterpret_cast<T*>(value_);
  }

  void cleanup() {
    destroy();
    exist_ = false;
  }

  optional& assign(const T& v) {
    if (exist_) {
      *get() = v;
    } else {
      create(v);
      exist_ = true;
    }
    return *this;
  }

  optional& assign(const optional& rhs) {
    if (rhs.exist_) return assign(*rhs.get());
    if (!exist_) return *this;
    cleanup();
    return *this;
  }

  optional& assign(optional&& rhs) {
    if (rhs.exist_) return assign(std::move(*rhs.get()));
    if (!exist_) return *this;
    cleanup();
    return *this;
  }

  bool exist_;
  ALIGNAS_TYPE(T) char value_[sizeof(T)];
};

}  // namespace common

#undef ALIGNAS_TYPE
#undef ALIGNAS

#endif  // COMMON_OPTIONAL_H_
