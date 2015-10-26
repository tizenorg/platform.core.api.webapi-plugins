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

}//messaging
}//extension
