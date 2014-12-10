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

#include <map>
#include <utility>
#include <Commons/FunctionDefinition.h>
#include <Commons/FunctionDeclaration.h>
#include <Commons/Exception.h>
#include "plugin_config.h"

#define MESSAGING_FEATURE_API_READ "http://tizen.org/privilege/messaging.read"
#define MESSAGING_FEATURE_API_SEND "http://tizen.org/privilege/messaging.send"    // before sendMessage move to write privilege start
#define MESSAGING_FEATURE_API_WRITE "http://tizen.org/privilege/messaging.write"

#define MESSAGING_DEVICE_CAP_READ "messaging.read"
#define MESSAGING_DEVICE_CAP_SEND "messaging.send"    // before sendMessage move to write privilege start
#define MESSAGING_DEVICE_CAP_WRITE "messaging.write"

namespace DeviceAPI {
namespace Messaging {

using namespace WrtDeviceApis::Commons;

static WrtDeviceApis::Commons::FunctionMapping createMessagingFunctions();
static WrtDeviceApis::Commons::FunctionMapping MessagingFunctions = createMessagingFunctions();

#pragma GCC visibility push(default)

DEFINE_FUNCTION_GETTER(Messaging, MessagingFunctions);

#pragma GCC visibility pop

static WrtDeviceApis::Commons::FunctionMapping createMessagingFunctions()
{
     /**
     * Device capabilities
     */

    ACE_CREATE_DEVICE_CAP(DEVICE_CAP_MESSAGING_READ, MESSAGING_DEVICE_CAP_READ);
    ACE_CREATE_DEVICE_CAP(DEVICE_CAP_MESSAGING_SEND, MESSAGING_DEVICE_CAP_SEND);
    ACE_CREATE_DEVICE_CAP(DEVICE_CAP_MESSAGING_WRITE, MESSAGING_DEVICE_CAP_WRITE);

    ACE_CREATE_DEVICE_CAPS_LIST(EMPTY_DEVICE_LIST);

    ACE_CREATE_DEVICE_CAPS_LIST(DEVICE_LIST_MESSAGING_READ);
    ACE_ADD_DEVICE_CAP(DEVICE_LIST_MESSAGING_READ, DEVICE_CAP_MESSAGING_READ);

    ACE_CREATE_DEVICE_CAPS_LIST(DEVICE_LIST_MESSAGING_SEND);
    ACE_ADD_DEVICE_CAP(DEVICE_LIST_MESSAGING_SEND, DEVICE_CAP_MESSAGING_SEND);

    ACE_CREATE_DEVICE_CAPS_LIST(DEVICE_LIST_MESSAGING_WRITE);
    ACE_ADD_DEVICE_CAP(DEVICE_LIST_MESSAGING_WRITE, DEVICE_CAP_MESSAGING_WRITE);

    /**
    * API features
    */
    ACE_CREATE_FEATURE(FEATURE_MESSAGING_READ, MESSAGING_FEATURE_API_READ);
    ACE_CREATE_FEATURE(FEATURE_MESSAGING_SEND, MESSAGING_FEATURE_API_SEND);
    ACE_CREATE_FEATURE(FEATURE_MESSAGING_WRITE, MESSAGING_FEATURE_API_WRITE);

    ACE_CREATE_FEATURE_LIST(MESSAGING_FEATURES_MESSAGING_READ_SEND_WRITE);

    ACE_ADD_API_FEATURE(MESSAGING_FEATURES_MESSAGING_READ_SEND_WRITE, FEATURE_MESSAGING_READ);
    ACE_ADD_API_FEATURE(MESSAGING_FEATURES_MESSAGING_READ_SEND_WRITE, FEATURE_MESSAGING_SEND);
    ACE_ADD_API_FEATURE(MESSAGING_FEATURES_MESSAGING_READ_SEND_WRITE, FEATURE_MESSAGING_WRITE);

    ACE_CREATE_FEATURE_LIST(MESSAGING_FEATURES_MESSAGING_READ);
    ACE_ADD_API_FEATURE(MESSAGING_FEATURES_MESSAGING_READ, FEATURE_MESSAGING_READ);

    ACE_CREATE_FEATURE_LIST(MESSAGING_FEATURES_MESSAGING_SEND);
    ACE_ADD_API_FEATURE(MESSAGING_FEATURES_MESSAGING_SEND, FEATURE_MESSAGING_SEND);

    ACE_CREATE_FEATURE_LIST(MESSAGING_FEATURES_MESSAGING_WRITE);
    ACE_ADD_API_FEATURE(MESSAGING_FEATURES_MESSAGING_WRITE, FEATURE_MESSAGING_WRITE);

// before sendMessage move to write privilege start
    ACE_CREATE_FEATURE_LIST(MESSAGING_FEATURES_MESSAGING_SEND_WRITE);

    ACE_ADD_API_FEATURE(MESSAGING_FEATURES_MESSAGING_SEND_WRITE, FEATURE_MESSAGING_SEND);
    ACE_ADD_API_FEATURE(MESSAGING_FEATURES_MESSAGING_SEND_WRITE, FEATURE_MESSAGING_WRITE);
// before sendMessage move to write privilege end

    /**
     * Functions
     */

     FunctionMapping MessagingMapping;

    //get Message Service
    AceFunction getMessagingServiceFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_GET_MESSAGE_SERVICE,
            MESSAGING_FEATURES_MESSAGING_READ_SEND_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert(std::make_pair(
            MESSAGING_FUNCTION_API_GET_MESSAGE_SERVICE,
            getMessagingServiceFunc));

      /**  Read  **/
    AceFunction stopSyncFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_STOP_SYNC,
            MESSAGING_FEATURES_MESSAGING_READ,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_STOP_SYNC,
            stopSyncFunc));

