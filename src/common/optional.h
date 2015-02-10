// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_OPTIONAL_H_
#define COMMON_OPTIONAL_H_

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 8
  #define ALIGNAS_TYPE(x) alignas(x)
  #define ALIGNAS(x) alignas(x)
#else
  #define ALIGNAS_TYPE(x) __attribute__((__aligned__(__alignof(x))))
  #define ALIGNAS(x) __attribute__((aligned(x)))
#endif


#include <cassert>
#include <new>
#include <utility>
#include <type_traits>

namespace common {

template<typename T>
class optional {
  static_assert(!std::is_reference<T>::value, "reference is not supported");

 public:
  optional() : exist_(false) {}
  explicit optional(std::nullptr_t) : exist_(false) {}
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
    assert(exist_);
    return reinterpret_cast<const T*>(value_);
  }

  T* get() {
    assert(exist_);
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
