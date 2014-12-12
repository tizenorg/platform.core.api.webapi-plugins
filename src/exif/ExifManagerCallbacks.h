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

#ifndef __TIZEN_EXIF_EXIFMANAGER_CALLBACK_USER_DATA_H_
#define __TIZEN_EXIF_EXIFMANAGER_CALLBACK_USER_DATA_H_

#include <string>
#include <memory>
#include <CallbackUserData.h>
#include "ExifInformation.h"


namespace DeviceAPI {
namespace Exif {

///////////////// ExifManagerCallbackUserData ////////////////////////
class ExifManagerCallbackUserData: public Common::CallbackUserData {
public:
    ExifManagerCallbackUserData(JSContextRef globalCtx,
            std::string uri);
    virtual ~ExifManagerCallbackUserData();

    std::string& getUri();
    void setUri(const std::string& uri);

    void setError(const std::string& err_name,
            const std::string& err_message);
    bool isError() const;
    const std::string& getErrorName() const;
    const std::string& getErrorMessage() const;

private:
    std::string m_uri;

    bool m_is_error;
    std::string m_err_name;
    std::string m_err_message;
};


///////////////// ExifInfoCallbackData ////////////////////////
class ExifInfoCallbackData;
typedef std::shared_ptr<ExifInfoCallbackData> ExifInfoCallbackDataPtr;

struct ExifInfoCallbackDataHolder {
    ExifInfoCallbackDataPtr ptr;
};

class ExifInfoCallbackData : public ExifManagerCallbackUserData
{
public:
    ExifInfoCallbackData(JSContextRef globalCtx, std::string uri);
    virtual ~ExifInfoCallbackData();

    void setExifInfo(ExifInformationPtr info);
    ExifInformationPtr getExifInfo() const;

private:
    ExifInformationPtr m_exif_info;
};

///////////////// GetThumbnailCallbackUserData ////////////////////////
class GetThumbnailCallbackUserData: public ExifManagerCallbackUserData {
public:
    GetThumbnailCallbackUserData(JSContextRef global_ctx,
            std::string uri);
    virtual ~GetThumbnailCallbackUserData();

    std::string& getThumbnailUri();
    void setThumbnailUri(const std::string& uri);

private:
    std::string m_thumbnail_uri;
};

} // Exif
} // DeviceAPI

#endif // __TIZEN_EXIF_EXIFMANAGER_CALLBACK_USER_DATA_H_
