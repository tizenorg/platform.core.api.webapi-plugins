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

#include "ExifManager.h"

#include <libexif/exif-loader.h>
#include <system/device.h>

#include <GlobalContextManager.h>
#include <JSUtil.h>
#include <JSWebAPIErrorFactory.h>
#include <Logger.h>

#include "ExifUtil.h"
#include "JSExifInformation.h"

using namespace DeviceAPI::Common;
using namespace DeviceAPI::Time;

namespace DeviceAPI {
namespace Exif {

ExifManager::ExifManager()
{
}

ExifManager::~ExifManager()
{
}

ExifManager&  ExifManager::getInstance()
{
    static ExifManager instance;
    return instance;
}

gboolean ExifManager::getExifInfoCompleted(void *data)
{
    LOGD("Entered");

    auto callback = static_cast<ExifInfoCallbackData*>(data);
    if (!callback) {
        LOGE("callback is null");
        return false;
    }

    JSContextRef context = callback->getContext();
    if (!Common::GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return false;
    }

    try {
        if (callback->isError()) {
            JSObjectRef error_obj = Common::JSWebAPIErrorFactory::makeErrorObject(
                    context,
                    callback->getErrorName(),
                    callback->getErrorMessage());
            callback->callErrorCallback(error_obj);
        }
        else {
            JSValueRef js_value = JSExifInformation::makeJSObject(context,
                    callback->getExifInfo());
            callback->callSuccessCallback(js_value);
        }
    }
    catch (const Common::BasePlatformException &error) {
        LOGE("%s (%s)", (error.getName()).c_str(), (error.getMessage()).c_str());
    }
    catch (...) {
        LOGE("Unknown error occured when calling getExifInfo callback");
    }

    delete callback;
    callback = NULL;
    return false;
}

void verifyThatURIIsFile(const std::string& uri,
        const std::string& req_permission)
{
    if(!ExifUtil::isValidAbsoluteURI(uri)) {
            LOGE("URI [%s] is not valid absolute URI", uri.c_str());
            throw InvalidValuesException("URI is not valid");
    }

    bool exists = false, granted = false;
    Filesystem::NodeType type = Filesystem::NT_FILE;
    ExifUtil::getURIInfo(uri, Filesystem::NT_FILE, req_permission, exists, type, granted);

    if(!exists) {
        LOGE("URI [%s] is not pointing at existing file", uri.c_str());
        throw NotFoundException("URI is not pointing at existing file");
    }

    if(type != Filesystem::NT_FILE) {
        LOGE("URI [%s] is not pointing at file", uri.c_str());
        throw NotFoundException("URI is not pointing at file");
    }

    if(!granted) {
        LOGE("URI [%s] is pointing at file which cannot be accesed (not: %s)",
                uri.c_str(), req_permission.c_str());
        throw IOException("URI is pointing at file which cannot be accessed"
                " - not enough permissions");
    }
}

void* ExifManager::getExifInfoThread(void* data)
{
    LOGD("Entered");
    ExifInfoCallbackData* callback = NULL;

    try {
        callback = static_cast<ExifInfoCallbackData*>(data);
        if (!callback) {
            LOGE("Callback is NULL!");
            return NULL;
        }

        verifyThatURIIsFile(callback->getUri(), "r");
        callback->setExifInfo(ExifInformation::loadFromURI(callback->getUri()));
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        callback->setError(err.getName(), err.getMessage());
    }
    catch (...) {
        LOGE("Get Exif from file failed");
        callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                "Get Exif from file failed");
    }

    if (0 == g_idle_add(getExifInfoCompleted, static_cast<void*>(callback))) {
        delete callback;
        callback = NULL;
        LOGE("g_idle addition failed");
    }

