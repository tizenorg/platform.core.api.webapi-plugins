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

#ifndef __TIZEN_EXIF_EXIFMANAGER_H_
#define __TIZEN_EXIF_EXIFMANAGER_H_

#include <SecurityAccessor.h>
#include <libexif/exif-data.h>
#include <glib.h>

#include "ExifManagerCallbacks.h"
#include "ExifInformation.h"

namespace DeviceAPI {
namespace Exif {

class ExifManager: public Common::SecurityAccessor
{
public:
    static ExifManager& getInstance();
    virtual ~ExifManager();

    void getExifInfo(ExifInfoCallbackData* cbdata);
    void saveExifInfo(ExifInfoCallbackData* cbdata);
    void getThumbnail(GetThumbnailCallbackUserData* cbdata);

private:
    ExifManager();
    ExifManager(const ExifManager&);
    ExifManager& operator=(const ExifManager&);

    static gboolean getExifInfoCompleted(void* data);
    static void* getExifInfoThread(void* data);

    static gboolean saveExifInfoCompleted(void* data);
    static void* saveExifInfoThread(void* data);

    static gboolean getThumbnailCompleted(void* data);
    static void* getThumbnailThread(void* data);
};

} // Exif
} // DeviceAPI

#endif // __TIZEN_EXIF_EXIFMANAGER_H_
