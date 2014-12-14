// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGING_MESSAGING_UTIL_H_
#define MESSAGING_MESSAGING_UTIL_H_

#include <string>
#include <vector>

namespace extension {
namespace messaging {

extern const char* JSON_CMD;
extern const char* JSON_ACTION;
extern const char* JSON_CALLBACK_ID;
extern const char* JSON_CALLBACK_SUCCCESS;
extern const char* JSON_CALLBACK_ERROR;
extern const char* JSON_CALLBACK_PROGRESS;
extern const char* JSON_CALLBACK_KEEP;
extern const char* JSON_DATA;

enum MessageType {
    UNDEFINED = 0,
    SMS,
    MMS,
    EMAIL
};

enum MessageStatus {
    STATUS_UNDEFINED = 0,
    STATUS_DRAFT,
    STATUS_SENDING,
    STATUS_SENT,
    STATUS_LOADED,
    STATUS_FAILED
};

class MessagingUtil {
public:
    static MessageType stringToMessageType(std::string);
    static std::string messageTypeToString(MessageType);
    static std::string ltrim(const std::string& input);
    static std::string extractSingleEmailAddress(const std::string& address);
    static std::vector<std::string> extractEmailAddresses(
            const std::vector<std::string>& addresses);
};

} // messaging
} // extension
#endif // MESSAGING_MESSAGING_UTIL_H_