    return NULL;
}

void ExifManager::getExifInfo(ExifInfoCallbackData* cbdata)
{
    LOGD("Entered");
    if (!cbdata) {
        LOGE("cbdata is NULL");
        throw DeviceAPI::Common::UnknownException("NULL callback error");
    }

    pthread_t thread;
    if (pthread_create(&thread, NULL, getExifInfoThread,
                static_cast<void*>(cbdata))) {
        LOGE("Failed to create pthread for getExifInfo()");
        throw UnknownException("Could not load Exif from file");
    }

    if (pthread_detach(thread)) {
        LOGE("Failed to detach the pthread for getExifInfo()");
    }
}

gboolean ExifManager::saveExifInfoCompleted(void* data)
{
    LOGD("Entered");

    auto callback = static_cast<ExifInfoCallbackData*>(data);
    if (!callback) {
        LOGE("callback is null");
        return false;
    }

    JSContextRef context = callback->getContext();
    if (!Common::GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return false;
    }

    try {
        if (callback->isError()) {
            JSObjectRef error_obj = Common::JSWebAPIErrorFactory::makeErrorObject(
                    context,
                    callback->getErrorName(),
                    callback->getErrorMessage());
            callback->callErrorCallback(error_obj);
        }
        else {
            callback->callSuccessCallback();
        }
    }
    catch (const Common::BasePlatformException &error) {
        LOGE("%s (%s)", (error.getName()).c_str(), (error.getMessage()).c_str());
    }
    catch (...) {
        LOGE("Unknown error occured when calling saveExifInfo callback");
    }

    delete callback;
    callback = NULL;
    return false;
}

void* ExifManager::saveExifInfoThread(void* data)
{
    LOGD("Entered");
    ExifInfoCallbackData* callback = NULL;

    try {
        callback = static_cast<ExifInfoCallbackData*>(data);
        if (!callback) {
            LOGE("Callback is NULL!");
            return NULL;
        }

        verifyThatURIIsFile(callback->getUri(), "rw");

        const std::string file_path = ExifUtil::convertUriToPath(callback->getUri());
        callback->getExifInfo()->saveToFile(file_path);
    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        callback->setError(err.getName(), err.getMessage());
    }
    catch (...) {
        LOGE("Save Exif to file failed");
        callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                "Save Exif to file failed");
    }

    if (0 == g_idle_add(saveExifInfoCompleted, static_cast<void*>(callback))) {
        delete callback;
        callback = NULL;
        LOGE("g_idle addition failed");
    }

    return NULL;
}

void ExifManager::saveExifInfo(ExifInfoCallbackData* cbdata)
{
    LOGD("Entered");
    if (!cbdata) {
        LOGE("cbdata is NULL");
        throw DeviceAPI::Common::UnknownException("NULL callback error");
    }

    pthread_t thread;
    if (pthread_create(&thread, NULL, saveExifInfoThread,
                static_cast<void*>(cbdata))) {
        LOGE("Failed to create pthread for saveExifInfo()");
        throw UnknownException("Could not save Exif to file");
    }

    if (pthread_detach(thread)) {
        LOGE("Failed to detach the pthread for saveExifInfo()");
    }
}

gboolean ExifManager::getThumbnailCompleted(void* data)
{
    LOGD("Entered");
    auto callback = static_cast<GetThumbnailCallbackUserData*>(data);
    if (!callback) {
        LOGE("Callback is null");
        return false;
    }

    JSContextRef context = callback->getContext();
    if (!GlobalContextManager::getInstance()->isAliveGlobalContext(context)) {
        LOGE("context was closed");
        delete callback;
        callback = NULL;
        return false;
    }

    try {
        if (callback->isError()) {
            LOGD("Calling error callback");
            JSObjectRef errobj = JSWebAPIErrorFactory::makeErrorObject(context,
                    callback->getErrorName(),
                    callback->getErrorMessage());
            callback->callErrorCallback(errobj);
        }
        else {
            const std::string& thumbnail_uri = callback->getThumbnailUri();
            JSValueRef js_result = NULL;
            if (thumbnail_uri.empty()) {
                js_result = JSValueMakeNull(context);
            }
            else {
                js_result = JSUtil::toJSValueRef(context, thumbnail_uri);
            }
            callback->callSuccessCallback(js_result);
        }
    }
    catch (const BasePlatformException& err) {
        LOGE("Error while calling getThumbnail callback: %s (%s)",
                (err.getName()).c_str(), (err.getMessage()).c_str());
    }
    catch (...) {
        LOGE("Unknown error occured when calling getThumbnail callback");
    }

    delete callback;
    callback = NULL;

    return false;
}

