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

#include "common/callback_user_data.h"
//#include "common/multi_callback_user_data.h"
//#include <stdarg.h>

//namespace DeviceAPI {
namespace common {

namespace {
const char* CALLBACK_SUCCESS = "success";
const char* CALLBACK_ERROR = "error";
} //anonymous namespace

CallbackUserData::CallbackUserData():
//        m_context(global_ctx),
//        m_impl(new MultiCallbackUserData()),
        m_is_act(true)
{
}

CallbackUserData::~CallbackUserData()
{
//    delete m_impl;
}

//void CallbackUserData::setSuccessCallback(JSValueRef on_success)
//{
//    if(on_success && JSValueIsObject(m_context, on_success)) {
//        JSObjectRef success = JSValueToObject(m_context, on_success, NULL);
//        m_impl->setCallback(CALLBACK_SUCCESS, success);
//    }
//}
//
//void CallbackUserData::setErrorCallback(JSValueRef on_error)
//{
//    if(on_error && JSValueIsObject(m_context, on_error)) {
//        JSObjectRef error = JSValueToObject(m_context, on_error, NULL);
//        m_impl->setCallback(CALLBACK_ERROR, error);
//    }
//}
//
//void CallbackUserData::callSuccessCallback(int count, JSValueRef obj[])
//{
//    if(!m_is_act) {
//        return;
//    }
//
//    m_impl->invokeCallback(CALLBACK_SUCCESS, count, obj);
//}
//
//void CallbackUserData::callSuccessCallback(JSValueRef obj)
//{
//    JSValueRef args[1] = {obj};
//    callSuccessCallback(1, args);
//}
//
//void CallbackUserData::callSuccessCallback()
//{
//    callSuccessCallback(0, NULL);
//}
//
//void CallbackUserData::callErrorCallback(int count, JSValueRef obj[])
//{
//    if(!m_is_act) {
//        return;
//    }
//
//    m_impl->invokeCallback(CALLBACK_ERROR, count, obj);
//}
//
//void CallbackUserData::callErrorCallback(JSValueRef obj)
//{
//    JSValueRef args[1] = {obj};
//    callErrorCallback(1, args);
//}
//
//void CallbackUserData::callErrorCallback()
//{
//    callErrorCallback(0, NULL);
//}
//
//JSContextRef CallbackUserData::getContext()
//{
//    return m_context;
//}

void CallbackUserData::setActive(bool act)
{
    m_is_act = act;
}

bool CallbackUserData::isActive() const
{
    return m_is_act;
}

void CallbackUserData::setJson(std::shared_ptr<picojson::value> json)
{
    m_json = json;
}

std::shared_ptr<picojson::value> CallbackUserData::getJson() const
{
    return m_json;
}

} // Common
//} // DeviceAPI
