//
// Tizen Web Device API
// Copyright (c) 2013 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @file        MessagingUtil.h
 */

#ifndef __TIZEN_MESSAGING_UTIL_H__
#define __TIZEN_MESSAGING_UTIL_H__
#include <string>
#include <Logger.h>
#include <PlatformException.h>
#include "MessageFolder.h"

namespace DeviceAPI {
namespace Messaging {

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
    static std::string messageFolderTypeToString(MessageFolderType);

    static MessageType stringToMessageType(std::string);
    static std::string messageTypeToString(MessageType);

    static MessageStatus stringToMessageStatus(std::string status);
    static std::string messageStatusToString(MessageStatus status);

    static std::string ltrim(const std::string& input);
    static std::string extractSingleEmailAddress(const std::string& address);
    static std::vector<std::string> extractEmailAddresses(
            const std::vector<std::string>& addresses);

    /**
    * Throws Common::IOException when file cannot be opened.
    *
    * To increase performance invoke this function this way:
    * std::string result = loadFileContentToString(...);
    * Reason: no copy constructor will be invoked on return.
    */
    static std::string loadFileContentToString(const std::string& file_path);

    /**
     * Function converts vector of shared_pointers into JSArray
     */
    template<class T, class U>
    static JSObjectRef vectorToJSObjectArray(JSContextRef context,
            const std::vector<T> & vec)
    {
        size_t count = vec.size();

        JSObjectRef array[count];
        for (size_t i = 0; i < count; ++i) {
            array[i] = U::makeJSObject(context, vec[i]);
        }
        JSObjectRef result = JSObjectMakeArray(context, count,
                count > 0 ? array : NULL, NULL);
        if (!result) {
            LOGW("Failed to create array");
            throw Common::UnknownException("Failed to create array");
        }
        return result;
    }

};

}
}
#endif // __TIZEN_MESSAGING_UTIL_H__
