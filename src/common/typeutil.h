// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_TYPEUTIL_H_
#define COMMON_TYPEUTIL_H_

#include <string>

#include "common/picojson.h"

namespace common {

namespace WIDLTypeValidator {

enum WIDLType {
  StringType,
  ArrayType,
  ObjectType,
  BooleanType,
  DoubleType,
  FloatType,
  ByteType,
  OctetType,
  ShortType,
  UnsignedShortType,
  LongType,
  UnsignedLongType,
  LongLongType,
  UnsignedLongLongType,
};

using picojson::object;
using picojson::value;
using picojson::array;

template <WIDLType E>
bool IsType(const value& arg, const char* name);

#define IS_TYPE_NORANGE(wtype, ctype) \
    template <> bool IsType<wtype>(const value& arg, const char* name) { \
      return arg.get(name).is<ctype>(); \
    }
IS_TYPE_NORANGE(StringType, std::string)
IS_TYPE_NORANGE(ArrayType, array)
IS_TYPE_NORANGE(ObjectType, object)
IS_TYPE_NORANGE(BooleanType, bool)
IS_TYPE_NORANGE(DoubleType, double)
IS_TYPE_NORANGE(FloatType, double)
#undef IS_TYPE_NORANGE

#define IS_TYPE_RANGE(wtype, min, max) \
      template <> bool IsType<wtype>(const value& arg, const char* name) { \
        const value& v = arg.get(name); \
        if (!v.is<double>()) return false; \
        double n = v.get<double>(); \
        if (n < min || n > max) return false; \
        return true; \
      }
IS_TYPE_RANGE(ByteType, -128, 127)
IS_TYPE_RANGE(OctetType, 0, 255)
IS_TYPE_RANGE(ShortType, -32768, 32767)
IS_TYPE_RANGE(UnsignedShortType, 0, 65535)
IS_TYPE_RANGE(LongType, -2147483648, 2147483647)
IS_TYPE_RANGE(UnsignedLongType, 0, 4294967295)
IS_TYPE_RANGE(LongLongType, -9223372036854775808, 9223372036854775807)
IS_TYPE_RANGE(UnsignedLongLongType, 0, 18446744073709551615)
#undef IS_TYPE_RANGE

}  // namespace WIDLTypeValidator

namespace CTypeConveter {

enum CType {
  StringType,
  ArrayType
};

}  // namespace CTypeConveter

}  // namespace common

