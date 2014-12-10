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
 * @file        addFoldersChangeListener_01.js
 */

/**
 *
 *  Attribute      | Attribute filter| Attribute range filter
 *                 | supported       | supported
 * ----------------+-----------------+------------------------
 *  id             | No              | No
 *  parentId       | No              | No
 *  serviceId      | Yes             | No
 *  contentType    | No              | No
 *  name           | No              | No
 *  path           | No              | No
 *  type           | No              | No
 *  synchronizable | No              | No
 */

var prefferedEmailService = "a.jacak.testmail";

function containCaseInsensitive(string, searchString) {
    var lcString = string.toLowerCase();
    var lcSearchString = searchString.toLowerCase();
    return lcString.indexOf(lcSearchString)>=0;
}

var currentService = null;

function listFolders(prefix, folders) {
    for (var i=0; i<folders.length; i++) {
        var folder = folders[i];
        console.log(prefix + " FOLDER[" + i + "] id: "
                + folder.id
                + " name:" + folder.name
                + " serviceid: " + folder.serviceId);
    }
}

var folderChangeCB = {
    foldersupdated: function(folders) {
        console.log(folders.length + " folder(s) updated");
        listFolders("foldersupdated",folders);
    },
    foldersadded: function(folders) {
        console.log(folders.length + " folder(s) added");
        listFolders("foldersadded",folders);
    },
    foldersremoved: function(folders) {
        console.log(folders.length + " folder(s) removed");
        listFolders("foldersremoved",folders);
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

        var filter = new tizen.AttributeFilter("serviceId", "EXACTLY", currentService.id);

        currentService.messageStorage.addFoldersChangeListener(folderChangeCB, filter);
        currentService.sync(serviceSyncSuccessCB, serviceSyncFailCB, 30);
    } else {
        console.log("servicesListSuccessCB: ERROR: could not find any email service!");
    }
}

tizen.messaging.getMessageServices("messaging.email", servicesListSuccessCB);