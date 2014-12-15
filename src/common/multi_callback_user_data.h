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

// Class added for backward compatibility with WRT plugins.
// To be cleaned/replaced in the future (TODO).

#ifndef __TIZEN_COMMON_MULTI_CALLBACK_USER_DATA_H__
#define __TIZEN_COMMON_MULTI_CALLBACK_USER_DATA_H__

//#include <JavaScriptCore/JavaScript.h>
#include <string>
#include <map>
#include <memory>

//namespace DeviceAPI {
namespace common {

class MultiCallbackUserData;
typedef std::shared_ptr<MultiCallbackUserData> MultiCallbackUserDataPtr;

class MultiCallbackUserData {
public:
    MultiCallbackUserData();
//    MultiCallbackUserData(JSContextRef global_ctx, JSObjectRef object);
    virtual ~MultiCallbackUserData();

//    JSContextRef getContext() const;
//    void setCallback(const std::string &key, JSObjectRef callback);
//
//    void invokeCallback(const std::string &key);
//    void invokeCallback(const std::string &key, JSValueRef obj);
//    void invokeCallback(const std::string &key, int count, JSValueRef obj[]);

//private:
//    JSContextRef m_context;
//    JSObjectRef m_object;
//    typedef std::map<const std::string, JSObjectRef> CallbackMapT;
//    CallbackMapT m_callbacks;
};

} // Common
//} // DeviceAPI

#endif //__TIZEN_COMMON_MULTI_CALLBACK_USER_DATA_H__
