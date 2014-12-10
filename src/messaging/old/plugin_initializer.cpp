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

#include <Commons/plugin_initializer_def.h>
#include <Commons/WrtAccess/WrtAccess.h>

#include <GlobalContextManager.h>
#include <Logger.h>
#include <Security.h>
#include <TimeTracer.h>

#include "JSMessageAttachment.h"
#include "JSMessage.h"
#include "JSMessagingManager.h"
#include "MessagingManager.h"

#include "plugin_config.h"

using namespace WrtDeviceApis;
using namespace WrtDeviceApis::Commons;
using namespace DeviceAPI::Common;

namespace DeviceAPI {
namespace Messaging {

AceSecurityStatus messagingAceCheckAccessFunction(const char* functionName)
{
    return MESSAGING_CHECK_ACCESS(functionName);
}

DEFINE_GLOBAL_SECURITY_ACCESSOR(gSecurityAccessor);

DEFINE_SECURITY_ACCESSOR_SETTER(AceCheckerMessagingSetter, MessagingManager, gSecurityAccessor);

class_definition_options_t MessagingOptions =
{
    JS_CLASS,
    CREATE_INSTANCE,
    ALWAYS_NOTICE,
    USE_OVERLAYED, //ignored
    AceCheckerMessagingSetter,
    NULL,
    NULL
};

class_definition_options_t MessagingInterfaceOptions = {
    JS_INTERFACE,
    CREATE_INSTANCE,
    NONE_NOTICE,
    USE_OVERLAYED, //ignored
    NULL, // JSWidget::acquireGlobalContext,
    NULL,
    NULL
};

void on_widget_start_callback(int widgetId) {
    LOGD("[Tizen\\MessagingManager] on_widget_start_callback (%d)", widgetId);
    TIME_TRACER_INIT();
    try {
        WrtAccessSingleton::Instance().initialize(widgetId);
    } catch (...) {
        LOGE("WrtAccess initialization failed");
    }
    INITAILIZE_GLOBAL_SECURITY_ACCESSOR(gSecurityAccessor, messagingAceCheckAccessFunction);
}

void on_widget_stop_callback(int widgetId) {
    LOGD("[Tizen\\MessagingManager] on_widget_stop_callback (%d)", widgetId);
    std::string name = "MessagingManager";
    TIME_TRACER_EXPORT_REPORT_TO(TIME_TRACER_EXPORT_FILE, const_cast<char*>(name.c_str()));
    TIME_TRACER_RELEASE();
    try {
        WrtAccessSingleton::Instance().deinitialize(widgetId);
    } catch (...) {
        LOGE("WrtAccess deinitialization failed");
    }
    FINALIZE_GLOBAL_SECURITY_ACCESSOR(gSecurityAccessor);
}

void on_frame_load_callback(const void * context)
{
    LOGD("[Tizen\\MessagingManager] on_frame_load_callback (%p)", context);
    DeviceAPI::Common::GlobalContextManager::getInstance()->
            addGlobalContext(static_cast<JSContextRef>(context));
}

void on_frame_unload_callback(const void * context)
{
    LOGD("[Tizen\\MessagingManager] on_frame_unload_callback (%p)", context);
    DeviceAPI::Common::GlobalContextManager::getInstance()->
            removeGlobalContext(static_cast<JSContextRef>(context));
}


PLUGIN_ON_WIDGET_START(on_widget_start_callback)
PLUGIN_ON_WIDGET_STOP(on_widget_stop_callback)

PLUGIN_ON_FRAME_LOAD(on_frame_load_callback)
PLUGIN_ON_FRAME_UNLOAD(on_frame_unload_callback)

PLUGIN_CLASS_MAP_BEGIN
PLUGIN_CLASS_MAP_ADD_CLASS(WRT_JS_EXTENSION_OBJECT_TIZEN,
        TIZEN_MESSAGING_MESSAGING_MANAGER_CLASS,
        (js_class_template_getter)DeviceAPI::Messaging::JSMessagingManager::getClassRef,
        &MessagingOptions)
PLUGIN_CLASS_MAP_ADD_INTERFACE(WRT_JS_EXTENSION_OBJECT_TIZEN,
        TIZEN_MESSAGING_MESSAGE_INTERFACE,
        (js_class_template_getter)DeviceAPI::Messaging::JSMessage::getClassRef,
        (js_class_constructor_cb_t)DeviceAPI::Messaging::JSMessage::constructor,
        &MessagingInterfaceOptions)
PLUGIN_CLASS_MAP_ADD_INTERFACE(WRT_JS_EXTENSION_OBJECT_TIZEN,
        TIZEN_MESSAGING_MESSAGE_ATTACHMENT_INTERFACE,
        (js_class_template_getter)DeviceAPI::Messaging::JSMessageAttachment::getClassRef,
        (js_class_constructor_cb_t)DeviceAPI::Messaging::JSMessageAttachment::constructor,
        &MessagingInterfaceOptions)
PLUGIN_CLASS_MAP_END

} // Messaging
} // DeviceAPI
