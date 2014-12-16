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

#ifndef __TIZEN_COMMON_CALLBACK_USER_DATA_H__
#define __TIZEN_COMMON_CALLBACK_USER_DATA_H__

//#include <JavaScriptCore/JavaScript.h>
#include "common/picojson.h"

#include <memory>

//namespace DeviceAPI {
namespace common {

//class MultiCallbackUserData;

class CallbackUserData {
public:
    CallbackUserData();
    virtual ~CallbackUserData();

//    JSContextRef getContext();
//    void setSuccessCallback(JSValueRef on_success);
//    void setErrorCallback(JSValueRef on_error);
//
//    void callSuccessCallback();
//    void callSuccessCallback(JSValueRef obj);
//    void callSuccessCallback(int count, JSValueRef obj[]);
//
//    void callErrorCallback();
//    void callErrorCallback(JSValueRef obj);
//    void callErrorCallback(int count, JSValueRef obj[]);

    void setActive(bool act);
    bool isActive() const;
    void setJson(std::shared_ptr<picojson::value> json);
    std::shared_ptr<picojson::value> getJson() const;

protected:
    std::shared_ptr<picojson::value> m_json;

private:
//    JSContextRef m_context;
//    MultiCallbackUserData* m_impl;
    bool m_is_act;
};

} // Common
//} // DeviceAPI

#endif //__TIZEN_COMMON_CALLBACK_USER_DATA_H__

