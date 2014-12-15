//
// Tizen Web Device API
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#ifndef _TIZEN_ARCHIVE_JS_ARCHIVE_FILE_ENTRY_H_
#define _TIZEN_ARCHIVE_JS_ARCHIVE_FILE_ENTRY_H_
#include <JavaScriptCore/JavaScript.h>

#include "ArchiveFileEntry.h"

namespace DeviceAPI {
namespace Archive {

// these constants are used in JSArchiveFile.cpp and JSArchiveFileEntry.cpp
extern const char* ARCHIVE_FILE_ENTRY_OPT_DEST;
extern const char* ARCHIVE_FILE_ENTRY_OPT_STRIP;
extern const char* ARCHIVE_FILE_ENTRY_OPT_COMPRESSION_LEVEL;

class JSArchiveFileEntry {
public:
    static const JSClassDefinition* getClassInfo();
    static const JSClassRef getClassRef();

    static ArchiveFileEntryPtr getPrivateObject(JSContextRef context,
            JSValueRef velue_ref);

    static JSObjectRef makeJSObject(JSContextRef context,
            ArchiveFileEntryPtr native);

private:

    /**
     * This member variable contains the values which has to be passed
     * when the this class is embedded into JS Engine.
     */
    static JSClassDefinition m_classInfo;

    /**
     * This structure describes a statically declared function property.
     */
    static JSStaticFunction m_function[];

    /**
     * This member variable contains the initialization values
     * for the static properties of this class.
     * The values are given according to the data structure JSPropertySpec
     */
    static JSStaticValue m_property[];

    static JSClassRef m_jsClassRef;

    /**
     * The callback invoked when an object is first created.
     */
    static void initialize(JSContextRef context, JSObjectRef object);

    /**
     * The callback invoked when an object is finalized.
     */
    static void finalize(JSObjectRef object);

    static JSValueRef getName(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getSize(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getCompressedSize(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef getModified(JSContextRef context,
            JSObjectRef object,
            JSStringRef propertyName,
            JSValueRef* exception);

    static JSValueRef extract(JSContextRef context,
            JSObjectRef object,
            JSObjectRef thisObject,
            size_t argumentCount,
            const JSValueRef arguments[],
            JSValueRef* exception);
};

} // Archive
} // DeviceAPI

#endif // _TIZEN_ARCHIVE_JS_ARCHIVE_FILE_ENTRY_H_
