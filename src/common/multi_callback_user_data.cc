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

#include "common/multi_callback_user_data.h"
//#include "GlobalContextManager.h"
//#include "PlatformException.h"
//#include "WebKitProxy.h"
//#include "JSUtil.h"
#include "common/logger.h"

//using namespace std;

//namespace DeviceAPI {
namespace common {

MultiCallbackUserData::MultiCallbackUserData() //:
//        m_context(global_ctx),
//        m_object(NULL)
{
}

//MultiCallbackUserData::MultiCallbackUserData(JSContextRef global_ctx,
//        JSObjectRef object) :
//    m_context(global_ctx),
//    m_object(object)
//{
//    if(m_object) {
//        JSValueProtect(m_context, m_object);
//    }
//}
//
//MultiCallbackUserData::~MultiCallbackUserData()
//{
//    if(!GlobalContextManager::getInstance()->isAliveGlobalContext(m_context)) {
//        //Remove Callback functions in Native Map
//        CallbackMapT::iterator itr;
//        for(itr = m_callbacks.begin(); itr != m_callbacks.end(); ++itr) {
//            if(itr->second) {
//                JSValueUnprotect(m_context, itr->second);
//            }
//        }
//
//        //Remove Callback Object
//        if(m_object) {
//            JSValueUnprotect(m_context, m_object);
//        }
//    }
//}
//
//void MultiCallbackUserData::setCallback(const string &key, JSObjectRef callback)
//{
//    // Callback Object Case
//    if(m_object) {
//        JSUtil::setProperty(m_context, m_object, key.c_str(), callback,
//                kJSPropertyAttributeNone);
//        return;
//    }
//
//    // Callback function Case
//    CallbackMapT::iterator itr = m_callbacks.find(key);
//    if(itr != m_callbacks.end() && itr->second) {
//        JSValueUnprotect(m_context, itr->second);
//    }
//
//    if(callback) {
//        JSValueProtect(m_context, callback);
//    }
//
//    m_callbacks[key] = callback;
//}
//
//
//void MultiCallbackUserData::invokeCallback(const std::string &key, int count,
//        JSValueRef obj[])
//{
//    if(!GlobalContextManager::getInstance()->isAliveGlobalContext(m_context)) {
//        LOGE("context was closed");
//        return;
//    }
//
//    // Callback Object case
//    if(m_object) {
//        try {
//            // Getting callback value
//            JSValueRef callbackValue = JSUtil::getProperty(m_context, m_object,
//                     key.c_str());
//
//            // Testing existing
//            if(JSValueIsUndefined(m_context, callbackValue)) {
//                LOGE("There is no such callback: [%s]", key.c_str());
//                return;
//            }
//
//            JSObjectRef callbackObject = JSUtil::JSValueToObject(m_context, callbackValue);
//
//            // Testing type validation
//            if(!JSObjectIsFunction(m_context, callbackObject)) {
//                LOGE("[%s] is not function", key.c_str());
//                return;
//            }
//
//            JSValueRef exception = NULL;
//            JSObjectCallAsFunction(m_context, callbackObject, NULL, count, obj,
//                     &exception);
//
//            if(exception){
//                WebKitProxy::reportException(m_context, exception);
//            }
//            // check Exception in function call
//            if(exception != NULL) {
//                throw UnknownException(m_context, exception);
//            }
//        } catch(const BasePlatformException& err) {
//            LOGE("Error in Callback invoke - %s:%s", err.getName().c_str(),
//                    err.getMessage().c_str());
//        }
//        return;
//    }
//
//    // Callback function case
//    CallbackMapT::iterator itr = m_callbacks.find(key);
//    if(itr == m_callbacks.end()) {
//        LOGE("There is no such callback: [%s]", key.c_str());
//        return;
//    }
//
//    if(itr->second) {
//        JSValueRef exception = NULL;
//        JSObjectCallAsFunction(m_context, itr->second , NULL, count, obj, &exception);
//        if(exception)
//            WebKitProxy::reportException(m_context, exception);
//    }
//    else {
//        LOGE("The callback: [%s] is NULL", key.c_str());
//    }
//}
//
//void MultiCallbackUserData::invokeCallback(const std::string &key, JSValueRef obj)
//{
//    JSValueRef args[1] = {obj};
//    invokeCallback(key, 1, args);
//}
//
//void MultiCallbackUserData::invokeCallback(const std::string &key)
//{
//    invokeCallback(key, 0, NULL);
//}
//
//JSContextRef MultiCallbackUserData::getContext() const
//{
//    return m_context;
//}

} // Common
//} // DeviceAPI
