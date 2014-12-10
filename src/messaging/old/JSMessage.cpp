//
// Tizen Web Device API
// Copyright (c) 2012 Samsung Electronics Co., Ltd.
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

#include <JSWebAPIErrorFactory.h>
#include <PlatformException.h>
#include <JSUtil.h>

#include <GlobalContextManager.h>
#include <ArgumentValidator.h>
#include <Export.h>
#include <Logger.h>

#include "JSMessage.h"
#include "JSMessageBody.h"
#include "JSMessageAttachment.h"

#include "plugin_config.h"

#include "MessagingUtil.h"
#include "Message.h"
#include "MessageSMS.h"
#include "MessageMMS.h"
#include "MessageEmail.h"

using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

namespace {
const char* MESSAGE = "Message";

const char* MESSAGE_DICTIONARY_ATTRIBUTE_PLAIN_BODY = "plainBody";
const char* MESSAGE_DICTIONARY_ATTRIBUTE_HTML_BODY = "htmlBody";
}

namespace JSMessageKeys {
const char* MESSAGE_ATTRIBUTE_ID = "id";
const char* MESSAGE_ATTRIBUTE_CONVERSATION_ID = "conversationId";
const char* MESSAGE_ATTRIBUTE_FOLDER_ID = "folderId";
const char* MESSAGE_ATTRIBUTE_TYPE = "type";
const char* MESSAGE_ATTRIBUTE_TIMESTAMP = "timestamp";
const char* MESSAGE_ATTRIBUTE_FROM = "from";
const char* MESSAGE_ATTRIBUTE_TO = "to"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_CC = "cc"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_BCC = "bcc"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_BODY = "body";
const char* MESSAGE_ATTRIBUTE_IS_READ = "isRead";
const char* MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY = "isHighPriority"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_SUBJECT = "subject"; // used also in dictionary
const char* MESSAGE_ATTRIBUTE_IN_RESPONSE_TO = "inResponseTo";
const char* MESSAGE_ATTRIBUTE_MESSAGE_STATUS = "messageStatus";
const char* MESSAGE_ATTRIBUTE_ATTACHMENTS = "attachments";
const char* MESSAGE_ATTRIBUTE_HAS_ATTACHMENT = "hasAttachment";
} // namespace JSMessage

using namespace JSMessageKeys;

JSClassRef JSMessage::m_jsClassRef = NULL;

JSClassDefinition JSMessage::m_classInfo = {
    0,
    kJSClassAttributeNone,
    MESSAGE,
    NULL,
    JSMessage::m_property,
    NULL, // m_function
    JSMessage::initialize,
    JSMessage::finalize,
    NULL, //hasProperty,
    NULL, //getProperty,
    NULL, //setProperty,
    NULL, //deleteProperty,
    NULL, //getPropertyNames,
    NULL, //callAsFunction,
    NULL, //callAsConstructor,
    NULL, //hasInstance,
    NULL, //convertToType,
};

