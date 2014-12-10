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
// For simplicity this script is using ONLY first found email message service.
// If you want to change this please visit servicesListSuccessCB function.
//
// It is important to test mailservice which contains at least TWO mails containing
// attachments.
//

/**
 * @file        testLoadMessageAttachment_01.js
 */

var currentService = null;

function loadMsgAttSuccessCB(attachment) {
    console.log("loadMsgAttSuccessCB: received attachment:\n"
            + "        attachmentId: " + attachment.id + "\n"
            + "        filePath: " + attachment.filePath + "\n"
            + "        messageId: " + attachment.messageId + "\n"
            + "        mimeType: " + attachment.mimeType);
}

function loadMsgAttErrorCB(error) {
    console.log("loadMsgAttErrorCB: Cannot load message attachment:" + error.message);
}

function findMessagesSuccessCB(messages) {
    console.log("messagesFoundCB: found: " + messages.length + " messages");

    for (var msgIndex = 0; msgIndex < messages.length; ++msgIndex) {
        var message = messages[msgIndex];

        for(var attIndex = 0; attIndex < message.attachments.length; ++attIndex) {
            var attachment = message.attachments[attIndex];

            console.log("messagesFoundCB: requesting loadMessageAttachment for:\n"
                    + "        messageId: " + message.id + "\n"
                    + "        message: " + message.subject + "\n"
                    + "        attachmentId: " + attachment.id);

            currentService.loadMessageAttachment(attachment, loadMsgAttSuccessCB,
                         loadMsgAttErrorCB);
        }
    }
}

function findMessagesErrorCB(err) {
    console.log("servicesListSuccessCB: error:" + err);
}

function servicesListSuccessCB(services) {
    console.log("servicesListSuccessCB: received:" + services.length + " services");

    if(services.length > 0) {
        currentService = services[0];
        console.log("servicesListSuccessCB: testing service:" + currentService.name);
        console.log("servicesListSuccessCB: service id is:" + currentService.id);
        var filter = new tizen.AttributeFilter("hasAttachment", "EXACTLY", true);
        currentService.messageStorage.findMessages(filter, findMessagesSuccessCB,
            findMessagesErrorCB);
    } else {
        console.log("servicesListSuccessCB: ERROR: could not find any email service!");
    }
}

tizen.messaging.getMessageServices("messaging.email", servicesListSuccessCB);
