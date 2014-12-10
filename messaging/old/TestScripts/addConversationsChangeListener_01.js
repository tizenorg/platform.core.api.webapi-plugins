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

/**
 * @file        addConversationsChangeListener_01.js
 */

//
//
//  Attribute      | Attribute filter| Attribute range filter
//                 | supported       | supported
// ----------------+-----------------+------------------------
// id              | Yes             | No
// type            | Yes             | No
// timestamp       | No              | Yes
// messageCount    | Yes             | No
// unreadMessages  | Yes             | No
// preview         | Yes             | No
// subject         | No              | No
// isRead          | No              | No
// from            | Yes             | No
// to              | Yes             | No
// cc              | No              | No
// bcc             | No              | No
// lastMessageId   | No              | No


var prefferedEmailService = "a.jacak.testmail";

function containCaseInsensitive(string, searchString) {
    var lcString = string.toLowerCase();
    var lcSearchString = searchString.toLowerCase();
    return lcString.indexOf(lcSearchString)>=0;
}

var currentService = null;

function listConversations(prefix, conversations) {

    for (var i=0; i<conversations.length; i++) {
        var conversation = conversations[i];
        console.log(prefix + " CONVERSATION[" + i + "] " +
                " id: " + conversation.id +
                " type: " + conversation.type +
                " timestamp: " + conversation.timestamp +
                " messageCount: " + conversation.messageCount +
                " unreadMessages: " + conversation.unreadMessages +
                " preview: " + conversation.preview +
                " subject:" + conversation.subject +
                " isRead: " + conversation.isRead +
                " from: " + conversation.from +
                " lastMessageId: " + conversation.lastMessageId);
    }
}

var conversationChangeCB = {
    conversationsupdated: function(conversations) {
        console.log(conversations.length + " conversation(s) updated");
        listConversations("conversationsupdated", conversations);
    },
    conversationsadded: function(conversations) {
        console.log(conversations.length + " conversation(s) added");
        listConversations("conversationsadded", conversations);
    },
    conversationsremoved: function(conversations) {
        console.log(conversations.length + " conversation(s) removed");
        listConversations("conversationsremoved", conversations);
    }
};

function serviceSyncSuccessCB() {
    console.log("Synced!");
}

function serviceSyncFailCB() {
    console.log("Sync failed!");
}

function servicesListSuccessCB(services) {
    console.log("servicesListSuccessCB: received:" + services.length + " services");

    if(services.length > 0) {

        var chosenServiceIndex = 0;
        for(var i = 0; i < services.length; i++) {
            if(containCaseInsensitive(services[i].name, prefferedEmailService)) {
                chosenServiceIndex = i;
                console.log("servicesListSuccessCB: " +
                        "found service which name contains: " +
                        prefferedEmailService);
                break;
            }
        }
        currentService = services[chosenServiceIndex];
        console.log("servicesListSuccessCB: testing service:" + currentService.name);

        var filter = new tizen.AttributeFilter("preview", "CONTAINS", "Home");
        currentService.messageStorage.addConversationsChangeListener(
                conversationChangeCB, filter);

        currentService.sync(serviceSyncSuccessCB, serviceSyncFailCB, 30);
    } else {
        console.log("servicesListSuccessCB: ERROR: could not find any email service!");
    }
}

tizen.messaging.getMessageServices("messaging.email", servicesListSuccessCB);
