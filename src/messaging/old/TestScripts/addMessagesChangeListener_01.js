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
// Please send two emails with subject containing "Alaska"
//  1. subject "Alaska Good" and body containing "hack"
//  2. subject "Alaska Bad" and body NOT containing "hack"
//
// Only 1. should printed in JS console.

/**
 * @file        addMessagesChangeListener_01.js
 */

//https://developer.tizen.org/help/index.jsp?topic=%2Forg.tizen.gettingstarted%2Fhtml%2Ftizen_overview%2Fapplication_filtering.htm

var prefferedEmailService = "a.jacak.testmail";

function containCaseInsensitive(string, searchString) {
    var lcString = string.toLowerCase();
    var lcSearchString = searchString.toLowerCase();
    return lcString.indexOf(lcSearchString)>=0;
}

var currentService = null;

function listMessages(prefix, messages) {
    for (var i=0; i<messages.length; i++) {
        var message = messages[i];
        console.log(prefix + " MSG[" + i + "] status: "
                    + message.messageStatus
                    + " subject:" + message.subject);
    }
}

var messageChangeCB = {
    messagesupdated: function(messages) {
        console.log(messages.length + " message(s) updated");
        listMessages("messagesupdated",messages);
    },
    messagesadded: function(messages) {
       console.log("messagesadded: " + messages.length + " message(s) added");
       listMessages("messagesadded",messages);

    },
    messagesremoved: function(messages) {
        console.log(messages.length + " message(s) removed");
        listMessages("messagesremoved",messages);
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

        var subjectFilter1 = new tizen.AttributeFilter("subject", "CONTAINS", "Alaska");
        var subjectFilter2 = new tizen.AttributeFilter("subject", "CONTAINS", "Alabama");
        var subsfilter = new tizen.CompositeFilter("UNION",
                [subjectFilter1, subjectFilter2]);

        var pbodyFilter = new tizen.AttributeFilter("body.plainBody", "CONTAINS", "hack");
        var filter = new tizen.CompositeFilter("INTERSECTION", [subsfilter, pbodyFilter]);

        currentService.messageStorage.addMessagesChangeListener(messageChangeCB, filter);
        currentService.sync(serviceSyncSuccessCB, serviceSyncFailCB, 30);
    } else {
        console.log("servicesListSuccessCB: ERROR: could not find any email service!");
    }
}

tizen.messaging.getMessageServices("messaging.email", servicesListSuccessCB);
