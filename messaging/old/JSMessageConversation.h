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

#ifndef __TIZEN_JS_MESSAGE_CONVERSATION_H__
#define __TIZEN_JS_MESSAGE_CONVERSATION_H__

#include <JavaScriptCore/JavaScript.h>
#include "MessageConversation.h"

namespace DeviceAPI {
namespace Messaging {

namespace JSMessageConversationKeys {
extern const char* MESSAGE_CONVERSATION_ID;
extern const char* MESSAGE_CONVERSATION_TYPE;
extern const char* MESSAGE_CONVERSATION_TIMESTAMP;
extern const char* MESSAGE_CONVERSATION_MSG_COUNT;
extern const char* MESSAGE_CONVERSATION_UNREAD_MSG;
extern const char* MESSAGE_CONVERSATION_PREVIEW;
extern const char* MESSAGE_CONVERSATION_SUBJECT;
extern const char* MESSAGE_CONVERSATION_IS_READ;
extern const char* MESSAGE_CONVERSATION_FROM;
extern const char* MESSAGE_CONVERSATION_TO;
extern const char* MESSAGE_CONVERSATION_CC;
extern const char* MESSAGE_CONVERSATION_BCC;
extern const char* MESSAGE_CONVERSATION_LAST_MSG_ID;
}

class JSMessageConversation
{
public:
/* @struct JSClassDefinition
 *@abstract This structure contains properties and callbacks that define a type of object.
 *@abstract All fields other than the version field are optional. Any pointer may be NULL.
 */
    static const JSClassDefinition* getClassInfo();

    static JSClassRef getClassRef();

    static std::shared_ptr<MessageConversation> getPrivateObject(JSContextRef context,
            JSValueRef value);

    static void setPrivateObject(JSObjectRef object,
            std::shared_ptr<MessageConversation> data);

    static JSObjectRef makeJSObject(JSContextRef context,
            std::shared_ptr<MessageConversation> ptr);

private:
    /**
     * The callback invoked when an object is first created.
     */
    static void initialize(JSContextRef context, JSObjectRef object);

    /**
     * The callback invoked when an object is finalized.
     */
    static void finalize(JSObjectRef object);

    static JSValueRef getId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getType(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getTimestamp(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getMsgCount(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getUnreadMsg(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getPreview(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getSubject(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getIsRead(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getFrom(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getTo(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getCC(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getBCC(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getLastMsgId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);


    /**
     * This structure contains properties and callbacks that define a type of object.
     */
    static JSClassDefinition m_classInfo;

    /**
     * This member variable contains the initialization values for the static properties
     * of this class.
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

#endif //__TIZEN_JS_MESSAGE_CONVERSATION_H__
