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

#ifndef MESSAGING_MESSAGE_ATTACHMENT_H_
#define MESSAGING_MESSAGE_ATTACHMENT_H_

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <msg_types.h>
#include <email-types.h>

namespace extension {
namespace messaging {

class MessageAttachment;

struct MessageAttachmentHolder {
    std::shared_ptr<MessageAttachment> ptr;
};

typedef std::vector<std::shared_ptr<MessageAttachment>> AttachmentPtrVector;

class MessageAttachment {
private:
    static std::map<std::string,unsigned int>& MIMETypeStringToEnumMap;
    static std::map<unsigned int, std::string>& MIMETypeEnumToStringMap;
    static std::map<std::string,unsigned int>& initializeMIMETypeStringToEnumMap();
    static std::map<unsigned int, std::string>& initializeMIMETypeEnumToStringMap();
    int m_id;
    bool m_isIdSet;
    int m_messageId;
    bool m_isMessageIdSet;
    std::string m_mimeType;
    bool m_isMimeTypeSet;
    std::string m_filePath;
    bool m_isFilePathSet;
    bool m_isSaved;
public:
    MessageAttachment();
    ~MessageAttachment();

    int getId();
    void setId(int value);
    bool isIdSet();
    void unsetId();
    int getMessageId();
    void setMessageId(int value);
    bool isMessageIdSet();
    void unsetMessageId();
    std::string getMimeType();
    void setMimeType(const std::string &value);
    bool isMimeTypeSet();
    void unsetMimeType();
    std::string getFilePath();
    std::string getShortFileName() const;
    void setFilePath(const std::string &value);
    bool isFilePathSet();
    void unsetFilePath();
    void setIsSaved(const bool isSaved);
    bool isSaved() const;
    static unsigned int MIMETypeStringToEnum(std::string str);
    static std::string MIMETypeEnumToString(unsigned int num);

    /**
     * This methods updates:
     *      setId(attachment_data.attachment_id);
     *      setMessageId(attachment_data.mail_id);
     *      setMimeType(attachment_data.attachment_mime_type);
     *      setFilePath(attachment_data.attachment_path);
     */
    void updateWithAttachmentData(const email_attachment_data_t& attachment_data);
};

} // messaging
} // extension

#endif // MESSAGING_MESSAGE_ATTACHMENT_H_
