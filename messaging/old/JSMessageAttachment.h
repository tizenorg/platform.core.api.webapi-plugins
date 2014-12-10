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

#ifndef __TIZEN_JS_MESSAGE_ATTACHMENT_H__
#define __TIZEN_JS_MESSAGE_ATTACHMENT_H__

#include <JavaScriptCore/JavaScript.h>
#include <memory>

namespace DeviceAPI {
namespace Messaging {

class MessageAttachment;
typedef std::vector<std::shared_ptr<MessageAttachment>> AttachmentPtrVector;

class JSMessageAttachment
{
public:
    static const JSClassDefinition* getClassInfo();

    static JSClassRef getClassRef();

    static JSObjectRef constructor(JSContextRef ctx,
            JSObjectRef constructor,
            size_t argumentCount,
            const JSValueRef arguments[],
            JSValueRef* exception);

    static std::shared_ptr<MessageAttachment> getPrivateObject(JSContextRef context,
            JSValueRef value);

    static void setPrivateObject(JSObjectRef object,
            std::shared_ptr<MessageAttachment> data);

    static JSObjectRef makeJSObject(JSContextRef context,
        std::shared_ptr<MessageAttachment> attptr);

    /**
     * Function converts vector of shared_pointers with MessageAttachment
     * into JSArray
     */
    static JSObjectRef attachmentVectorToJSObjectArray(JSContextRef context,
            const AttachmentPtrVector& atts);

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

    static JSValueRef getId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getMsgId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getMimeType(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getFilePath(JSContextRef context,
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

#endif //__TIZEN_JS_MESSAGE_ATTACHMENT_H__

