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

#include "ExifManagerCallbacks.h"

namespace DeviceAPI {
namespace Exif {

///////////////// ExifManagerCallbackUserData ////////////////////////
ExifManagerCallbackUserData::ExifManagerCallbackUserData(JSContextRef globalCtx,
        std::string uri) :
    CallbackUserData(globalCtx),
    m_uri(uri),
    m_is_error(false)
{
    LOGD("Entered");
}

ExifManagerCallbackUserData::~ExifManagerCallbackUserData()
{
}

void ExifManagerCallbackUserData::setError(const std::string& err_name,
const std::string& err_message)
{
    // keep only first error in chain
    if (!m_is_error) {
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;
    }
}

std::string& ExifManagerCallbackUserData::getUri()
{
    return m_uri;
}

void ExifManagerCallbackUserData::setUri(const std::string&  uri)
{
    m_uri = uri;
}

bool ExifManagerCallbackUserData::isError() const
{
    return m_is_error;
}

const std::string& ExifManagerCallbackUserData::getErrorName() const
{
    return m_err_name;
}

const std::string& ExifManagerCallbackUserData::getErrorMessage() const
{
    return m_err_message;
}


///////////////// ExifInfoCallbackData ////////////////////////
ExifInfoCallbackData::ExifInfoCallbackData(
        JSContextRef globalCtx, std::string uri):
    ExifManagerCallbackUserData(globalCtx, uri)
{
    LOGD("Entered");
}

ExifInfoCallbackData::~ExifInfoCallbackData()
{
    LOGD("Entered");
}

void ExifInfoCallbackData::setExifInfo(ExifInformationPtr info)
{
    m_exif_info = info;
}

ExifInformationPtr ExifInfoCallbackData::getExifInfo() const
{
    return m_exif_info;
}

///////////////// GetThumbnailCallbackUserData ////////////////////////
GetThumbnailCallbackUserData::GetThumbnailCallbackUserData(JSContextRef global_ctx,
        std::string uri) :
        ExifManagerCallbackUserData(global_ctx, uri)
{
}

GetThumbnailCallbackUserData::~GetThumbnailCallbackUserData()
{
}

std::string& GetThumbnailCallbackUserData::getThumbnailUri()
{
    return m_thumbnail_uri;
}

void GetThumbnailCallbackUserData::setThumbnailUri(const std::string& uri)
{
    m_thumbnail_uri = uri;
}

} // Exif
} // DeviceAPI