#ifdef TEST_TYPEUTIL
#include <cstdlib>
#include <cstdio>
//using namespace common::WIDLTypeValidator;
int main(void) {
  const char *tc = 
      "{"
        "\"string\": \"string value\","
        "\"object\": {\"first\":1, \"second\":2},"
        "\"array\": [1,2,3,4],"
        "\"array2\": [1,\"two\",3,{\"obj\":\"obj!!\"}],"
        "\"boolean\": true,"
        "\"double\": 12345678.9999,"
        "\"float\": 12345.678,"
        "\"byte1\": -128,"
        "\"byte2\": 127,"
        "\"octet\": 255,"
        "\"short1\": -32768,"
        "\"short2\": 32767,"
        "\"ushort\": 65535,"
        "\"long1\": -2147483648,"
        "\"long2\": 2147483647,"
        "\"ulong\": 4294967295,"
        "\"longlong1\": -9223372036854775808,"
        "\"longlong2\": 9223372036854775807,"
        "\"ulonglong\": 18446744073709551615,"
        "\"zero\": 0"
      "}";
  picojson::value v;
  std::string err = picojson::parse(v, tc, tc + strlen(tc));
  printf("parse err : %s\n", err.c_str());


  using common::WIDLTypeValidator::WIDLType;
  using common::WIDLTypeValidator::IsType;

#define TC_POSITIVE(name, wtype) \
  printf("%s positive %s : %s\n", #wtype, \
         IsType<wtype>(v, name) ? "pass" : "fail", \
         v.get(name).to_str().c_str());
#define TC_NEGATIVE(name, wtype) \
  printf("%s negative %s : %s\n", #wtype, \
         !IsType<wtype>(v, name) ? "pass" : "fail", \
         v.get(name).to_str().c_str());

  TC_POSITIVE("string", WIDLType::StringType);
  TC_NEGATIVE("object", WIDLType::StringType);
  TC_NEGATIVE("array", WIDLType::StringType);
  TC_NEGATIVE("array2", WIDLType::StringType);
  TC_NEGATIVE("boolean", WIDLType::StringType);
  TC_NEGATIVE("double", WIDLType::StringType);
  TC_NEGATIVE("longlong", WIDLType::StringType);
  TC_NEGATIVE("ulonglong", WIDLType::StringType);

  TC_POSITIVE("object", WIDLType::ObjectType);
  TC_NEGATIVE("string", WIDLType::ObjectType);
  TC_NEGATIVE("array", WIDLType::ObjectType);
  TC_NEGATIVE("array2", WIDLType::ObjectType);
  TC_NEGATIVE("boolean", WIDLType::ObjectType);
  TC_NEGATIVE("double", WIDLType::ObjectType);
  TC_NEGATIVE("longlong", WIDLType::ObjectType);
  TC_NEGATIVE("ulonglong", WIDLType::ObjectType);

  TC_POSITIVE("array", WIDLType::ArrayType);
  TC_POSITIVE("array2", WIDLType::ArrayType);
  TC_NEGATIVE("string", WIDLType::ArrayType);
  TC_NEGATIVE("boolean", WIDLType::ArrayType);
  TC_NEGATIVE("double", WIDLType::ArrayType);
  TC_NEGATIVE("longlong", WIDLType::ArrayType);
  TC_NEGATIVE("ulonglong", WIDLType::ArrayType);

  TC_POSITIVE("boolean", WIDLType::BooleanType);
  TC_NEGATIVE("object", WIDLType::BooleanType);
  TC_NEGATIVE("string", WIDLType::BooleanType);
  TC_NEGATIVE("array", WIDLType::BooleanType);
  TC_NEGATIVE("array2", WIDLType::BooleanType);
  TC_NEGATIVE("double", WIDLType::BooleanType);
  TC_NEGATIVE("longlong", WIDLType::BooleanType);
  TC_NEGATIVE("ulonglong", WIDLType::BooleanType);

  TC_POSITIVE("byte1", WIDLType::ByteType);
  TC_POSITIVE("byte2", WIDLType::ByteType);
  TC_NEGATIVE("object", WIDLType::ByteType);
  TC_NEGATIVE("string", WIDLType::ByteType);
  TC_NEGATIVE("array", WIDLType::ByteType);
  TC_NEGATIVE("array2", WIDLType::ByteType);
  TC_NEGATIVE("boolean", WIDLType::ByteType);
  TC_NEGATIVE("double", WIDLType::ByteType);
  TC_NEGATIVE("longlong", WIDLType::ByteType);
  TC_NEGATIVE("ulonglong", WIDLType::ByteType);

  TC_POSITIVE("octet", WIDLType::OctetType);
  TC_POSITIVE("zero", WIDLType::OctetType);
  TC_NEGATIVE("byte1", WIDLType::OctetType);
  TC_NEGATIVE("object", WIDLType::OctetType);
  TC_NEGATIVE("string", WIDLType::OctetType);
  TC_NEGATIVE("array", WIDLType::OctetType);
  TC_NEGATIVE("array2", WIDLType::OctetType);
  TC_NEGATIVE("boolean", WIDLType::OctetType);
  TC_NEGATIVE("double", WIDLType::OctetType);
  TC_NEGATIVE("longlong", WIDLType::OctetType);
  TC_NEGATIVE("ulonglong", WIDLType::OctetType);

  TC_POSITIVE("short1", WIDLType::ShortType);
  TC_POSITIVE("short2", WIDLType::ShortType);
  TC_NEGATIVE("object", WIDLType::ShortType);
  TC_NEGATIVE("string", WIDLType::ShortType);
  TC_NEGATIVE("array", WIDLType::ShortType);
  TC_NEGATIVE("array2", WIDLType::ShortType);
  TC_NEGATIVE("boolean", WIDLType::ShortType);
  TC_NEGATIVE("double", WIDLType::ShortType);
  TC_NEGATIVE("longlong", WIDLType::ShortType);
  TC_NEGATIVE("ulonglong", WIDLType::ShortType);

  TC_POSITIVE("ushort", WIDLType::UnsignedShortType);
  TC_POSITIVE("zero", WIDLType::UnsignedShortType);
  TC_NEGATIVE("object", WIDLType::UnsignedShortType);
  TC_NEGATIVE("string", WIDLType::UnsignedShortType);
  TC_NEGATIVE("array", WIDLType::UnsignedShortType);
  TC_NEGATIVE("array2", WIDLType::UnsignedShortType);
  TC_NEGATIVE("boolean", WIDLType::UnsignedShortType);
  TC_NEGATIVE("double", WIDLType::UnsignedShortType);
  TC_NEGATIVE("longlong", WIDLType::UnsignedShortType);
  TC_NEGATIVE("ulonglong", WIDLType::UnsignedShortType);

  TC_POSITIVE("long1", WIDLType::LongType);
  TC_POSITIVE("long2", WIDLType::LongType);
  TC_NEGATIVE("object", WIDLType::LongType);
  TC_NEGATIVE("string", WIDLType::LongType);
  TC_NEGATIVE("array", WIDLType::LongType);
  TC_NEGATIVE("array2", WIDLType::LongType);
  TC_NEGATIVE("boolean", WIDLType::LongType);
  TC_NEGATIVE("longlong", WIDLType::LongType);
  TC_NEGATIVE("ulonglong", WIDLType::LongType);

  TC_POSITIVE("ulong", WIDLType::UnsignedLongType);
  TC_POSITIVE("zero", WIDLType::UnsignedLongType);
  TC_NEGATIVE("object", WIDLType::UnsignedLongType);
  TC_NEGATIVE("string", WIDLType::UnsignedLongType);
  TC_NEGATIVE("array", WIDLType::UnsignedLongType);
  TC_NEGATIVE("array2", WIDLType::UnsignedLongType);
  TC_NEGATIVE("boolean", WIDLType::UnsignedLongType);
  TC_NEGATIVE("longlong", WIDLType::UnsignedLongType);
  TC_NEGATIVE("ulonglong", WIDLType::UnsignedLongType);

  TC_POSITIVE("longlong1", WIDLType::LongLongType);
  TC_POSITIVE("longlong2", WIDLType::LongLongType);
  TC_NEGATIVE("object", WIDLType::LongLongType);
  TC_NEGATIVE("string", WIDLType::LongLongType);
  TC_NEGATIVE("array", WIDLType::LongLongType);
  TC_NEGATIVE("array2", WIDLType::LongLongType);
  TC_NEGATIVE("boolean", WIDLType::LongLongType);
  TC_NEGATIVE("ulonglong", WIDLType::LongLongType);

  TC_POSITIVE("ulonglong", WIDLType::UnsignedLongLongType);
  TC_POSITIVE("zero", WIDLType::UnsignedLongLongType);
  TC_NEGATIVE("object", WIDLType::UnsignedLongLongType);
  TC_NEGATIVE("string", WIDLType::UnsignedLongLongType);
  TC_NEGATIVE("array", WIDLType::UnsignedLongLongType);
  TC_NEGATIVE("array2", WIDLType::UnsignedLongLongType);
  TC_NEGATIVE("boolean", WIDLType::UnsignedLongLongType);

#undef TC_POSITIVE
#undef TC_NEGATIVE
}
#endif  // TEST_TYPEUTIL

#endif  // COMMON_TYPEUTIL_H_
