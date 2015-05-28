/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
 
#include "folders_callback_data.h"

namespace extension {
namespace messaging {


FoldersCallbackData::FoldersCallbackData(PostQueue& queue):
        m_filter(),
        m_is_error(false),
        queue_(queue)
{
    LoggerD("Entered");
}

FoldersCallbackData::FoldersCallbackData(long cid, PostQueue& queue, bool keep):
        m_filter(),
        m_is_error(false),
        queue_(queue)
{
    LoggerD("Entered");
    auto json = std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    picojson::object& o = json->get<picojson::object>();
    o[JSON_CALLBACK_ID] = picojson::value(static_cast<double>(cid));
    o[JSON_CALLBACK_KEEP] = picojson::value(keep);
    setJson(json);
}

FoldersCallbackData::~FoldersCallbackData() {
    LoggerD("Entered");
}

void FoldersCallbackData::addFolder(std::shared_ptr<MessageFolder> folder)
{
    m_folders.push_back(folder);
}

const std::vector<std::shared_ptr<MessageFolder>>& FoldersCallbackData::getFolders() const
{
    return m_folders;
}

void FoldersCallbackData::setFilter(tizen::AbstractFilterPtr filter)
{
    m_filter = filter;
}

tizen::AbstractFilterPtr FoldersCallbackData::getFilter() const
{
    return m_filter;
}

void FoldersCallbackData::setError(const std::string& err_name,
        const std::string& err_message)
{
    LoggerD("Entered");
    // keep only first error in chain
    if (!m_is_error) {
        LoggerD("Error has not been set yet");
        m_is_error = true;
        m_err_name = err_name;
        m_err_message = err_message;

        picojson::object& obj = m_json->get<picojson::object>();
        obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);

        auto obj_error = picojson::object();
        obj_error[JSON_ERROR_NAME] = picojson::value(err_name);
        obj_error[JSON_ERROR_MESSAGE] = picojson::value(err_message);
        obj[JSON_DATA] = picojson::value(obj_error);
    }
}

void FoldersCallbackData::SetError(const common::PlatformResult& error)
{
    LoggerD("Entered");
  // keep only first error in chain
  if (!m_is_error) {
      LoggerD("Error has not been set yet");
    m_is_error = true;
    picojson::object& obj = m_json->get<picojson::object>();
    obj[JSON_ACTION] = picojson::value(JSON_CALLBACK_ERROR);
    auto obj_data = picojson::object();
    obj_data[JSON_ERROR_CODE] = picojson::value(static_cast<double>(error.error_code()));
    obj_data[JSON_ERROR_MESSAGE] = picojson::value(error.message());
    obj[JSON_DATA] = picojson::value(obj_data);
  }
}

bool FoldersCallbackData::isError() const
{
    return m_is_error;
}

std::string FoldersCallbackData::getErrorName() const
{
    return m_err_name;
}

std::string FoldersCallbackData::getErrorMessage() const
{
    return m_err_message;
}

}//messaging
}//extension
