//
// Tizen Web Device API
// Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
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

#ifndef __TIZEN_JS_MESSAGING_MANAGER_H__
#define __TIZEN_JS_MESSAGING_MANAGER_H__

#include <JavaScriptCore/JavaScript.h>

namespace DeviceAPI {
namespace Messaging {

class JSMessagingManager
{
public:
// @struct JSClassDefinition
// @abstract This structure contains properties and callbacks that define a type of object.
//           All fields other than the version field are optional. Any pointer may be NULL.
    static const JSClassDefinition* getClassInfo();
    static const JSClassRef getClassRef();

private:
    /**
     * The callback invoked when an object is first created.
     */
    static void initialize(JSContextRef context, JSObjectRef object);

    /**
     * The callback invoked when an object is finalized.
     */
    static void finalize(JSObjectRef object);

    /**
    * The callback invoked when determining whether an object has a property.
    */
    static bool hasProperty(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName);

    static JSValueRef getMessageServices(JSContextRef context,
            JSObjectRef object,
            JSObjectRef thisObject,
            size_t argumentCount,
            const JSValueRef arguments[],
            JSValueRef* exception);

    /**
     * This structure contains properties and callbacks that define a type of object.
     */
    static JSClassDefinition m_classInfo;

    /**
     * This structure describes a statically declared function property.
     */
    static JSStaticFunction m_function[];

    /**
     * This structure describes a statically declared function property.
     */
    static JSClassRef m_jsClassRef;

};

} //Messaging
} //DeviceAPI

#endif //__TIZEN_JS_MESSAGING_MANAGER_H__