void* ExifManager::getThumbnailThread(void* data)
{
    LOGD("Entered");
    GetThumbnailCallbackUserData* callback = NULL;
    ExifLoader* exif_loader = NULL;
    ExifData* exif_data = NULL;

    try {
        callback = static_cast<GetThumbnailCallbackUserData*>(data);
        if (!callback) {
            LOGE("Callback is NULL!");
            return NULL;
        }

        verifyThatURIIsFile(callback->getUri(), "r");

        exif_loader = exif_loader_new();
        if (!exif_loader) {
            LOGE("exif_loader_new failed -> returned NULL");
            throw UnknownException("Could not get thumbnail from Exif");
        }

        const std::string file_path = ExifUtil::convertUriToPath(callback->getUri());
        LOGD("Get thumbnail from Exif in file: [%s]", file_path.c_str());

        exif_loader_write_file(exif_loader, file_path.c_str());
        exif_data = exif_loader_get_data(exif_loader);
        if (!exif_data) {
            LOGE("exif_loader_get_data failed -> returned NULL");
            throw UnknownException("Could not get thumbnail from Exif");
        }

        if (exif_data->data && exif_data->size) {
            gchar* ch_uri = g_base64_encode(exif_data->data, exif_data->size);

            std::string ext = file_path.substr(file_path.find_last_of(".") + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            if ("jpg" == ext) {
                ext = "jpeg";
            }

            if ("jpeg" == ext || "png" == ext || "gif" == ext) {
                std::string uri ="data:image/"+ext+";base64," + ch_uri;
                callback->setThumbnailUri(uri);
            }
            else {
                LOGE("extension: %s is not valid (jpeg/jpg/png/gif is supported)");
                throw InvalidValuesException("Invalid file type");
            }
        }
        else {
            callback->setThumbnailUri(std::string());
        }

    }
    catch (const BasePlatformException& err) {
        LOGE("%s (%s)", (err.getName()).c_str(), (err.getMessage()).c_str());
        callback->setError(err.getName(), err.getMessage());
    }
    catch (...) {
        LOGE("Get thumbnail from Exif failed");
        callback->setError(JSWebAPIErrorFactory::UNKNOWN_ERROR,
                "Get thumbnail from Exif failed");
    }

    if (exif_loader) {
        exif_loader_unref(exif_loader);
        exif_loader = NULL;
    }

    if (exif_data) {
        exif_data_unref(exif_data);
        exif_data = NULL;
    }

    if (!g_idle_add(getThumbnailCompleted, static_cast<void*>(callback))) {
        LOGE("g_idle addition failed");
        delete callback;
        callback = NULL;
    }

    return NULL;
}

void ExifManager::getThumbnail(GetThumbnailCallbackUserData* cbdata)
{
    LOGD("Entered");
    if (!cbdata) {
        LOGE("cbdata is NULL");
        throw DeviceAPI::Common::UnknownException("NULL callback error");
    }

    pthread_t thread;
    if (pthread_create(&thread, NULL, getThumbnailThread,
                static_cast<void*>(cbdata))) {
        LOGE("Failed to create pthread for getThumbnail()");
        throw UnknownException("Could not get thumbnail from Exif");
    }

    if (pthread_detach(thread)) {
        LOGE("Failed to detach the pthread for getThumbnail()");
    }
}

} // Exif
} // DeviceAPI
