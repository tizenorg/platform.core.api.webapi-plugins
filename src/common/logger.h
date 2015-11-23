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

#endif // COMMON_LOGGER_H_
