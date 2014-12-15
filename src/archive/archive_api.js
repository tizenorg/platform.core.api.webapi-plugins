// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;

function throwException_(err) {
    throw new tizen.WebAPIException(err.code, err.name, err.message);
}

function callSync_(msg) {
    var ret = extension.internal.sendSyncMessage(JSON.stringify(msg));
    var obj = JSON.parse(ret);
    if (obj.error)
        throwException_(obj.error);
    return obj.result;
}

/**
 * Enumeration for the compression level.
 * @enum {string}
 */
var ArchiveCompressionLevel = {
    STORE: "STORE",
    FAST: "FAST",
    NORMAL: "NORMAL",
    BEST: "BEST"
};

/**
 * ArchiveFile interface provides access to member files of the archive file.
 */
function ArchiveFile() {
}

/**
 * Adds a new member file to ArchiveFile.
 */
ArchiveFile.prototype.add = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "sourceFile", type: types_.STRING }, //TODO: add FileReferece validation
        { name: "onsuccess", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "onprogress", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "options", type: types_.DICTIONARY, optional: true, nullable: true }
    ]);
};

/**
 * Extracts every file from this ArchiveFile to a given directory.
 */
ArchiveFile.prototype.extractAll = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "destDir", type: types_.STRING }, //TODO: add FileReferece validation
        { name: "onsuccess", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "onprogress", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "options", type: types_.DICTIONARY, optional: true, nullable: true }
    ]);
};

/**
 * Retrieves information about the member files in ArchiveFile.
 */
ArchiveFile.prototype.getEntries = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "onsuccess", type: types_.FUNCTION },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
    ]);
};

/**
 * Retrieves information about ArchiveFileEntry with the specified name in ArchiveFile.
 */
ArchiveFile.prototype.getEntryByName = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "name", type: types_.STRING },
        { name: "onsuccess", type: types_.FUNCTION },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
    ]);
};

/**
 * Closes the ArchiveFile.
 */
ArchiveFile.prototype.close = function () {
};

/**
 * The ArchiveFileEntry interface provides access to ArchiveFile member information and file data.
 */
function ArchiveFileEntry() {
}

/**
 * Extracts ArchiveFileEntry to the given location.
 */
ArchiveFileEntry.prototype.extract = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "destDir", type: types_.STRING }, //TODO: add FileReferece validation
        { name: "onsuccess", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "onprogress", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "stripName", type: types_.STRING, optional: true, nullable: true },
        { name: "overwrite", type: types_.BOOLEAN, optional: true, nullable: true }
    ]);
};

/**
 * The ArchiveManager interface provides methods for global operations related to ArchiveFile.
 */
function ArchiveManager() {
}

/**
 * Opens the archive file. After this operation, it is possible to add or get files to and from the archive.
 */
ArchiveManager.prototype.open = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "file", type: types_.STRING }, //TODO: add FileReferece validation
        { name: "mode", type: types_.ENUM, values: ["r", "rw", "w", "a"] },
        { name: "onsuccess", type: types_.FUNCTION },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "options", type: types_.DICTIONARY, optional: true, nullable: true }
    ]);

    callSync_({
        'cmd': 'ArchiveManager_open',
        'file': args.file,
        'onsuccess': args.onsuccess,
        'onerror': args.onerror,
        'options': args.options
    });
};

/**
 * Cancels an operation with the given identifier.
 */
ArchiveManager.prototype.abort = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "opId", type: types_.LONG }
    ]);
};

//Exports
exports = new ArchiveManager();
