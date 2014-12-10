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
 * @file        MessagingUtil.cpp
 */

#include <PlatformException.h>
#include <string>
#include "MessagingUtil.h"
#include <Logger.h>
#include <fstream>
#include <streambuf>

using namespace std;
using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

namespace {
const string TYPE_SMS = "messaging.sms";
const string TYPE_MMS = "messaging.mms";
const string TYPE_EMAIL = "messaging.email";
const string SENT = "SENT";
const string SENDING = "SENDING";
const string FAILED = "FAILED";
const string DRAFT = "DRAFT";

const string FOLDER_TYPE_INBOX = "INBOX";
const string FOLDER_TYPE_OUTBOX = "OUTBOX";
const string FOLDER_TYPE_DRAFTS = "DRAFTS";
const string FOLDER_TYPE_SENTBOX = "SENTBOX";
}

string MessagingUtil::messageFolderTypeToString(MessageFolderType type)
{
    switch(type) {
        case MessageFolderType::MESSAGE_FOLDER_TYPE_INBOX:
            return FOLDER_TYPE_INBOX;
        case MessageFolderType::MESSAGE_FOLDER_TYPE_OUTBOX:
            return FOLDER_TYPE_OUTBOX;
        case MessageFolderType::MESSAGE_FOLDER_TYPE_DRAFTS:
            return FOLDER_TYPE_DRAFTS;
        case MessageFolderType::MESSAGE_FOLDER_TYPE_SENTBOX:
            return FOLDER_TYPE_SENTBOX;
        default:
            return "";
    }
}

MessageType MessagingUtil::stringToMessageType(string type)
{
    if (TYPE_SMS == type) {
        return MessageType(SMS);
    }
    if (TYPE_MMS == type) {
        return MessageType(MMS);
    }
    if (TYPE_EMAIL == type) {
        return MessageType(EMAIL);
    }
    std::string exceptionMsg = "Not supported type: ";
    exceptionMsg += type;
    throw TypeMismatchException(exceptionMsg.c_str());
}

string MessagingUtil::messageTypeToString(MessageType type)
{
    if (type == MessageType(SMS)) {
        return TYPE_SMS;
    }
    if (type == MessageType(MMS)) {
        return TYPE_MMS;
    }
    if (type == MessageType(EMAIL)) {
        return TYPE_EMAIL;
    }
    throw TypeMismatchException("Invalid MessageType");
}

MessageStatus MessagingUtil::stringToMessageStatus(std::string status)
{
    LOGD("Converting string %s to MessageStatus.", status.c_str());
    if(status == SENT) {
        return STATUS_SENT;
    }
    if(status == SENDING) {
        return STATUS_SENDING;
    }
    if(status == FAILED) {
        return STATUS_FAILED;
    }
    if(status == DRAFT) {
        return STATUS_DRAFT;
    }
    LOGE("Invalid MessageStatus");
    throw TypeMismatchException("Invalid MessageStatus");
}

std::string MessagingUtil::messageStatusToString(MessageStatus status) {
    LOGD("Converting MessageStatus %d to string.", (int)status);
    switch(status) {
        case STATUS_SENT:
            return SENT;
        case STATUS_SENDING:
            return SENDING;
        case STATUS_FAILED:
            return FAILED;
        case STATUS_DRAFT:
            return DRAFT;
        default:
        // According to Web API documentation: If the status of the current
        // message does not correspond to any item from the list, an empty
        // value is returned.
            LOGD("Unsupported or undefined MessageStatus");
            return "";
    }
}

std::string MessagingUtil::ltrim(const std::string& input)
{
    std::string str = input;
    std::string::iterator i;
    for (i = str.begin(); i != str.end(); i++) {
        if (!isspace(*i)) {
            break;
        }
    }
    if (i == str.end()) {
        str.clear();
    } else {
        str.erase(str.begin(), i);
    }
    return str;
}

std::string MessagingUtil::extractSingleEmailAddress(const std::string& address)
{
    std::size_t found_begin = address.rfind('<');
    std::size_t found_end = address.rfind('>');
    // if both '<' and '>' bracket found and '<' is before '>'
    // then extract email address from the inside
    if(found_begin != std::string::npos &&
            found_end != std::string::npos &&
            found_begin < found_end) {
        return address.substr(found_begin+1, found_end-found_begin-1);
    }
    else {
        // return unmodified source string
        return address;
    }
}

std::vector<std::string> MessagingUtil::extractEmailAddresses(
        const std::vector<std::string>& addresses)
{
    std::vector<std::string> extractedAddresses;
    for(auto it = addresses.begin(); it != addresses.end(); ++it) {
        extractedAddresses.push_back(MessagingUtil::extractSingleEmailAddress(*it));
    }

    return extractedAddresses;
}

std::string MessagingUtil::loadFileContentToString(const std::string& file_path)
{
    std::ifstream input_file;
    input_file.open(file_path, std::ios::in);

    if (input_file.is_open()) {
        std::string outString;
        input_file.seekg(0, std::ios::end);
        outString.reserve(input_file.tellg());
        input_file.seekg(0, std::ios::beg);

        outString.assign((std::istreambuf_iterator<char>(input_file)),
                std::istreambuf_iterator<char>());
        input_file.close();
        return outString;
    } else {
        std::stringstream ss_error_msg;
        ss_error_msg << "Failed to open file: " << file_path;
        throw Common::IOException(ss_error_msg.str().c_str());
    }
}

}
}
