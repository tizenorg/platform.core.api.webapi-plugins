// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_LOGGER_H_
#define COMMON_LOGGER_H_

#include <dlog.h>

// Tizen 3.0 uses different debug flag (DLOG_DEBUG_ENABLE) which is always
// enabled, following code allows to disable logs with DLOG_DEBUG priority if
// TIZEN_DEBUG_ENABLE is not set.
// This code should be removed when DLOG_DEBUG_ENABLE flag is no longer set
// by default in dlog.h file.
#undef LOG_
#ifdef TIZEN_DEBUG_ENABLE
#define LOG_(id, prio, tag, fmt, arg...) \
  ({ do { \
    __dlog_print(id, prio, tag, "%s: %s(%d) > " fmt, __MODULE__, __func__, __LINE__, ##arg); \
  } while (0); })
#else  // TIZEN_DEBUG_ENABLE
#define LOG_(id, prio, tag, fmt, arg...) \
  ({ do { \
    if ((int)prio != DLOG_DEBUG) { \
      __dlog_print(id, prio, tag, "%s: %s(%d) > " fmt, __MODULE__, __func__, __LINE__, ##arg); \
    } \
  } while (0); })
#endif  // TIZEN_DEBUG_ENABLE

#undef SECURE_LOG_
#ifdef TIZEN_DEBUG_ENABLE
#define SECURE_LOG_(id, prio, tag, fmt, arg...) \
  ({ do { \
    __dlog_print(id, prio, tag, "%s: %s(%d) > [SECURE_LOG] " fmt, __MODULE__, __func__, __LINE__, ##arg); \
  } while (0); })
#else  // TIZEN_DEBUG_ENABLE
#define SECURE_LOG_(id, prio, tag, fmt, arg...) NOP(fmt, ##arg)
#endif  // TIZEN_DEBUG_ENABLE

#include <string>
#include <cstring>
#include <sstream>

#include "common/utils.h"

#undef LOGGER_TAG
#define LOGGER_TAG "WEBAPI_PLUGINS"

#define _LOGGER_LOG(prio, fmt, args...) \
  LOG_(LOG_ID_MAIN, prio, LOGGER_TAG, fmt, ##args)

#define _LOGGER_SLOG(prio, fmt, args...) \
  SECURE_LOG_(LOG_ID_MAIN, prio, LOGGER_TAG, fmt, ##args)

#define LoggerD(fmt, args...) _LOGGER_LOG(DLOG_DEBUG, fmt, ##args)
#define LoggerI(fmt, args...) _LOGGER_LOG(DLOG_INFO, fmt, ##args)
#define LoggerW(fmt, args...) _LOGGER_LOG(DLOG_WARN, fmt, ##args)
#define LoggerE(fmt, args...) _LOGGER_LOG(DLOG_ERROR, fmt, ##args)

#define SLoggerD(fmt, args...) _LOGGER_SLOG(DLOG_DEBUG, fmt, ##args)
#define SLoggerI(fmt, args...) _LOGGER_SLOG(DLOG_INFO, fmt, ##args)
#define SLoggerW(fmt, args...) _LOGGER_SLOG(DLOG_WARN, fmt, ##args)
#define SLoggerE(fmt, args...) _LOGGER_SLOG(DLOG_ERROR, fmt, ##args)

// A few definitions of macros that don't generate much code. These are used
// by LOGGER() and LOGGER_IF, etc. Since these are used all over our code, it's
// better to have compact code for these operations.
#ifdef TIZEN_DEBUG_ENABLE
#define COMPACT_LOG_DEBUG \
  LogMessage(__MODULE__, __func__, __LINE__, DLOG_DEBUG).stream()
#else
#define COMPACT_LOG_DEBUG \
  true ? (void) 0 : LogMessageVoidify() & (std::ostringstream())
#endif

#define COMPACT_LOG_INFO \
  LogMessage(__MODULE__, __func__, __LINE__, DLOG_INFO).stream()
#define COMPACT_LOG_WARN \
  LogMessage(__MODULE__, __func__, __LINE__, DLOG_WARN).stream()
#define COMPACT_LOG_ERROR \
  LogMessage(__MODULE__, __func__, __LINE__, DLOG_ERROR).stream()

#define LOGGER(priority) COMPACT_LOG_ ## priority
#define LOGGER_IF(priority, condition) \
  !(condition) ? (void) 0 : LogMessageVoidify() & (LOGGER(priority))

// This class more or less represents a particular log message.
// You create an instance of LogMessage and then stream stuff to it.
// When you finish streaming to it, ~LogMessage is called and the
// full message gets streamed to dlog.
//
// You shouldn't actually use LogMessage's constructor to log things,
// though. You should use the LOGGER() macro (and variants thereof) above.
class LogMessage {
 public:
  LogMessage(const char* file, const char* function, int line,
      log_priority priority);
  ~LogMessage();

  std::ostream& stream() { return stream_; }

 private:
  const char* file_;
  const char* function_;
  const int line_;
  log_priority priority_;

  std::ostringstream stream_;

  DISALLOW_COPY_AND_ASSIGN(LogMessage);
};

// This class is used to explicitly ignore values in the conditional
// logging macros. This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify {
public:
  LogMessageVoidify() {}

  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(std::ostream &) {}
};

// internal macros
#define LogAndReportError_2(error, object) \
  do { \
    LoggerE("Reporting error."); \
    ReportError(error, object); \
  } while(false)

#define LogAndReportError_3(error, object, log) \
  do { \
    LoggerE log; \
    ReportError(error, object); \
  } while(false)

#define LogAndReportError_X(_0, _1, _2, _3, FUNC, ...) FUNC

// usage:
// LogAndCreateResult(const common::PlatformResult& result, picojson::object* obj, [(const char* log, ...)])
//  LogAndReportError(result, &out);
//  LogAndReportError(result, &out, ("Failed to open archive."));
//  LogAndReportError(result, &out, ("Failed to open archive: %d.", 11));

#define LogAndReportError(...) \
  LogAndReportError_X(, ##__VA_ARGS__, \
     LogAndReportError_3(__VA_ARGS__), \
     LogAndReportError_2(__VA_ARGS__) \
  )

// internal macros
#define LogAndCreateResult_1(error_code) \
  ( \
    LoggerE("Creating PlatformResult with error"), \
    common::PlatformResult(error_code) \
  )

#define LogAndCreateResult_2(error_code, msg) \
  ( \
    LoggerE("Creating PlatformResult with error"), \
    common::PlatformResult(error_code, msg) \
  )

#define LogAndCreateResult_3(error_code, msg, log) \
  ( \
    LoggerE log, \
    common::PlatformResult(error_code, msg) \
  )

#define LogAndCreateResult_X(_0, _1, _2, _3, FUNC, ...) FUNC

// LogAndCreateResult(common::ErrorCode code, [const char* error_message, (const char* log, ...)])
// usage:
//  auto pr1 = LogAndCreateResult(ErrorCode::IO_ERR);
//  PlatformResult pr2 = LogAndCreateResult(ErrorCode::IO_ERR);
//  return LogAndCreateResult(ErrorCode::IO_ERR);
//
//  auto pr3 = LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.");
//  PlatformResult pr4 = LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.");
//  return LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.");
//
//  auto pr5 = LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.", ("Not a directory: reporting IO error"));
//  PlatformResult pr6 = LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.", ("Not a directory: reporting IO error"));
//  return LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.", ("Not a directory: reporting IO error"));
//
//  auto pr7 = LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.", ("Not a directory: reporting IO error: %d", 33));
//  PlatformResult pr8 = LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.", ("Not a directory: reporting IO error: %d", 33));
//  return LogAndCreateResult(ErrorCode::IO_ERR, "Not a directory.", ("Not a directory: reporting IO error: %d", 33));

#define LogAndCreateResult(...) \
  LogAndCreateResult_X(, ##__VA_ARGS__, \
     LogAndCreateResult_3(__VA_ARGS__), \
     LogAndCreateResult_2(__VA_ARGS__), \
     LogAndCreateResult_1(__VA_ARGS__) \
  )

// internal macros
#define LogAndCreateTizenError_1(error_name) \
  ( \
    LoggerE("Creating TizenError"), \
    common::error_name() \
  )

#define LogAndCreateTizenError_2(error_name, msg) \
  ( \
    LoggerE("Creating TizenError"), \
    common::error_name(msg) \
  )

#define LogAndCreateTizenError_3(error_name, msg, log) \
  ( \
    LoggerE log, \
    common::error_name(msg) \
  )

#define LogAndCreateTizenError_X(_0, _1, _2, _3, FUNC, ...) FUNC

#define LogAndCreateTizenError(...) \
  LogAndCreateTizenError_X(, ##__VA_ARGS__, \
     LogAndCreateTizenError_3(__VA_ARGS__), \
     LogAndCreateTizenError_2(__VA_ARGS__), \
     LogAndCreateTizenError_1(__VA_ARGS__) \
  )

// internal macros
#define LogAndReturnTizenError_1(error) \
  do { \
    LoggerE("Reporting error."); \
    return error; \
  } while(false)

#define LogAndReturnTizenError_2(error, log) \
  do { \
    LoggerE log; \
    return error; \
  } while(false)

#define LogAndReturnTizenError_X(_0, _1, _2, FUNC, ...) FUNC

#define LogAndReturnTizenError(...) \
  LogAndReturnTizenError_X(, ##__VA_ARGS__, \
     LogAndReturnTizenError_2(__VA_ARGS__), \
     LogAndReturnTizenError_1(__VA_ARGS__) \
  )

namespace common {

// defined here, so LoggerD() depends on TIZEN_DEBUG_ENABLE flag on per-module
// basis
class ScopeLogger {
 public:
  ScopeLogger(const std::string& f, const std::string& m, int l, const std::string& ex = "")
      : file_(f), method_(m), extra_(ex) {
    LoggerD("%s: %s(%d) > Enter %s", file_.c_str(), method_.c_str(), l, extra_.c_str());
  }

  ~ScopeLogger() {
    LoggerD("%s: %s > Exit %s", file_.c_str(), method_.c_str(), extra_.c_str());
  }

 private:
  std::string file_;
  std::string method_;
  std::string extra_;
};

}  // common

#define ScopeLogger(EX) const common::ScopeLogger __sl__{__MODULE__, __func__, __LINE__, EX}

#endif // COMMON_LOGGER_H_
