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
#include <Commons/plugin_initializer_def.h>
#include <Commons/WrtAccess/WrtAccess.h>
#include <GlobalContextManager.h>
#include <Logger.h>

#include "JSArchiveManager.h"
#include "JSArchiveFile.h"
#include "JSArchiveFileEntry.h"
#include "plugin_config.h"

namespace DeviceAPI {
namespace Archive {

using namespace DeviceAPI::Common;
using namespace WrtDeviceApis::Commons;

namespace Options{
class_definition_options_t ArchiveOptions = {
    JS_CLASS,
    CREATE_INSTANCE,
    NONE_NOTICE,
    USE_OVERLAYED,
    NULL,
    NULL,
    NULL
};
}

void on_widget_start_callback(int widget_id)
{
    LOGD("[TIZEN\\Archive] on_widget_start_callback (%d)", widget_id);
    try {
        WrtAccessSingleton::Instance().initialize(widget_id);
    }
    catch (...) {
        LOGE("WrtAccess initialization failed");
    }
}

void on_widget_stop_callback(int widget_id)
{
    LOGD("[TIZEN\\Archive ] on_widget_stop_callback (%d)", widget_id);
    try {
        WrtAccessSingleton::Instance().deinitialize(widget_id);
    }
    catch (...) {
        LOGE("WrtAccess deinitialization failed");
    }
}

void on_frame_load_callback(const void* context)
{
    LOGD("[Tizen\\Archive] on_frame_unload_callback ( %p )", context);
    GlobalContextManager::getInstance()->addGlobalContext(static_cast<JSContextRef>(context));
}

void on_frame_unload_callback(const void* context)
{
    LOGD("[Tizen\\Archive] on_frame_unload_callback ( %p )", context);
    GlobalContextManager::getInstance()->removeGlobalContext(static_cast<JSContextRef>(context));
}

PLUGIN_ON_WIDGET_START(on_widget_start_callback)
PLUGIN_ON_WIDGET_STOP(on_widget_stop_callback)

PLUGIN_ON_FRAME_LOAD(on_frame_load_callback)
PLUGIN_ON_FRAME_UNLOAD(on_frame_unload_callback)

PLUGIN_CLASS_MAP_BEGIN
PLUGIN_CLASS_MAP_ADD_CLASS(WRT_JS_EXTENSION_OBJECT_TIZEN,
        TIZEN_ARCHIVE_ARCHIVE_CLASS,
        (js_class_template_getter)JSArchiveManager::getClassRef,
        &Options::ArchiveOptions)
PLUGIN_CLASS_MAP_END

}
}

