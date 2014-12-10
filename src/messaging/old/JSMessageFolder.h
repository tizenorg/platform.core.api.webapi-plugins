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

/**
 * @file        JSMessageFolder.h
 */


#ifndef __TIZEN_JS_MESSAGE_FOLDER_H__
#define __TIZEN_JS_MESSAGE_FOLDER_H__

#include <JavaScriptCore/JavaScript.h>
#include "MessageFolder.h"

namespace DeviceAPI {
namespace Messaging {

class JSMessageFolder
{
public:
    static const JSClassDefinition* getClassInfo();

    static JSClassRef getClassRef();

    static std::shared_ptr<MessageFolder> getPrivateObject(JSContextRef context, JSValueRef value);

    static void setPrivateObject(JSObjectRef object,
            std::shared_ptr<MessageFolder> data);

    static JSObjectRef makeJSObject(JSContextRef context,
            std::shared_ptr<MessageFolder> ptr);

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

    static JSValueRef getParentId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getServiceId(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getContentType(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getName(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getPath(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getType(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getSynchronizable(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static bool setName(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
            JSValueRef* exception);

    static bool setSynchronizable(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef value,
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

#endif //__TIZEN_JS_MESSAGE_FOLDER_H__

