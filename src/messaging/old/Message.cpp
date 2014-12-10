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
 * @file        Message.cpp
 */

#include "Message.h"
#include "MessageSMS.h"
#include "MessageMMS.h"
#include "MessageEmail.h"
#include "MessagingUtil.h"
#include "FilesystemExternalUtils.h"
#include "FilesystemUtils.h"
#include "Ecore_File.h"
#include "JSMessage.h"

#include <PlatformException.h>
#include <Logger.h>
#include <time.h>
#include <sys/stat.h>
#include <sstream>

namespace DeviceAPI {

using namespace Tizen;

namespace Messaging {

// *** constructor
Message::Message():
    m_id(-1), m_id_set(false), m_conversation_id(-1),
    m_conversation_id_set(false), m_folder_id(-1), m_folder_id_set(false),
    m_type(UNDEFINED), m_timestamp_set(false), m_from_set(false),
    m_body(new(std::nothrow) MessageBody()),
    m_service_id(0), m_is_read(false), m_has_attachment(false),
    m_high_priority(false), m_in_response(-1), m_in_response_set(false),
    m_service_id_set(false), m_status(STATUS_UNDEFINED),
    m_sim_index(TAPI_NETWORK_DEFAULT_DATA_SUBS_UNKNOWN)
{
    LOGD("Message constructor (%p)", this);
}

Message::~Message()
{
    LOGD("Message destructor (%p)", this);
}

// *** attribute getters
int Message::getId() const
{
    return m_id;
}

int Message::getConversationId() const
{
    return m_conversation_id;
}

int Message::getFolderId() const
{
    // TODO: folderId is supported different way in SMS/MMS and email
    return m_folder_id;
}

MessageType Message::getType() const
{
    return m_type;
}

time_t Message::getTimestamp() const
{
    return m_timestamp;
}

std::string Message::getFrom() const
{
    return m_from;
}

std::vector<std::string> Message::getTO() const
{
    return m_to;
}

JSObjectRef Message::getJSTO(JSContextRef global_ctx)
{
    return m_to.getJSArray(global_ctx);
}

std::vector<std::string> Message::getCC() const
{
    return m_cc;
}

JSObjectRef Message::getJSCC(JSContextRef global_ctx)
{
    return m_cc.getJSArray(global_ctx);
}

std::vector<std::string> Message::getBCC() const
{
    return m_bcc;
}

JSObjectRef Message::getJSBCC(JSContextRef global_ctx)
{
    return m_bcc.getJSArray(global_ctx);
}

std::shared_ptr<MessageBody> Message::getBody() const
{
    return m_body;
}

bool Message::getIsRead() const
{
    return m_is_read;
}

bool Message::getHasAttachment() const
{
    // This function should be reimplemented for MMS and email
    return m_has_attachment;
}

bool Message::getIsHighPriority() const
{
    return m_high_priority;
}

std::string Message::getSubject() const
{
    return m_subject;
}

int Message::getInResponseTo() const
{
    return m_in_response;
}

MessageStatus Message::getMessageStatus() const
{
    return m_status;
}

AttachmentPtrVector Message::getMessageAttachments() const
{
    return m_attachments;
}

JSObjectRef Message::getJSMessageAttachments(JSContextRef global_ctx)
{
    return m_attachments.getJSArray(global_ctx);
}

int Message::getServiceId() const
{
    return m_service_id;
}

TelNetworkDefaultDataSubs_t Message::getSimIndex() const
{
    return m_sim_index;
}

// *** attributes setters
void Message::setId(int id)
{
    m_id = id;
    m_id_set = true;
    m_body->setMessageId(m_id);
}

void Message::setConversationId(int id)
{
    m_conversation_id = id;
    m_conversation_id_set = true;
}

void Message::setFolderId(int id)
{
    m_folder_id = id;
    m_folder_id_set = true;
}

// type setting not allowed - no setter for type

void Message::setTimeStamp(time_t timestamp)
{
    m_timestamp = timestamp;
    m_timestamp_set = true;
}

void Message::setFrom(std::string from)
{
    m_from = from;
    m_from_set = true;
}

void Message::setTO(std::vector<std::string> &to)
{
    // Recipient's format validation should be done by Core API service
    m_to = to;

    if(m_to.empty()) {
        LOGD("Recipient's list cleared");
        return;
    }
}

void Message::setCC(std::vector<std::string> &cc)
{
    // implementation (address/number format checking) is message specific
}

void Message::setBCC(std::vector<std::string> &bcc)
{
    // implementation (address/number format checking) is message specific
}

void Message::setBody(std::shared_ptr<MessageBody>& body)
{
    // while replacing message body old body should have some invalid id mark
    m_body->setMessageId(-1);
    m_body = body;
    if(m_id_set) {
        m_body->setMessageId(m_id);
    }
}

void Message::setIsRead(bool read)
{
    m_is_read = read;
}

// has attachment can't be set explicity -> no setter for this flag

void Message::setIsHighPriority(bool highpriority)
{
    // High priority field is used only in MessageEmail
    m_high_priority = highpriority;
}

void Message::setSubject(std::string subject)
{
    // Subject is used only in MessageEmail and MessageMMS
}

void Message::setInResponseTo(int inresp)
{
    m_in_response = inresp;
    m_in_response_set = true;
}

void Message::setMessageStatus(MessageStatus status)
{
    m_status = status;
}

void Message::setMessageAttachments(AttachmentPtrVector &attachments)
{
    // implementation provided only for MMS and email
}


void Message::setServiceId(int service_id)
{
    m_service_id = service_id;
    m_service_id_set = true;
}

void Message::setSimIndex(TelNetworkDefaultDataSubs_t sim_index)
{
    m_sim_index = sim_index;
}

// ***  support for optional, nullable (at JS layer) attibutes
bool Message::is_id_set() const
{
    return m_id_set;
}

bool Message::is_conversation_id_set() const
{
    return m_conversation_id_set;
}

bool Message::is_folder_id_set() const
{
    return m_folder_id_set;
}

bool Message::is_timestamp_set() const
{
    return m_timestamp_set;
}

bool Message::is_from_set() const
{
    return m_from_set;
}

bool Message::is_in_response_set() const
{
    return m_in_response_set;
}

bool Message::is_service_is_set() const
{
    return m_service_id_set;
}

std::string Message::convertEmailRecipients(const std::vector<std::string> &recipients)
{
    std::string address = "";
    unsigned size = recipients.size();
    for (unsigned i=0; i<size; ++i)
    {
        address += "<"+recipients[i]+">; ";
    }

    return address;
}

std::string saveToTempFile(const std::string &data)
{
    char buf[] = "XXXXXX";
    int res = 0;

    mode_t mask = umask(S_IWGRP | S_IWOTH);
    res = mkstemp(buf);   //Just generate unique name

    std::string fileName = std::string("/tmp/") + buf;

    mode_t old_mask = umask(mask);
    FILE *file = fopen(fileName.c_str(), "w");
    umask(old_mask);

    if (NULL == file) {
        LOGE("Failed to create file");
        throw Common::UnknownException("Failed to create file");
    }
    if (fprintf(file, "%s", data.c_str()) < 0) {
        LOGE("Failed to write data into file");
        throw Common::UnknownException("Failed to write data into file");
    }
    fflush(file);
    fclose(file);
    return fileName;
}

std::string copyFileToTemp(const std::string& sourcePath)
{
    LOGD("Entered");
    char buf[] = "XXXXXX";
    std::string dirPath, fileName, attPath, tmpPath;

    mode_t mask = umask(S_IWGRP | S_IWOTH);
    int err = mkstemp(buf);
    if (-1 == err) {
        LOGW("Failed to create unique filename");
    }

    umask(mask);
    dirPath = "/tmp/" + std::string(buf);

    if ( sourcePath[0] != '/' ) {
        attPath = Filesystem::External::fromVirtualPath(sourcePath);
    } else { // Assuming that the path is a real path
        attPath = sourcePath;
    }

    // Looking for the last occurrence of slash in source path
    std::size_t slashPos;
    if ((slashPos = attPath.find_last_of('/')) == std::string::npos) {
        throw Common::UnknownException(
                "Error while copying file to temp: the source path is invalid.");
    }

    fileName = attPath.substr(slashPos + 1);
    tmpPath = dirPath + "/" + fileName;

    LOGD("attPath: %s, tmpPath: %s", attPath.c_str(), tmpPath.c_str());
    if(EINA_TRUE != ecore_file_mkdir(dirPath.c_str())) {
        throw Common::UnknownException("Unknown error while creating temp directory.");
    }

    if(EINA_TRUE != ecore_file_cp(attPath.c_str(), tmpPath.c_str())) {
        throw Common::UnknownException("Unknown error while copying file to temp.");
    }

    return dirPath;
}

void removeDirFromTemp(const std::string& dirPath)
{
    if(EINA_TRUE != ecore_file_rmdir(dirPath.c_str())) {
        throw Common::UnknownException("Unknown error while deleting temp directory.");
    }
}

email_mail_data_t* Message::convertPlatformEmail(std::shared_ptr<Message> message)
{
    if(EMAIL != message->getType()) {
        LOGE("Invalid type");
        throw Common::InvalidValuesException("Invalid type.");
    }

    email_mail_data_t *mail_data = NULL;

    if(message->is_id_set()) {
        email_get_mail_data(message->getId(), &mail_data);
    } else {
        mail_data = (email_mail_data_t*)malloc(
                sizeof(email_mail_data_t));
        memset(mail_data, 0x00, sizeof(email_mail_data_t));
    }

    if(!message->getFrom().empty()) {
        std::string from = "<"+message->getFrom()+">";
        mail_data->full_address_from = strdup(from.c_str());
    }

    if(!message->getTO().empty()) {
        std::string to = Message::convertEmailRecipients(message->getTO());
        mail_data->full_address_to = strdup(to.c_str());
    }

    if(!message->getCC().empty()) {
        std::string cc = Message::convertEmailRecipients(message->getCC());
        mail_data->full_address_cc = strdup(cc.c_str());
    }

    if(!message->getBCC().empty()) {
        std::string bcc = Message::convertEmailRecipients(message->getBCC());
        mail_data->full_address_bcc = strdup(bcc.c_str());
    }

    if(!message->getSubject().empty()) {
        std::string subject = message->getSubject();
        mail_data->subject = strdup(subject.c_str());
    }

    if(message->getBody()) {
        std::shared_ptr<MessageBody> body;
        body = message->getBody();
        if(!body->getPlainBody().empty()) {
            std::string body_file_path = saveToTempFile(body->getPlainBody());
            mail_data->file_path_plain = strdup(body_file_path.c_str());
            if(!mail_data->file_path_plain)
            {
                LOGE("Plain Body file is NULL.");
                free(mail_data);
                mail_data = NULL;
                throw Common::UnknownException("Plain Body file is NULL.");
            }
        }

        if(!body->getHtmlBody().empty()) {
            std::string html_file_path = saveToTempFile(body->getHtmlBody());
            mail_data->file_path_html = strdup(html_file_path.c_str());
            if(!mail_data->file_path_html)
            {
                LOGE("Html Body file is NULL.");
                free(mail_data);
                mail_data = NULL;
                throw Common::UnknownException("Html Body file is NULL.");
            }
        } else if(!body->getPlainBody().empty()) {
            // check html data is exist if not exist copy plain body to html body
            std::string html_file_path = saveToTempFile(body->getPlainBody());
            mail_data->file_path_html = strdup(html_file_path.c_str());
            if(!mail_data->file_path_html)
            {
                LOGE("Plain Body file is NULL.");
                free(mail_data);
                mail_data = NULL;
                throw Common::UnknownException("Plain Body file is NULL.");
            }
        }
    }

    mail_data->flags_seen_field = message->getIsRead()?1:0;

    if(message->getIsHighPriority()) {
        mail_data->priority = EMAIL_MAIL_PRIORITY_HIGH;
    } else {
        mail_data->priority = EMAIL_MAIL_PRIORITY_NORMAL;
    }

    return mail_data;
}

void addSingleEmailAttachment(std::shared_ptr<Message> message,
        std::shared_ptr<MessageAttachment> att, AttachmentType attType)
{
    std::string dirPath = copyFileToTemp(att->getFilePath());

    email_attachment_data_t* tmp = new email_attachment_data_t();
    tmp->attachment_name = strdup(att->getShortFileName().c_str());
    tmp->attachment_path  = strdup(std::string(dirPath + "/"
            + att->getShortFileName()).c_str());
    tmp->save_status = 1;
    tmp->inline_content_status = attType;

    int id = message->getId();
    int err = email_add_attachment(id, tmp);
    if(EMAIL_ERROR_NONE != err) {
        LOGE("Error while adding attachment %d", err);
        err = email_free_attachment_data(&tmp, 1);
        if (EMAIL_ERROR_NONE != err) {
            LOGW("Failed to free attachment data");
        }
        throw Common::UnknownException("Unknown error while adding attachment");
    }

    att->setId(tmp->attachment_id);
    att->setMessageId(id);
    err = email_free_attachment_data(&tmp, 1);
    if (EMAIL_ERROR_NONE != err) {
        LOGW("Failed to free attachment data");
    }

    removeDirFromTemp(dirPath);
}

void Message::addEmailAttachments(std::shared_ptr<Message> message)
{
    LOGD("Entered");

    int attachment_data_count = 0, error;
    email_mail_data_t *mail = NULL;
    email_attachment_data_t *attachment_data_list = NULL;
    email_meeting_request_t *meeting_req = NULL;


    AttachmentPtrVector attachments = message->getMessageAttachments();
    AttachmentPtrVector inlineAttachments = message->getBody()->getInlineAttachments();
    LOGD("Attachments size: %d", attachments.size());
    LOGD("Inline attachments size: %d", inlineAttachments.size());
    LOGD("Adding attachments for mail id = [%d]\n", message->getId());
    for (auto it = attachments.begin(); it != attachments.end(); ++it) {
        addSingleEmailAttachment(message, *it, AttachmentType::EXTERNAL);
    }
    for (auto it = inlineAttachments.begin(); it != inlineAttachments.end(); ++it) {
        addSingleEmailAttachment(message, *it, AttachmentType::INLINE);
    }

    //Update of mail on server using function email_update_mail() is not possible.
    //Attachment is updated only locally, so there is need to use workaround:
    //1. add new mail with null attachments list
    //2. add attachments to mail (locally)
    //3. create new email with attachments and add it to server
    //4. delete mail without attachments

    //getting mail and attachments data
    mail = Message::convertPlatformEmail(message);
    error = email_get_attachment_data_list(mail->mail_id, &attachment_data_list, &attachment_data_count);
    if (EMAIL_ERROR_NONE != error) {
        email_free_mail_data(&mail, 1);
        email_free_attachment_data(&attachment_data_list,attachment_data_count);
        LOGE("Error while adding attachments. Failed to get attachment list.");
        throw Common::UnknownException("Error while adding attachments. Failed to get attachment list.");
    }

    //save mail without attachments id
    int tmp_id = mail->mail_id;

    //adding new mail with attachments
    error = email_add_mail(mail, attachment_data_list, attachment_data_count, meeting_req, 0);
    if (EMAIL_ERROR_NONE != error) {
        email_free_mail_data(&mail, 1);
        email_free_attachment_data(&attachment_data_list,attachment_data_count);
        LOGE("Error while re-adding mail: %d", error);
        throw Common::UnknownException("Error while re-adding mail");
    }
    LOGD("mail added - new id = [%d]\n", mail->mail_id);

    //refresh message object
    message->setId(mail->mail_id);
    message->setMessageStatus(MessageStatus::STATUS_DRAFT);
    for (auto it = attachments.begin(); it != attachments.end(); ++it) {
        (*it)->setMessageId(mail->mail_id);
    }
    for (auto it = inlineAttachments.begin(); it != inlineAttachments.end(); ++it) {
        (*it)->setMessageId(mail->mail_id);
    }
    email_free_attachment_data(&attachment_data_list,attachment_data_count);

    //deleting mail without attachments
    error = email_delete_mail(mail->mailbox_id,&tmp_id,1,1);
    if (EMAIL_ERROR_NONE != error) {
        email_free_mail_data(&mail, 1);
        LOGE("Error while deleting mail from server: %d", error);
        throw Common::UnknownException("Error while deleting mail from server");
    }
    email_free_mail_data(&mail, 1);
}

void Message::addSMSRecipientsToStruct(const std::vector<std::string> &recipients,
        msg_struct_t &msg)
{
    const unsigned size = recipients.size();
    for (unsigned int i = 0; i < size; ++i) {
        char *address = const_cast<char *>(recipients.at(i).c_str());

        LOGD("[%d] address:[%s]", i, address);
        msg_struct_t tmpAddr = NULL;
        if (MSG_SUCCESS
                == msg_list_add_item(msg, MSG_MESSAGE_ADDR_LIST_HND, &tmpAddr)) {
            msg_set_int_value(tmpAddr, MSG_ADDRESS_INFO_ADDRESS_TYPE_INT,
                    MSG_ADDRESS_TYPE_PLMN);
            msg_set_int_value(tmpAddr, MSG_ADDRESS_INFO_RECIPIENT_TYPE_INT,
                    MSG_RECIPIENTS_TYPE_TO);
            msg_set_str_value(tmpAddr, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR,
                    address, strlen(address));
        }
        else {
            LOGE("failed to add address[%d] %s", i, address);
            throw Common::UnknownException("failed to add address");
        }
    }
}

void Message::addMMSRecipientsToStruct(const std::vector<std::string> &recipients,
        msg_struct_t &msg, int type)
{
    const unsigned size = recipients.size();
    for (unsigned int i = 0; i < size; ++i) {

        msg_struct_t tmpAddr = NULL;
        int address_type = MSG_ADDRESS_TYPE_PLMN;
        const std::size_t found = recipients[i].find("@");
        if (std::string::npos != found) {
            address_type = MSG_ADDRESS_TYPE_EMAIL;
        }

        char *address = const_cast<char *>(recipients.at(i).c_str());
        LOGD("[%d] address:[%s] address_type:%d type:%d", i, address, address_type, type);

        int error = msg_list_add_item(msg, MSG_MESSAGE_ADDR_LIST_HND, &tmpAddr);
        if (MSG_SUCCESS == error) {
            msg_set_int_value(tmpAddr, MSG_ADDRESS_INFO_ADDRESS_TYPE_INT,
                    address_type);
            msg_set_int_value(tmpAddr, MSG_ADDRESS_INFO_RECIPIENT_TYPE_INT,
                    type);
            msg_set_str_value(tmpAddr, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR,
                    address, strlen(address));
        }
        else {
            LOGE("[%d] failed to add address: [%s], error: %d", i, address, error);
            throw Common::UnknownException("failed to add address");
        }
    }
}

void Message::addMMSBodyAndAttachmentsToStruct(const AttachmentPtrVector attach,
        msg_struct_t &mms_struct, Message* message)
{
    LOGD("Entered with %d attachments", attach.size());

    int size = attach.size();
    for (int i = 0; i < size; i++) {

        msg_struct_t tmpAtt = NULL;
        int error = msg_list_add_item(mms_struct, MSG_STRUCT_MMS_ATTACH, &tmpAtt);
        if (MSG_SUCCESS == error) {

            //Ensure we have right id set
            attach[i]->setId(i+1);
            attach[i]->setMessageId(message->getId());

            //-------------------------------------------------------------------------
            // set file path, file name, file size
            if (attach.at(i)->isFilePathSet()) {
                std::string filepath = attach.at(i)->getFilePath();
                LOGD("att[%d]: org filepath: %s", i, filepath.c_str());
                if(Filesystem::External::isVirtualPath(filepath)) {
                    // TODO
                    // When introducing below line fromVirtualPath() function
                    // needed context, but never used it - allowing for null
                    // context pointer. If it appears to need a real context
                    // it will need a fix here.
                    filepath = Filesystem::External::fromVirtualPath(filepath);
                    LOGD("att[%d]: org virtual filepath: %s", i, filepath.c_str());
                }
                msg_set_str_value(tmpAtt, MSG_MMS_ATTACH_FILEPATH_STR,
                        const_cast<char*>(filepath.c_str()), filepath.size());
                const size_t last_slash_idx = filepath.find_last_of("\\/");
                if (std::string::npos != last_slash_idx) {
                    filepath.erase(0, last_slash_idx + 1);
                }

                LOGD("att[%d] filename: %s", i, filepath.c_str());
                msg_set_str_value(tmpAtt, MSG_MMS_ATTACH_FILENAME_STR,
                        const_cast<char*>(filepath.c_str()), filepath.size());
                struct stat st;
                if (stat(const_cast<char*>(filepath.c_str()), &st)) {
                    LOGE("Stat error");
                }
                const int fsize = st.st_size;
                msg_set_int_value(tmpAtt, MSG_MMS_ATTACH_FILESIZE_INT, fsize);
                LOGD("att[%d]: filesize: %d", i,fsize);
            }

            //-------------------------------------------------------------------------
            //set mime type
            if (attach.at(i)->isMimeTypeSet()) {
                unsigned int type = MessageAttachment::MIMETypeStringToEnum(
                        attach.at(i)->getMimeType());
                msg_set_int_value(tmpAtt, MSG_MMS_ATTACH_MIME_TYPE_INT, type);
                msg_set_str_value(tmpAtt, MSG_MMS_ATTACH_CONTENT_TYPE_STR,
                                    const_cast<char*>(attach.at(i)->getMimeType().c_str()),
                                    MSG_MSG_ID_LEN);


                LOGD("att[%d]: setting mime type:0x%x (orignal:%s)", i, type,
                    attach.at(i)->getMimeType().c_str());
            }
        } else {
            LOGE("att[%d]: failed to add attachment");
            throw Common::UnknownException("failed to add attachment");
        }
    }
}

msg_struct_t Message::convertPlatformShortMessageToStruct(Message* message,
        msg_handle_t handle)
{
    LOGD("Entered");

    if (message->getType() != SMS && message->getType() != MMS) {
        LOGD("Invalid type");
        throw Common::InvalidValuesException("Invalid type");
    }

    msg_error_t err = MSG_SUCCESS;
    msg_struct_t sendOpt = msg_create_struct(MSG_STRUCT_SENDOPT);
    msg_struct_t msg = msg_create_struct(MSG_STRUCT_MESSAGE_INFO);

    if (message->is_id_set()) { // id is set - the message exists in database
        msg_message_id_t id = (msg_message_id_t) message->getId();
        err = msg_get_message(handle, id, msg, sendOpt);
        if (err != MSG_SUCCESS) {
            msg_release_struct(&sendOpt);
            msg_release_struct(&msg);
            LOGD("msg_get_message() Fail [%d]", err);
            throw Common::UnknownException("msg_get_message() Fail");
        }
        LOGD("Using existing msg for id: %d", id);
    } else { // id is not set - the message does not exist in database
        MessageType msgType = message->getType();
        if (msgType == MessageType::SMS) {
            // Set message type to SMS
            if (MSG_SUCCESS
                    != msg_set_int_value(msg, MSG_MESSAGE_TYPE_INT, MSG_TYPE_SMS)) {
                msg_release_struct(&sendOpt);
                msg_release_struct(&msg);
                LOGE("Set SMS type error");
                throw Common::UnknownException("Set SMS type error");
            }
        } else {
            // Set message type to MMS
            if (MSG_SUCCESS
                    != msg_set_int_value(msg, MSG_MESSAGE_TYPE_INT, MSG_TYPE_MMS)) {
                msg_release_struct(&sendOpt);
                msg_release_struct(&msg);
                LOGE("Set MMS type error");
                throw Common::UnknownException("Set MMS type error");
            }
        }
    }
    msg_release_struct(&sendOpt);

    int type;
    msg_get_int_value(msg, MSG_MESSAGE_TYPE_INT, &type);
    LOGD("Message(%p): MSG_MESSAGE_TYPE = %d", message, type);

    if (type == MSG_TYPE_SMS) {
        // Set SMS message body text
        std::shared_ptr<MessageBody> body;
        body = message->getBody();
        if (!body->getPlainBody().empty()) {
            msg_set_str_value(msg, MSG_MESSAGE_SMS_DATA_STR, const_cast<char*>
                    (body->getPlainBody().c_str()), body->getPlainBody().size());
        }

        // Reset SMS recipients
        int error = msg_list_clear(msg, MSG_MESSAGE_ADDR_LIST_HND);
        if( MSG_SUCCESS != error) {
            LOGE("Failed to clear address list, error: %d", error);
            throw Common::UnknownException("Failed to clear address list");
        }

        // Set SMS recipients
        std::vector<std::string> recp_list = message->getTO();
        if (!recp_list.empty()) {
            message->addSMSRecipientsToStruct(recp_list, msg);
        }

    } else if (type == MSG_TYPE_MMS) {
        // Set message type to MMS
        if (MSG_SUCCESS
                != msg_set_int_value(msg, MSG_MESSAGE_TYPE_INT, MSG_TYPE_MMS)) {
            LOGE("Message(%p): Set MMS type error", message);
            throw Common::UnknownException("Set MMS type error");
        }
        // Create MMS data
        msg_struct_t mms_data = msg_create_struct(MSG_STRUCT_MMS);
        if (mms_data == NULL) {
            LOGE("Message(%p): Set MMS data error", message);
            throw Common::UnknownException("Set MMS data error");
        }
        // Set MMS message subject
        std::string subject = message->getSubject();
        if (subject != "") {
            int r = msg_set_str_value(msg, MSG_MESSAGE_SUBJECT_STR,
                    const_cast<char*>(subject.c_str()), subject.size());
            if (r != MSG_SUCCESS) {
                LOGE("Message(%p): Set MMS subject error: %d", message, r);
                throw Common::UnknownException("Set MMS subject error");
            }
        }
        // Set MMS message text
        std::shared_ptr<MessageBody> body;
        body = message->getBody();
        if (!body->getPlainBody().empty()) {
            LOGD("Message(%p): PlainBody is NOT empty", message);

            static const int ROOT_LAYOUT_WIDTH = 100;
            static const int ROOT_LAYOUT_HEIGHT = 100;
            static const int WHITE_COLOR = 0xffffff;
            static const int BLACK_COLOR = 0x000000;

            //----------------------------------------------------------------------------
            //Region
            msg_struct_t region;
            msg_list_add_item(mms_data, MSG_STRUCT_MMS_REGION, &region);
            msg_set_str_value(region, MSG_MMS_REGION_ID_STR, const_cast<char*>("Text"), 4);

            msg_set_int_value(region, MSG_MMS_REGION_LENGTH_LEFT_INT, 0);
            msg_set_int_value(region, MSG_MMS_REGION_LENGTH_TOP_INT, 0);
            msg_set_int_value(region, MSG_MMS_REGION_LENGTH_WIDTH_INT,
                    ROOT_LAYOUT_WIDTH);
            msg_set_int_value(region, MSG_MMS_REGION_LENGTH_HEIGHT_INT,
                    ROOT_LAYOUT_HEIGHT);
            msg_set_int_value(region, MSG_MMS_REGION_BGCOLOR_INT, WHITE_COLOR);

            msg_set_bool_value(region, MSG_MMS_REGION_LENGTH_LEFT_PERCENT_BOOL, true);
            msg_set_bool_value(region, MSG_MMS_REGION_LENGTH_TOP_PERCENT_BOOL, true);
            msg_set_bool_value(region, MSG_MMS_REGION_LENGTH_WIDTH_PERCENT_BOOL, true);
            msg_set_bool_value(region, MSG_MMS_REGION_LENGTH_HEIGHT_PERCENT_BOOL, true);

            //----------------------------------------------------------------------------
            //Page
            msg_struct_t page;
            msg_list_add_item(mms_data, MSG_STRUCT_MMS_PAGE, &page);
            msg_set_int_value(page, MSG_MMS_PAGE_PAGE_DURATION_INT, 0);

            //----------------------------------------------------------------------------
            //Media
            msg_struct_t media;
            msg_list_add_item(page, MSG_STRUCT_MMS_MEDIA, &media);
            msg_set_int_value(media, MSG_MMS_MEDIA_TYPE_INT, MMS_SMIL_MEDIA_TEXT);
            msg_set_str_value(media, MSG_MMS_MEDIA_REGION_ID_STR,
                    const_cast<char*>("Text"), 4);

            std::string body_file_path = saveToTempFile(body->getPlainBody());
            int error = msg_set_str_value(media,
                    MSG_MMS_MEDIA_FILEPATH_STR,
                    const_cast<char*>(body_file_path.c_str()),
                    body_file_path.size());
            if (error != MSG_SUCCESS) {
                LOGE("Message(%p): Failed to set mms body filepath", message);
                throw Common::UnknownException("Failed to set mms body filepath");
            }
            msg_set_str_value(media, MSG_MMS_MEDIA_CONTENT_TYPE_STR,
                "text/plain", 10);

            //----------------------------------------------------------------------------
            //Smile text
            msg_struct_t smil_text;
            msg_get_struct_handle(media, MSG_MMS_MEDIA_SMIL_TEXT_HND, &smil_text);
            msg_set_int_value(smil_text, MSG_MMS_SMIL_TEXT_COLOR_INT, BLACK_COLOR);
            msg_set_int_value(smil_text, MSG_MMS_SMIL_TEXT_SIZE_INT,
                    MMS_SMIL_FONT_SIZE_NORMAL);
            msg_set_bool_value(smil_text, MSG_MMS_SMIL_TEXT_BOLD_BOOL, true);
        } else {
            LOGD("Message(%p): PlainBody is EMPTY", message);
        }
        // Set MMS attachments
        AttachmentPtrVector attach_list = message->getMessageAttachments();
        LOGD("Message(%p): id:%d subject:[%s] plainBody:[%s] contains %d attachments",
                message, message->getId(), message->getSubject().c_str(),
                message->getBody()->getPlainBody().c_str(), attach_list.size());

        msg_set_int_value(mms_data, MSG_MESSAGE_ATTACH_COUNT_INT,
                attach_list.size());
        if (!attach_list.empty()) {
            addMMSBodyAndAttachmentsToStruct(attach_list, mms_data, message);
        }
        // Set MMS body
        int r = msg_set_mms_struct(msg, mms_data);
        if (r != MSG_SUCCESS) {
            LOGE("Message(%p): Set MMS body error: %d", message, r);
            throw Common::UnknownException("Set MMS body error");
        }
        msg_release_struct(&mms_data);

        // Reset MMS recipients
        msg_list_clear(msg, MSG_MESSAGE_ADDR_LIST_HND);

        std::vector<std::string> recp_list = message->getTO();
        message->addMMSRecipientsToStruct(recp_list, msg, MSG_RECIPIENTS_TYPE_TO);

        recp_list = message->getCC();
        message->addMMSRecipientsToStruct(recp_list, msg, MSG_RECIPIENTS_TYPE_CC);

        recp_list = message->getBCC();
        message->addMMSRecipientsToStruct(recp_list, msg, MSG_RECIPIENTS_TYPE_BCC);
    }
    else {
        msg_release_struct(&msg);
        LOGE("Message(%p): Invalid message type", message);
        throw Common::InvalidValuesException("Invalid message type");
    }

    // set common attributes for SMS and MMS
    // Set message conversation id
    if (message->is_conversation_id_set()) {
        msg_set_int_value(msg, MSG_MESSAGE_THREAD_ID_INT, message->getConversationId());
    }
    // Set message folder id
    if (message->is_folder_id_set()) {
        msg_set_int_value(msg, MSG_MESSAGE_FOLDER_ID_INT, message->getFolderId());
    }
    // Set message timestamp
    if (message->is_timestamp_set()) {
        msg_set_int_value(msg, MSG_MESSAGE_DISPLAY_TIME_INT, message->getTimestamp());
    }
    // Set message from
    if (message->is_from_set()) {
        msg_set_str_value(msg, MSG_MESSAGE_REPLY_ADDR_STR, const_cast<char*>
                (message->getFrom().c_str()), message->getFrom().size());
    }
    // Set message if is response
    if (message->is_in_response_set()) {
        msg_set_int_value(msg, MSG_MESSAGE_DIRECTION_INT, message->getInResponseTo());
    }

    // Set SIM index
    // -1 means unknown - so do not set simindex in that case.
    int sim_index = static_cast<int>(message->getSimIndex());
    if (sim_index != -1) {
        int error =
            msg_set_int_value(msg, MSG_MESSAGE_SIM_INDEX_INT, sim_index+1);
        if ( MSG_SUCCESS != error) {
            LOGE("Failed to set sim index, error: %d", error);
            throw Common::UnknownException("Failed to set sim index");
        }
    }

    // Set message if is read
    msg_set_bool_value(msg, MSG_MESSAGE_READ_BOOL, message->getIsRead());

    LOGD("End");
    return msg;
}

std::string Message::getShortMsgSenderFromStruct(msg_struct_t &msg)
{
    msg_list_handle_t addr_list = NULL;
    msg_get_list_handle(msg, MSG_MESSAGE_ADDR_LIST_HND, (void **)&addr_list);

    char str_phone_number[MAX_ADDRESS_VAL_LEN];
    const int count = msg_list_length(addr_list);
    LOGD("Number of addresses: %d", count);

    for (int i = 0; i < count; ++i)
    {
        int tempInt = 0;
        msg_get_int_value(msg, MSG_MESSAGE_DIRECTION_INT, &tempInt);
        const int type = tempInt;

        if (MSG_DIRECTION_TYPE_MT == type)
        {
            msg_struct_t cur_addr_info = (msg_struct_t) msg_list_nth_data(addr_list, i);
            msg_get_str_value(cur_addr_info, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR,
                    str_phone_number, MAX_ADDRESS_VAL_LEN);

            LOGD("[%d/%d] is TYPE_MT, phone number is: %s", i, count, str_phone_number);

            if(0 != str_phone_number[0]) {
                return std::string(str_phone_number);
            }
        } else {
            LOGD("[%d/%d] is NOT of TYPE_MT skipping, is:%d", i, count, type);
        }
    }

    return std::string();
}

std::vector<std::string> Message::getSMSRecipientsFromStruct(msg_struct_t &msg)
{
    std::vector<std::string> address;
    msg_list_handle_t addr_list = NULL;
    if (MSG_SUCCESS
            == msg_get_list_handle(msg, MSG_MESSAGE_ADDR_LIST_HND,
                    (void **) &addr_list)) {
        unsigned size = msg_list_length(addr_list);
        for (unsigned int i = 0; i < size; i++) {
            msg_struct_t addr_info = NULL;
            char infoStr[MAX_ADDRESS_VAL_LEN];
            //get address
            addr_info = (msg_struct_t) msg_list_nth_data(addr_list, i);
            msg_get_str_value(addr_info, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR,
                    infoStr, MAX_ADDRESS_VAL_LEN);
            address.push_back(std::string(infoStr));
        }
    } else {
        LOGE("failed to get recipients");
        throw Common::UnknownException("failed to add recipients");
    }
    return address;
}

std::vector<std::string> Message::getMMSRecipientsFromStruct(msg_struct_t &msg,
        int type)
{
    std::vector<std::string> address;
    msg_list_handle_t addr_list = NULL;
    if (MSG_SUCCESS
            == msg_get_list_handle(msg, MSG_MESSAGE_ADDR_LIST_HND,
                    (void **) &addr_list)) {
        unsigned size = msg_list_length(addr_list);
        for (unsigned int i = 0; i < size; i++) {
            msg_struct_t addr_info = NULL;
            char infoStr[MAX_ADDRESS_VAL_LEN];
            int tempInt;
            //get address
            addr_info = (msg_struct_t) msg_list_nth_data(addr_list, i);
            msg_get_int_value(addr_info, MSG_ADDRESS_INFO_RECIPIENT_TYPE_INT,
                    &tempInt);
            if (tempInt == type) {
                msg_get_str_value(addr_info, MSG_ADDRESS_INFO_ADDRESS_VALUE_STR,
                        infoStr, MAX_ADDRESS_VAL_LEN);
                address.push_back(std::string(infoStr));
            }
        }
    } else {
        LOGE("failed to get recipients");
        throw Common::UnknownException("failed to add recipients");
    }
    return address;
}

void Message::setMMSBodyAndAttachmentsFromStruct(Message* message,
        msg_struct_t &msg)
{
    LOGD("Entered message(%p)", message);
    int tempInt = 0;
    char infoStr[MSG_FILEPATH_LEN_MAX + 1];

    msg_struct_t mms_struct = msg_create_struct(MSG_STRUCT_MMS);
    int error = msg_get_mms_struct(msg, mms_struct);
    if (MSG_SUCCESS != error) {
        LOGE("Cannot get mms struct, error:%d", error);
        msg_release_struct(&mms_struct);
        throw Common::UnknownException("cannot get mms struct");
    }

    bool body_has_been_set = false;
    // if there are some pages in msg_struct_t
    msg_list_handle_t page_list = NULL;
    error = msg_get_list_handle(mms_struct, MSG_MMS_PAGE_LIST_HND, (void **) &page_list);
    if (MSG_SUCCESS == error) {
        int pageLen = msg_list_length(page_list);
        LOGD("MSG_MMS_PAGE_LIST length:%d", pageLen);

        for (int p = 0; p < pageLen; ++p) {
            msg_struct_t page = (msg_struct_t) msg_list_nth_data(page_list, p);
            if (!page) {
                LOGE("returned page is null, continue");
                continue;
            }

            msg_list_handle_t media_list = NULL;
            error = msg_get_list_handle(page, MSG_MMS_PAGE_MEDIA_LIST_HND,
                    (void **) &media_list);
            if (MSG_SUCCESS == error) {
                int mediaLen = msg_list_length(media_list);
                LOGD("[p:%d] MSG_MMS_PAGE_MEDIA_LIST length:%d", p, mediaLen);

                for (int m = 0; m < mediaLen; ++m) {
                    msg_struct_t media = (msg_struct_t) msg_list_nth_data(media_list, m);
                    if (NULL == media) {
                        LOGE("returned media is null, continue");
                        continue;
                    }
                    // add media from pages to attachments vector
                    //set file path
                    memset(infoStr, 0, MSG_FILEPATH_LEN_MAX + 1);
                    msg_get_str_value(media, MSG_MMS_MEDIA_FILEPATH_STR, infoStr,
                            MSG_FILEPATH_LEN_MAX);
                    LOGD("[p:%d, m:%d] attachment file path:%s", p, m, infoStr);

                    msg_get_int_value(media, MSG_MMS_MEDIA_TYPE_INT, &tempInt);
                    const int msg_media_type = tempInt;
                    std::string msg_media_type_str =
                            MessageAttachment::MIMETypeEnumToString(msg_media_type);

                    LOGD("[p:%d, m:%d] MSG_MMS_MEDIA_TYPE: %d (%s)", p, m, msg_media_type,
                            msg_media_type_str.c_str());

                    //According to old implementation
                    // "text value on first page goes to body attribute"
                    if ((0 == p) && (MMS_SMIL_MEDIA_TEXT == msg_media_type)) {
                        LOGD("Loading body from file: %s ", infoStr);

                        try {
                            message->getBody()->setPlainBody(
                                    MessagingUtil::loadFileContentToString(infoStr));
                            body_has_been_set = true;

                            LOGD("Loaded body: %s",
                                    message->getBody()->getPlainBody().c_str());

                        } catch(const Common::BasePlatformException& exception) {
                            LOGE("Unhandled exception: %s (%s)!",
                                 (exception.getName()).c_str(),
                                 (exception.getMessage()).c_str());
                            LOGD("[p:%d, m:%d] body is not set", p, m);
                        }
                        catch (...) {
                            LOGE("Unknown exception occured during plain body loading");
                            LOGD("[p:%d, m:%d] body is not set", p, m);
                        }

                    } else {
                        std::shared_ptr<MessageAttachment> ma (new MessageAttachment());
                        ma->setFilePath(infoStr);

                        //set message id
                        msg_get_int_value(msg, MSG_MESSAGE_STORAGE_ID_INT, &tempInt);
                        ma->setMessageId(tempInt);

                        //set id
                        ma->setId(message->m_attachments.size() + 1);
                        message->m_attachments.push_back(ma);
                        message->m_has_attachment = true;

                        //set mime type
                        ma->setMimeType(msg_media_type_str);

                        MessageAttachment* att = ma.get();
                        LOGD("[p:%d, m:%d] added attachment: %p "
                                "(mime:0x%x mime:%s messageId:%d)", p, m, att,
                                msg_media_type, msg_media_type_str.c_str(),
                                ma->getMessageId());
                    }

                    msg_release_struct(&media);
                }
            } else {
                msg_release_struct(&mms_struct);
                LOGE("failed to get attachment");
                throw Common::UnknownException("failed to get attachment");
            }
            msg_release_struct(&page);
        }
    } else {
        msg_release_struct(&mms_struct);
        LOGE("failed to get attachment");
        throw Common::UnknownException("failed to get attachment");
    }

    if(false == body_has_been_set) {
        LOGW("Warning: body has not been set!");
    }

    LOGD("after MSG_MMS_PAGE_LIST attachments count is:%d",
            message->m_attachments.size());

    // if there are some other attachments add it to attachments vector
    msg_list_handle_t attach_list = NULL;
    error = msg_get_list_handle(mms_struct, MSG_MMS_ATTACH_LIST_HND, (void **)
            &attach_list);
    if (MSG_SUCCESS == error) {

        unsigned size = msg_list_length(attach_list);
        LOGD("MSG_MMS_ATTACH_LIST length:%d", size);

        for (unsigned int i = 0; i < size; i++) {
            msg_struct_t attach_info = NULL;
            attach_info = (msg_struct_t) msg_list_nth_data(attach_list, i);
            if(!attach_info) {
                LOGW("[att:%d] attach_info is NULL!", i);
                continue;
            }

            std::shared_ptr<MessageAttachment> ma (new MessageAttachment());

            //set message id
            msg_get_int_value(msg, MSG_MESSAGE_ID_INT, &tempInt);
            ma->setMessageId(tempInt);

            //set file path
            msg_get_str_value(attach_info, MSG_MMS_ATTACH_FILEPATH_STR, infoStr,
                    MSG_FILEPATH_LEN_MAX);
            ma->setFilePath(infoStr);

            //set attachment id
            ma->setId(message->m_attachments.size() + 1);

            //set mime type
            msg_get_int_value(attach_info, MSG_MMS_ATTACH_MIME_TYPE_INT, &tempInt);
            std::string type = MessageAttachment::MIMETypeEnumToString(tempInt);
            ma->setMimeType(type);

            MessageAttachment* att = ma.get();
            LOGD("[att:%d] added attachement: %p (mime:0x%x mime:%s path:%s id:%d)",
                i, att, tempInt, type.c_str(), infoStr, ma->getId());

            message->m_attachments.push_back(ma);
            message->m_has_attachment = true;

            msg_release_struct(&attach_info);
        }
    } else {
        msg_release_struct(&mms_struct);
        LOGE("failed to get attachment");
        throw Common::UnknownException("failed to add attachment");
    }

    LOGD("after MSG_MMS_ATTACH_LIST attachments count is:%d",
            message->m_attachments.size());
    msg_release_struct(&mms_struct);
}

Message* Message::convertPlatformShortMessageToObject(msg_struct_t msg){
    Message *message = NULL;
    int infoInt;
    bool infoBool;
    char infoStr[MAX_ADDRESS_VAL_LEN + 1];
    //get type
    msg_get_int_value(msg, MSG_MESSAGE_TYPE_INT, &infoInt);
    if (infoInt == MSG_TYPE_SMS) {
        message = new MessageSMS();
        // get SMS body
        std::shared_ptr<MessageBody> body(new MessageBody());
        char msgInfoStr[MAX_MSG_TEXT_LEN + 1];
        msg_get_str_value(msg, MSG_MESSAGE_SMS_DATA_STR, msgInfoStr, MAX_MSG_TEXT_LEN);
        body->setPlainBody(std::string(msgInfoStr));
        message->setBody(body);
        // get recipients
        std::vector<std::string> recp_list = message->getSMSRecipientsFromStruct(msg);
        message->setTO(recp_list);
    } else if (infoInt == MSG_TYPE_MMS) {
        message = new MessageMMS();

        // get MMS body
        msg_get_int_value(msg, MSG_MESSAGE_DATA_SIZE_INT, &infoInt);
        const int mms_body_length = infoInt;

        if(mms_body_length > 0) {
            std::unique_ptr<char[]> mms_body_str(new char[mms_body_length + 1]);
            memset(mms_body_str.get(), 0, (mms_body_length + 1) * sizeof(char));

            int error = msg_get_str_value(msg, MSG_MESSAGE_MMS_TEXT_STR,
                    mms_body_str.get(), mms_body_length);
            if(MSG_SUCCESS != error) {
                LOGE("Error:%d occured during: "
                        "msg_get_str_value(...,MSG_MESSAGE_MMS_TEXT_STR,...)", error);
            } else {
                //Check if fetched string is not empty
                if((mms_body_str.get())[0] != 0) {
                    LOGD("Fetched plain body (with MSG_MESSAGE_MMS_TEXT_STR):"
                            "[%s] length:%d", mms_body_str.get(), mms_body_length);

                    std::shared_ptr<MessageBody> body (new MessageBody());
                    std::string infoString;
                    infoString.assign(mms_body_str.get());
                    body->setPlainBody(infoString);
                    message->setBody(body);
                } else {
                    LOGW("Warning: fetched plain body is empty "
                            "despite reported length is:%d!", mms_body_length);
                }

                LOGD("Set plain body: [%s]", message->getBody()->getPlainBody().c_str());
            }
        } else {
            LOGW("Warning: mms plain body length is 0!");
        }

        // get recipients
        std::vector<std::string> recp_list = getMMSRecipientsFromStruct(msg,
                MSG_RECIPIENTS_TYPE_TO);
        message->setTO(recp_list);
        recp_list = getMMSRecipientsFromStruct(msg, MSG_RECIPIENTS_TYPE_CC);
        message->setCC(recp_list);
        recp_list = getMMSRecipientsFromStruct(msg, MSG_RECIPIENTS_TYPE_BCC);
        message->setBCC(recp_list);
        // get subject
        memset(infoStr, 0, MAX_ADDRESS_VAL_LEN + 1);
        msg_get_str_value(msg, MSG_MESSAGE_SUBJECT_STR, infoStr, MAX_SUBJECT_LEN);
        message->setSubject(infoStr);
        //set attachments
        setMMSBodyAndAttachmentsFromStruct(message, msg);
    } else {
        LOGE("Invalid Message type: %d", infoInt);
        throw Common::InvalidValuesException("Invalid Message type");
    }

    // get id
    msg_get_int_value(msg, MSG_MESSAGE_ID_INT, &infoInt);
    message->setId(infoInt);
    // get conversation id
    msg_get_int_value(msg, MSG_MESSAGE_THREAD_ID_INT, &infoInt);
    message->setConversationId(infoInt);
    // get folder id
    msg_get_int_value(msg, MSG_MESSAGE_FOLDER_ID_INT, &infoInt);
    message->setFolderId(infoInt);
    // get timestamp
    msg_get_int_value(msg, MSG_MESSAGE_DISPLAY_TIME_INT, &infoInt);
    message->setTimeStamp(infoInt);
    // get from
    const std::string& from = Message::getShortMsgSenderFromStruct(msg);
    message->setFrom(from);
    LOGD("Message(%p) from is: %s", message, message->getFrom().c_str());
    // get if is in response
    msg_get_int_value(msg, MSG_MESSAGE_DIRECTION_INT, &infoInt);
    LOGD("Message(%p) direction is: %d", message, infoInt);
    message->setInResponseTo(infoInt);
    // get is read
    msg_get_bool_value(msg, MSG_MESSAGE_READ_BOOL, &infoBool);
    message->setIsRead(infoBool);

    // get status

    // This "strange" fix has been taken from old implementation:
    // void Mms::readMessageStatus(msg_struct_t& messageData)
    //
    int error = msg_get_int_value(msg, MSG_MESSAGE_FOLDER_ID_INT, &infoInt);
    if(MSG_SUCCESS == error) {
        MessageStatus msg_status;
        switch (infoInt) {
            case MSG_INBOX_ID: msg_status = MessageStatus::STATUS_LOADED; break;
            case MSG_OUTBOX_ID: msg_status = MessageStatus::STATUS_SENDING; break;
            case MSG_SENTBOX_ID: msg_status = MessageStatus::STATUS_SENT; break;
            case MSG_DRAFT_ID: msg_status = MessageStatus::STATUS_DRAFT; break;
            default: msg_status = MessageStatus::STATUS_LOADED; break;
        }
        message->setMessageStatus(msg_status);

        LOGD("MSG_MESSAGE_FOLDER_ID:%d -> messageStatus:%s", infoInt,
                MessagingUtil::messageStatusToString(msg_status).c_str());
    }
    else
    {
        LOGE("Couldn't get MSG_MESSAGE_FOLDER_ID_INT, error:%d", error);
        error = msg_get_int_value(msg, MSG_SENT_STATUS_NETWORK_STATUS_INT, &infoInt);

        if(MSG_SUCCESS == error) {
            MessageStatus msg_status;
            if (infoInt == MSG_NETWORK_SEND_SUCCESS) {
                msg_status = MessageStatus::STATUS_SENT;
            } else if (infoInt == MSG_NETWORK_SENDING) {
                msg_status = MessageStatus::STATUS_SENDING;
            } else if (infoInt == MSG_NETWORK_SEND_FAIL) {
                msg_status = MessageStatus::STATUS_FAILED;
            } else if (infoInt == MSG_NETWORK_NOT_SEND) {
                msg_status = MessageStatus::STATUS_DRAFT;
            } else {
                LOGW("warning undefined messageStatus: %d!", infoInt);
                msg_status = MessageStatus::STATUS_UNDEFINED;
            }
            message->setMessageStatus(msg_status);

            LOGD("MSG_SENT_STATUS_NETWORK_STATUS:%d MessageStatus:%s", infoInt,
                MessagingUtil::messageStatusToString(msg_status).c_str());
        } else {
            LOGE("Couldn't get MSG_SENT_STATUS_NETWORK_STATUS_INT, error:%d", error);

            if(0 == message->getId()) {
                LOGW("Both MSG_SENT_STATUS_NETWORK_STATUS_INT and "
                        "MSG_MESSAGE_FOLDER_ID_INT failed, messageId == 0 ASSUMING that"
                        "this message is in DRAFT");
                message->setMessageStatus(MessageStatus::STATUS_DRAFT);
            }
        }
    }

    LOGD("End");
    return message;
}

std::vector<std::string> Message::split(const std::string& input,
        char delimiter)
{
    std::vector<std::string> ret;
    std::stringstream stream(input);
    std::string item;
    while (getline(stream, item, delimiter)) {
        ret.push_back(item);
    }
    return ret;
}

std::vector<std::string> Message::getEmailRecipientsFromStruct(const char *recipients)
{
    std::vector<std::string> tmp = Message::split(recipients, ';');
    for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end(); ++it) {
        *it = MessagingUtil::ltrim(*it);
    }

