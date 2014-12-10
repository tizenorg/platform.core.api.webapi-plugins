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

#ifndef __TIZEN_JS_MESSAGE_H__
#define __TIZEN_JS_MESSAGE_H__

#include <JavaScriptCore/JavaScript.h>
#include <Logger.h>
#include "Message.h"
// headers below needed to declare function that converts vector of
// MessageAttachment into JSArray
#include <vector>
#include <memory>
#include "MessageAttachment.h"

namespace DeviceAPI {
namespace Messaging {

namespace JSMessageKeys {
extern const char* MESSAGE_ATTRIBUTE_ID;
extern const char* MESSAGE_ATTRIBUTE_CONVERSATION_ID;
extern const char* MESSAGE_ATTRIBUTE_FOLDER_ID;
extern const char* MESSAGE_ATTRIBUTE_TYPE;
extern const char* MESSAGE_ATTRIBUTE_TIMESTAMP;
extern const char* MESSAGE_ATTRIBUTE_FROM;
extern const char* MESSAGE_ATTRIBUTE_TO;
extern const char* MESSAGE_ATTRIBUTE_CC;
extern const char* MESSAGE_ATTRIBUTE_BCC;
extern const char* MESSAGE_ATTRIBUTE_BODY;
extern const char* MESSAGE_ATTRIBUTE_IS_READ;
extern const char* MESSAGE_ATTRIBUTE_IS_HIGH_PRIORITY;
extern const char* MESSAGE_ATTRIBUTE_SUBJECT;
extern const char* MESSAGE_ATTRIBUTE_IN_RESPONSE_TO;
extern const char* MESSAGE_ATTRIBUTE_MESSAGE_STATUS;
extern const char* MESSAGE_ATTRIBUTE_ATTACHMENTS;
extern const char* MESSAGE_ATTRIBUTE_HAS_ATTACHMENT;
} //namespace MESSAGE_ATTRIBUTE

class JSMessage
{
public:
    static const JSClassDefinition* getClassInfo();

    static JSClassRef getClassRef();

    static JSObjectRef constructor(JSContextRef ctx,
            JSObjectRef constructor,
            size_t argumentCount,
            const JSValueRef arguments[],
            JSValueRef* exception);

    static std::shared_ptr<Message> getPrivateObject(JSContextRef context,
            JSValueRef value);

    static void setPrivateObject(JSObjectRef object,
            std::shared_ptr<Message> data);

    static JSObjectRef makeJSObject(JSContextRef context,
            std::shared_ptr<Message> ptr);

    /**
     * Function converts vector of shared_pointers with Message into JSArray
     */
    static JSObjectRef messageVectorToJSObjectArray(JSContextRef context,
            const MessagePtrVector& messages);

private:
    /**
     * The callback invoked when an object is first created.
     */
    static void initialize(JSContextRef context,
            JSObjectRef object);

    /**
     * The callback invoked when an object is finalized.
     */
    static void finalize(JSObjectRef object);

    static JSValueRef getAttachments(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getBccAddress(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getCcAddress(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getDestinationAddress(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getIsRead(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getMessageId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getMessagePriority(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getMessageType(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getSourceAddress(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getSubject(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getTime(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getFolder(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getMessageBody(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static bool setAttachments(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static bool setBccAddress(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static bool setCcAddress(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static bool setDestinationAddress(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static bool setIsRead(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static bool setMessagePriority(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static bool setSubject(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static bool setMessageBody(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static JSValueRef getConversationId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getInResponseTo(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getMessageStatus(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef hasAttachment(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    /**
     * This structure describes a statically declared function property.
     */
    static JSStaticFunction m_function[];

    /**
     * This structure contains properties and callbacks that define a type of object.
     */
    static JSClassDefinition m_classInfo;

    /**
     * This member variable contains the initialization values for the static properties of this class.
     * The values are given according to the data structure JSPropertySpec
     */
    static JSStaticValue m_property[];

    /**
     * This structure describes a statically declared function property.
     */
    static JSClassRef m_jsClassRef;

};

} //Messaging
} //DeviceAPI

#endif //__TIZEN_JS_MESSAGE_H__