    AceFunction findMessagesFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_FIND_MESSAGES,
            MESSAGING_FEATURES_MESSAGING_READ,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_FIND_MESSAGES,
            findMessagesFunc));

    AceFunction findConversationsFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_FIND_CONVERSATIONS,
            MESSAGING_FEATURES_MESSAGING_READ,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_FIND_CONVERSATIONS,
            findConversationsFunc));

    AceFunction findFoldersFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_FIND_FOLDERS,
            MESSAGING_FEATURES_MESSAGING_READ,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_FIND_FOLDERS,
            findFoldersFunc));

    AceFunction addMessagesChangeListenerFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_ADD_MESSAGES_CHANGE_LISTNER,
            MESSAGING_FEATURES_MESSAGING_READ,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_ADD_MESSAGES_CHANGE_LISTNER,
            addMessagesChangeListenerFunc));

    AceFunction addConversationsChangeListenerFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_ADD_CONVERSATIONS_CHANGE_LISTNER,
            MESSAGING_FEATURES_MESSAGING_READ,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_ADD_CONVERSATIONS_CHANGE_LISTNER,
            addConversationsChangeListenerFunc));

    AceFunction addFoldersChangeListenerFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_ADD_FOLDERS_CHANGE_LISTNER,
            MESSAGING_FEATURES_MESSAGING_READ,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_ADD_FOLDERS_CHANGE_LISTNER,
            addFoldersChangeListenerFunc));

    AceFunction removeChangeListenerFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_REMOVE_CHANGE_LISTENER,
            MESSAGING_FEATURES_MESSAGING_READ,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_REMOVE_CHANGE_LISTENER,
            removeChangeListenerFunc));

    /**  Send  **/

    // before sendMessage move to write privilege start
    AceFunction sendMessageFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_SEND_MESSAGE,
            MESSAGING_FEATURES_MESSAGING_SEND_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_SEND_MESSAGE,
            sendMessageFunc));
// before sendMessage move to write privilege end
      /**  Write  **/
    AceFunction loadMessageBodyFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_LOAD_MESSAGE_BODY,
            MESSAGING_FEATURES_MESSAGING_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_LOAD_MESSAGE_BODY,
            loadMessageBodyFunc));

    AceFunction loadMessageAttachmentFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_LOAD_MESSAGE_ATTACHMENT,
            MESSAGING_FEATURES_MESSAGING_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_LOAD_MESSAGE_ATTACHMENT,
            loadMessageAttachmentFunc));

    AceFunction syncFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_SYNC,
            MESSAGING_FEATURES_MESSAGING_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_SYNC,
            syncFunc));

    AceFunction syncFolderFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_SYNC_FOLDER,
            MESSAGING_FEATURES_MESSAGING_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_SYNC_FOLDER,
            syncFolderFunc));

    AceFunction addDraftMessageFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_ADD_DRAFT_MESSAGE,
            MESSAGING_FEATURES_MESSAGING_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_ADD_DRAFT_MESSAGE,
            addDraftMessageFunc));

    AceFunction removeMessagesFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_REMOVE_MESSAGES,
            MESSAGING_FEATURES_MESSAGING_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_REMOVE_MESSAGES,
            removeMessagesFunc));

    AceFunction removeConversationsFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_REMOVE_CONVERSATIONS,
            MESSAGING_FEATURES_MESSAGING_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_REMOVE_CONVERSATIONS,
            removeConversationsFunc));

    AceFunction updateMessagesFunc = ACE_CREATE_FUNCTION(
            FUNCTION_GET_MGR_SERVICE,
            MESSAGING_FUNCTION_API_UPDATE_MESSAGES,
            MESSAGING_FEATURES_MESSAGING_WRITE,
            EMPTY_DEVICE_LIST);

    MessagingMapping.insert( std::make_pair(
            MESSAGING_FUNCTION_API_UPDATE_MESSAGES,
            updateMessagesFunc));

    return MessagingMapping;
}

}
}

#undef MESSAGING_FEATURE_API
#undef MESSAGING_FEATURE_API_READ
#undef MESSAGING_FEATURE_API_SEND // before sendMessage move to write privilege start
#undef MESSAGING_FEATURE_API_WRITE
#undef MESSAGING_DEVICE_CAP_READ
#undef MESSAGING_DEVICE_CAP_SEND // before sendMessage move to write privilege start
#undef MESSAGING_DEVICE_CAP_WRITE