    if (tmp.begin() != tmp.end()) {
        if (*(tmp.begin()) == "") {
            tmp.erase(tmp.begin());
        }

        if (*(tmp.end() - 1) == "") {
            tmp.erase(tmp.end() - 1);
        }
    }

    // remove '<' and '>'
    tmp = MessagingUtil::extractEmailAddresses(tmp);
    return tmp;
}

std::shared_ptr<MessageBody> Message::convertEmailToMessageBody(
        email_mail_data_t& mail)
{
    LOGD("Enter");
    std::shared_ptr<MessageBody> body (new MessageBody());
    body->updateBody(mail);
    return body;
}

AttachmentPtrVector Message::convertEmailToMessageAttachment(email_mail_data_t& mail)
{
    LOGD("Enter");
    email_attachment_data_t* attachment = NULL;
    int attachmentCount = 0;

    AttachmentPtrVector att;

    if (EMAIL_ERROR_NONE != email_get_attachment_data_list(mail.mail_id,
            &attachment, &attachmentCount)) {
        throw Common::UnknownException("Couldn't get attachment.");
    }
    if ( attachment && attachmentCount > 0) {
        for (int i = 0; i < attachmentCount; i++) {
            std::shared_ptr<MessageAttachment> tmp_att (new MessageAttachment());
            tmp_att->updateWithAttachmentData(attachment[i]);
            att.push_back(tmp_att);
        }
    }

    email_free_attachment_data(&attachment, attachmentCount);
    return att;
}

std::shared_ptr<Message> Message::convertPlatformEmailToObject(
        email_mail_data_t& mail)
{
    LOGD("Enter");
    std::shared_ptr<Message> message(new MessageEmail());
    message->updateEmailMessage(mail);
    return message;
}

void Message::updateEmailMessage(email_mail_data_t& mail)
{
    LOGW("This should be called on MessageEmail instance");
}

/**
 *  Attribute      | Attribute filter| Attribute range filter
 *                 | supported       | supported
 * ----------------+-----------------+------------------------
 *  id             | Yes             | No
 *  serviceId      | Yes             | No
 *  conversationId | No              | No
 *  folderId       | Yes             | No
 *  type           | Yes             | No
 *  timestamp      | No              | Yes
 *  from           | Yes             | No
 *  to             | Yes             | No
 *  cc             | Yes             | No
 *  bcc            | Yes             | No
 *  body.plainBody | Yes             | No
 *  isRead         | Yes             | No
 *  hasAttachment  | Yes             | No
 *  isHighPriority | Yes             | No
 *  subject        | Yes             | No
 *  isResponseTo   | No              | No
 *  messageStatus  | No              | No
 *  attachments    | No              | No
 **/
namespace MESSAGE_FILTER_ATTRIBUTE {

const std::string ID = JSMessageKeys::MESSAGE_ATTRIBUTE_ID;
const std::string SERVICE_ID = "serviceId";
const std::string CONVERSATION_ID = JSMessageKeys::MESSAGE_ATTRIBUTE_CONVERSATION_ID;
const std::string FOLDER_ID = JSMessageKeys::MESSAGE_ATTRIBUTE_FOLDER_ID;
const std::string TYPE = JSMessageKeys::MESSAGE_ATTRIBUTE_TYPE;
const std::string TIMESTAMP = JSMessageKeys::MESSAGE_ATTRIBUTE_TIMESTAMP;
const std::string FROM = JSMessageKeys::MESSAGE_ATTRIBUTE_FROM;
const std::string TO = JSMessageKeys::MESSAGE_ATTRIBUTE_TO;
const std::string CC = JSMessageKeys::MESSAGE_ATTRIBUTE_CC;
const std::string BCC = JSMessageKeys::MESSAGE_ATTRIBUTE_BCC;
const std::string BODY_PLAIN_BODY = "body.plainBody";
const std::string IS_READ = JSMessageKeys::MESSAGE_ATTRIBUTE_IS_READ;
const std::string HAS_ATTACHMENT = JSMessageKeys::MESSAGE_ATTRIBUTE_HAS_ATTACHMENT;
const std::string IS_HIGH_PRIORITY = JSMessageKeys::MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY;
const std::string SUBJECT = JSMessageKeys::MESSAGE_ATTRIBUTE_SUBJECT;

} //namespace MESSAGE_FILTER_ATTRIBUTE

bool Message::isMatchingAttribute(const std::string& attribute_name,
            const FilterMatchFlag match_flag,
            AnyPtr match_value) const
{
    LOGD("Entered");
    auto key = match_value->toString();
    LOGD("attribute_name:%s match_flag:%d matchValue:%s", attribute_name.c_str(),
            match_flag, key.c_str());

    using namespace MESSAGE_FILTER_ATTRIBUTE;

    if (ID == attribute_name) {
        return FilterUtils::isStringMatching(key, std::to_string(getId()),
                match_flag);
    }
    else if (SERVICE_ID == attribute_name) {
        if(is_service_is_set()) {
            return FilterUtils::isStringMatching(key, std::to_string(getServiceId()),
                    match_flag);
        }
    }
    else if (FOLDER_ID == attribute_name) {
        return FilterUtils::isStringMatching(key, std::to_string(getFolderId()),
                match_flag);
    }
    else if (TYPE == attribute_name) {
        const std::string msgTypeStr = MessagingUtil::messageTypeToString(getType());
        return FilterUtils::isStringMatching(key, msgTypeStr, match_flag);
    }
    else if (FROM == attribute_name) {
        return FilterUtils::isStringMatching(key, getFrom() , match_flag);
    }
    else if (TO == attribute_name) {
        return FilterUtils::isAnyStringMatching(key, getTO(), match_flag);
    }
    else if (CC == attribute_name) {
        return FilterUtils::isAnyStringMatching(key, getCC(), match_flag);
    }
    else if (BCC == attribute_name) {
        return FilterUtils::isAnyStringMatching(key, getBCC(), match_flag);
    }
    else if (BODY_PLAIN_BODY == attribute_name) {
        if(getBody()) {
            return FilterUtils::isStringMatching(key, getBody()->getPlainBody(),
                    match_flag);
        }
    }
    else if (IS_READ == attribute_name) {
        return FilterUtils::isStringMatching(key, FilterUtils::boolToString(getIsRead()),
                match_flag);
    }
    else if (HAS_ATTACHMENT == attribute_name) {
        return FilterUtils::isStringMatching(key,
                FilterUtils::boolToString(getHasAttachment()),
                match_flag);
    }
    else if (IS_HIGH_PRIORITY == attribute_name) {
        return FilterUtils::isStringMatching(key,
                FilterUtils::boolToString(getIsHighPriority()),
                match_flag);
    }
    else if (SUBJECT == attribute_name) {
        return FilterUtils::isStringMatching(key, getSubject(), match_flag);
    }
    else {
        LOGD("attribute:%s is NOT SUPPORTED", attribute_name.c_str());
    }

    return false;
}

bool Message::isMatchingAttributeRange(const std::string& attribute_name,
            AnyPtr initial_value,
            AnyPtr end_value) const
{
    LOGD("Entered attribute_name: %s", attribute_name.c_str());

    using namespace MESSAGE_FILTER_ATTRIBUTE;
    if(TIMESTAMP == attribute_name) {
        return FilterUtils::isTimeStampInRange(getTimestamp(), initial_value,
                end_value);
    }
    else {
        LOGD("attribute:%s is NOT SUPPORTED", attribute_name.c_str());
    }

    return false;
}

} // Messaging
} // DeviceAPI
