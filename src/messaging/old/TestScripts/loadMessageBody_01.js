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
//
// This script registers messagesChangeListener and requests syncing first
// email message service.
//
// If there are new messages it requests loadBody for each message without
// downloaded body data.
//
// Please ensure that there is new email waiting to receive (send new mail before
// running this test).

/**
 * @file        testLoadMessageBody_01.js
 */

var currentService = null;

function loadMBodySuccessCB(message) {
    console.log("loadMBodySuccessCB: success!");
    console.log("loadMBodySuccessCB: message subject:" + message.subject);

    if(message.body.plainBody) {
        console.log("loadMBodySuccessCB: message plainBody:" + message.body.plainBody);
    } else if(message.body.htmlBody) {
        console.log("loadMBodySuccessCB: message htmlBody:" + message.body.htmlBody);
    } else {
        console.log("loadMBodySuccessCB:"
            + "ERROR: message do not contain plain nor htmlBody!");
    }
}
function loadMBodyErrorCB(error) {
    console.log("loadMBodyErrorCB: failes with error:"+error+"!");
}

var messageChangeCB = {
    messagesupdated: function(messages) {
        console.log(messages.length + " message(s) updated");
    },
    messagesadded: function(messages) {
       console.log("messagesadded: " + messages.length + " message(s) added");

        for (var i=0; i<messages.length; i++) {
            var message = messages[i];

            console.log("messagesadded: MSG[" + i + "] status: "
                     + message.messageStatus
                     + " subject:" + message.subject);

            if (message.body.loaded) {
                console.log("messagesadded: body for message[" + i
                         + "] is already loaded");
            } else {
                console.log("messagesadded: requesting loadMessageBody for message["
                         + i + "]");

                currentService.loadMessageBody(message, loadMBodySuccessCB,
                         loadMBodyErrorCB);
            }
        }
    },
    messagesremoved: function(messages) {
        console.log(messages.length + " message(s) removed");
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
        currentService = services[0];
        console.log("servicesListSuccessCB: testing service:" + currentService.name);

        currentService.messageStorage.addMessagesChangeListener(messageChangeCB);
        currentService.sync(serviceSyncSuccessCB, serviceSyncFailCB, 30);
    } else {
        console.log("servicesListSuccessCB: ERROR: could not find any email service!");
    }
}

tizen.messaging.getMessageServices("messaging.email", servicesListSuccessCB);