JSStaticValue JSMessage::m_property[] = {
    { MESSAGE_ATTRIBUTE_ID, getMessageId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTRIBUTE_CONVERSATION_ID, getConversationId, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTRIBUTE_FOLDER_ID, getFolder, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTRIBUTE_TYPE, getMessageType, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTRIBUTE_TIMESTAMP, getTime, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTRIBUTE_FROM, getSourceAddress, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
    { MESSAGE_ATTRIBUTE_TO, getDestinationAddress, setDestinationAddress, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_CC, getCcAddress, setCcAddress, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_BCC, getBccAddress, setBccAddress, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_BODY, getMessageBody, setMessageBody, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_IS_READ, getIsRead, setIsRead, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY, getMessagePriority, setMessagePriority, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_SUBJECT, getSubject, setSubject, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_IN_RESPONSE_TO, getInResponseTo, NULL, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_MESSAGE_STATUS, getMessageStatus, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
    { MESSAGE_ATTRIBUTE_ATTACHMENTS, getAttachments, setAttachments, kJSPropertyAttributeDontDelete },
    { MESSAGE_ATTRIBUTE_HAS_ATTACHMENT, hasAttachment, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
    { 0, 0, 0, 0 }
};

const JSClassDefinition* JSMessage::getClassInfo()
{
    LOGD("Entered");
    return &(m_classInfo);
}

JSClassRef DLL_EXPORT JSMessage::getClassRef()
{
    LOGD("Entered");
    if (!m_jsClassRef) {
        m_jsClassRef = JSClassCreate(&m_classInfo);
    }
    return m_jsClassRef;
}

JSObjectRef DLL_EXPORT JSMessage::constructor(JSContextRef context,
        JSObjectRef constructor,
        size_t argumentCount,
        const JSValueRef arguments[],
        JSValueRef* exception)
{
    LOGD("Entered");

    ArgumentValidator validator(context, argumentCount, arguments);

    JSObjectRef jsObjRef = JSObjectMake(context, JSMessage::getClassRef(), NULL);

    // constructor
    JSStringRef ctorName = JSStringCreateWithUTF8CString("constructor");
    JSObjectSetProperty(context, jsObjRef, ctorName, constructor,
        kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete | kJSPropertyAttributeDontEnum, NULL);
    JSStringRelease(ctorName);

    try {
        std::shared_ptr<Message> priv;

        std::string msgTypeString = validator.toString(0);
        LOGD(" Message type : %s", msgTypeString.c_str());
        JSObjectRef dictionary = validator.toObject(1, true);

        try {
            MessageType msgtype = MessagingUtil::stringToMessageType(msgTypeString);
            switch(msgtype) {
                case MessageType(SMS):
                    priv = std::make_shared<MessageSMS>();
                    break;
                case MessageType(MMS):
                    priv = std::make_shared<MessageMMS>();
                    break;
                case MessageType(EMAIL):
                    priv = std::make_shared<MessageEmail>();
                    break;
                default:
                    break;
            }
        }
        catch(const BasePlatformException &ex) {
            LOGE("Invalid message type given: %s.", msgTypeString.c_str());
            throw ex;
        }

        setPrivateObject(jsObjRef, priv);

        // both - dictionary and private object - have to be not NULL
        if (dictionary != NULL && priv.get() != NULL) {
            JSValueRef subjectData = JSUtil::getProperty(context, dictionary,
                    MESSAGE_ATTRIBUTE_SUBJECT);
            JSValueRef toData = JSUtil::getProperty(context, dictionary,
                    MESSAGE_ATTRIBUTE_TO);
            JSValueRef ccData = JSUtil::getProperty(context, dictionary,
                    MESSAGE_ATTRIBUTE_CC);
            JSValueRef bccData = JSUtil::getProperty(context, dictionary,
                    MESSAGE_ATTRIBUTE_BCC);
            JSValueRef plainBodyData = JSUtil::getProperty(context, dictionary,
                    MESSAGE_DICTIONARY_ATTRIBUTE_PLAIN_BODY);
            JSValueRef htmlBodyData = JSUtil::getProperty(context, dictionary,
                    MESSAGE_DICTIONARY_ATTRIBUTE_HTML_BODY);
            JSValueRef priorityData = JSUtil::getProperty(context, dictionary,
                    MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY);

            if (!JSValueIsUndefined(context, subjectData)) {
                std::string subject = JSUtil::JSValueToString(context, subjectData);
                LOGD(" Subject: %s", subject.c_str());
                priv->setSubject(subject);
            }
            if (!JSValueIsUndefined(context, toData)) {
                std::vector<std::string> to = JSUtil::JSArrayToStringVector(
                        context, toData);
                priv->setTO(to);
            }
            if (!JSValueIsUndefined(context, ccData)) {
                std::vector<std::string> cc = JSUtil::JSArrayToStringVector(
                        context, ccData);
                priv->setCC(cc);
            }
            if (!JSValueIsUndefined(context, bccData)) {
                std::vector<std::string> bcc = JSUtil::JSArrayToStringVector(
                        context, bccData);
                priv->setBCC(bcc);
            }
            if (!JSValueIsUndefined(context, htmlBodyData)) {
                std::string htmlBody = JSUtil::JSValueToString(context, htmlBodyData);
                LOGD(" htmlBody: %s", htmlBody.c_str());
                priv->getBody()->setHtmlBody(htmlBody);
            }
            if (!JSValueIsUndefined(context, priorityData)) {
                priv->setIsHighPriority(JSUtil::JSValueToBoolean(context, priorityData));
            }
            if (!JSValueIsUndefined(context, plainBodyData)) {
                std::string plainBody = JSUtil::JSValueToString(context, plainBodyData);
                LOGD(" plainBody: %s", plainBody.c_str());
                priv->getBody()->setPlainBody(plainBody);
            }
        }
    }
    catch (const BasePlatformException &err) {
        LOGE("Message creation failed: %s", err.getMessage().c_str());
    }
    catch (...) {
        LOGE("Message creation failed - unsupported error.");
    }

    return jsObjRef;
}

void JSMessage::initialize(JSContextRef context,
        JSObjectRef object)
{
    LOGD("Entered");
}

void JSMessage::finalize(JSObjectRef object)
{
    LOGD("Entered");
    // Below holder is fetched from JSObject because holder
    // with last shared_ptr instace should be removed
    MessageHolder* priv = static_cast<MessageHolder*>(JSObjectGetPrivate(object));
    JSObjectSetPrivate(object, NULL);
    delete priv;
}

std::shared_ptr<Message> JSMessage::getPrivateObject(JSContextRef context,
        JSValueRef value)
{
    if (!JSValueIsObjectOfClass(context, value, getClassRef())) {
        LOGE("Object type do not match");
        throw TypeMismatchException("Object type is not Message");
    }

    JSObjectRef object = JSUtil::JSValueToObject(context, value);
    MessageHolder* priv = static_cast<MessageHolder*>(
            JSObjectGetPrivate(object));
    if (!priv) {
        LOGE("NULL private data");
        throw UnknownException("Private data holder is null");
    }
    if (!(priv->ptr)) {
        LOGE("NULL shared pointer in private data");
        throw UnknownException("Private data is null");
    }

    return priv->ptr;
}

void JSMessage::setPrivateObject(JSObjectRef object, std::shared_ptr<Message> data)
{
    if (!data) {
        LOGE("NULL shared pointer given to set as private data");
        throw UnknownException("NULL private data given");
    }
    MessageHolder* priv = static_cast<MessageHolder*>(
            JSObjectGetPrivate(object));
    if (priv) {
        priv->ptr = data;
    }
    else {
        priv = new(std::nothrow) MessageHolder();
        if (!priv) {
            LOGE("Memory allocation failure");
            throw UnknownException("Failed to allocate memory");
        }
        priv->ptr = data;
        if(!JSObjectSetPrivate(object, static_cast<void*>(priv))) {
            delete priv;
            priv = NULL;
            LOGE("Failed to set private data in Message");
            throw UnknownException(
                    "Failed to set Message private data");
        }
    }
}

JSObjectRef JSMessage::makeJSObject(JSContextRef context,
        std::shared_ptr<Message> ptr)
{
    if (!ptr) {
        LOGE("NULL pointer to message given");
        throw UnknownException("NULL pointer to message given");
    }

    MessageHolder* priv = new(std::nothrow) MessageHolder();
    if (!priv) {
        LOGW("Failed to allocate memory for MessageHolder");
        throw UnknownException("Priv is null");
    }
    priv->ptr = ptr;

    JSObjectRef obj = JSObjectMake(context, getClassRef(), NULL);
    if(!JSObjectSetPrivate(obj, static_cast<void*>(priv))) {
        LOGE("Failed to set private in Message");
        throw UnknownException("Private data not set");
    }
    return obj;
}

JSObjectRef JSMessage::messageVectorToJSObjectArray(JSContextRef context,
        const MessagePtrVector& messages)
{
    size_t count = messages.size();

    JSObjectRef array[count];
    for (size_t i = 0; i < count; i++) {
        array[i] = JSMessage::makeJSObject(context, messages[i]);
    }
    JSObjectRef result = JSObjectMakeArray(context, count,
            count > 0 ? array : NULL, NULL);
    if (!result) {
        LOGW("Failed to create Message array");
        throw UnknownException("Message array is null");
    }
    return result;
}

JSValueRef JSMessage::getAttachments(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return priv->getJSMessageAttachments(
            Common::GlobalContextManager::getInstance()
                ->getGlobalContext(context));
    }
    catch(const BasePlatformException& err) {
        LOGE("Failed to get attachments: %s", err.getMessage().c_str());
    }
    catch(...) {
        LOGE("Unsupported error while getting message attachment.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getBccAddress(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return priv->getJSBCC(
            Common::GlobalContextManager::getInstance()
                ->getGlobalContext(context));
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get BCC. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting BCC.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getCcAddress(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return priv->getJSCC(
            Common::GlobalContextManager::getInstance()
                ->getGlobalContext(context));
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get CC. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting CC.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getDestinationAddress(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return priv->getJSTO(
            Common::GlobalContextManager::getInstance()
                ->getGlobalContext(context));
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get TO. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting TO.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getIsRead(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getIsRead());
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get is_read flag. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getMessageId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        if(priv->is_id_set()) {
            std::string stringid = std::to_string(priv->getId());
            return JSUtil::toJSValueRef(context, stringid);
        }
        else {
            return JSValueMakeNull(context);
        }
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get message id. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting message id.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getMessagePriority(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getIsHighPriority());
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get priority. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getMessageType(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context,
                MessagingUtil::messageTypeToString(priv->getType()));
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get message type. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting message type.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getSourceAddress(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        if(priv->is_from_set()) {
            return JSUtil::toJSValueRef(context, priv->getFrom());
        }
        else {
            return JSValueMakeNull(context);
        }
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get source address. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting source address.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getSubject(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");

    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getSubject());
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get subject. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting subject.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getTime(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        if(priv->is_timestamp_set()) {
            return JSUtil::makeDateObject(context, priv->getTimestamp());
        }
        else {
            return JSValueMakeNull(context);
        }
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get timestamp. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting timestamp.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getFolder(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        if(priv->is_folder_id_set()) {
            std::string stringid = std::to_string(priv->getFolderId());
            return JSUtil::toJSValueRef(context, stringid);
        }
        else {
            return JSValueMakeNull(context);
        }

    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get folder id. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting subject.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getMessageBody(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSMessageBody::makeJSObject(context, priv->getBody());
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to get MessageBody: %s", err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while setting BCC.");
    }
    return JSValueMakeUndefined(context);
}

bool JSMessage::setAttachments(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        if(!JSIsArrayValue(context,value)) {
            return true;
        }

        auto priv = getPrivateObject(context, object);

        AttachmentPtrVector atts;
        atts = JSUtil::JSArrayToType_<std::shared_ptr<MessageAttachment>>(
                context, value, JSMessageAttachment::getPrivateObject);
        priv->setMessageAttachments(atts);
    }
    catch (const BasePlatformException& err) {
        LOGE("Failed to set Attachments: %s", err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while setting Attachments.");
    }
    return true;
}

bool JSMessage::setBccAddress(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        if(!JSIsArrayValue(context,value)) {
            return true;
        }

        auto priv = getPrivateObject(context, object);

        std::vector<std::string> bcc = JSUtil::JSArrayToStringVector(
                context, value);
        priv->setBCC(bcc);
    }
    catch (const BasePlatformException &err) {
        LOGE("Failed to set BCC. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while setting BCC.");
    }
    return true;
}

bool JSMessage::setCcAddress(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        if(!JSIsArrayValue(context,value)) {
            return true;
        }

        auto priv = getPrivateObject(context, object);

        std::vector<std::string> cc = JSUtil::JSArrayToStringVector(
                context, value);
        priv->setCC(cc);
    }
    catch (const BasePlatformException &err) {
        LOGE("Failed to set CC. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while setting CC.");
    }
    return true;
}

bool JSMessage::setDestinationAddress(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef * exception)
{
    LOGD("Entered");
    try {
        if(!JSIsArrayValue(context,value)) {
            return true;
        }

        auto priv = getPrivateObject(context, object);

        std::vector<std::string> to = JSUtil::JSArrayToStringVector(
                context, value);
        priv->setTO(to);
    }
    catch (const BasePlatformException &err) {
        LOGE("Failed to set TO. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while setting TO.");
    }
    return true;
}

bool JSMessage::setIsRead(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef * exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        priv->setIsRead(JSUtil::JSValueToBoolean(context, value));
    }
    catch (const BasePlatformException &err) {
        LOGE("Failed to set isRead flag. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return true;
}

bool JSMessage::setMessagePriority(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef * exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        priv->setIsHighPriority(JSUtil::JSValueToBoolean(context, value));
    }
    catch (const BasePlatformException &err) {
        LOGE("Failed to set priority. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return true;
}

bool JSMessage::setSubject(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef * exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        priv->setSubject(JSUtil::JSValueToString(context, value));
    }
    catch (const BasePlatformException &err) {
        LOGE("Failed to set subject. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while setting subject.");
    }
    return true;
}

bool JSMessage::setMessageBody(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef value,
        JSValueRef* exception)
{
    LOGD("Entered");
    // TODO: MessageBody setting problem should be solved in spec or implementation
    return true;
}

JSValueRef JSMessage::getConversationId(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        if(priv->is_conversation_id_set()) {
            std::string stringid = std::to_string(priv->getConversationId());
            return JSUtil::toJSValueRef(context, stringid);
        }
        else {
            return JSValueMakeNull(context);
        }
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get conversation id. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting conversation id.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getInResponseTo(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        if(priv->is_in_response_set()) {
            std::string stringid = std::to_string(priv->getInResponseTo());
            return JSUtil::toJSValueRef(context, stringid);
        }
        else {
            return JSValueMakeNull(context);
        }
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get inResponeTo. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting inResponseTo.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::getMessageStatus(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context,
                MessagingUtil::messageStatusToString(priv->getMessageStatus()));
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get message status. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    // Catch all exceptions not related to platform API execution failures
    catch(...) {
        LOGE("Unsupported error while getting message status.");
    }
    return JSValueMakeUndefined(context);
}

JSValueRef JSMessage::hasAttachment(JSContextRef context,
        JSObjectRef object,
        JSStringRef propertyName,
        JSValueRef* exception)
{
    LOGD("Entered");
    try {
        auto priv = getPrivateObject(context, object);

        return JSUtil::toJSValueRef(context, priv->getHasAttachment());
    }
    catch(const BasePlatformException &err) {
        LOGE("Failed to get hasAttachment flag. %s : %s", err.getName().c_str(), err.getMessage().c_str());
    }
    return JSValueMakeUndefined(context);
}

} //Messaging
} //DeviceAPI

