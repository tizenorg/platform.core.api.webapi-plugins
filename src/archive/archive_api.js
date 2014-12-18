// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var bridge = xwalk.utils.NativeBridge(extension, true);

function throwException_(err) {
    throw new tizen.WebAPIException(err.code, err.name, err.message);
}

/**
 * Returns new unique opId
 */
var getNextOpId = (function () {
    var opId = 0,
        incOpId = function () {
            return opId++;
        };
    return incOpId;
}());

var Property = {
    W: 1 << 0,   // WRITABLE
    E: 1 << 1,   // ENUMERABLE
    C: 1 << 2    // CONFIGURABLE
}

/**
 * Example usage:
 * function Messaging () {
 *     propertyFactory_(this, 'ids', [2,3,4], Property.W | Property.E | Property.C);
 *     propertyFactory_(this, 'name', 'Name', Property.E);
 *     propertyFactory_(this, 'age', 25, Property.W);
 *     propertyFactory_(this, 'something', 1);
 *     propertyFactory_(this, 'getSomething', Property.E, {get: function(){return 100;}});
 * }
 * Will produce:
 * var m = new Messaging();
 * {
 *     id: [2,3,4],
 *     name: 'Name',
 *     age: 25
 * }
 *
 * m.name = 'A brand new name';
 * console.log(m.name); // Name
 */
function propertyFactory_(that, name, value, flags, options) {
    flags = flags || 0;
    if (options === null || typeof options !== 'object') {
        options = {};
    }
    if (!(options.get) && !(options.set)) {
        options.value = value;
    }
    if ((flags & Property.W) != 0) { options.writable     = true; }
    if ((flags & Property.E) != 0) { options.enumerable   = true; }
    if ((flags & Property.C) != 0) { options.configurable = true; }
    Object.defineProperty(
        that,
        name,
        options
    );
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
 * The ArchiveFileEntry interface provides access to ArchiveFile member information and file data.
 * This constructor is for internal use only.
 * It should be prohibited to call this constructor by user.
 */
function ArchiveFileEntry(data) {
    if (!(this instanceof ArchiveFileEntry)) {
        return new ArchiveFileEntry(data);
    }

    if (data === null || typeof data !== 'object') {
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
    }

    propertyFactory_(this, 'name',           data.name           || "",    Property.E);
    propertyFactory_(this, 'size',           data.size           || 0,     Property.E);
    propertyFactory_(this, 'compressedSize', data.compressedSize || 0,     Property.E);
    propertyFactory_(this, 'modified',       data.modified       || null , Property.E);
    propertyFactory_(this, '_handle',        data.handle         || -1 );
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
    ]),
    opId = getNextOpId();

    bridge.async({
        cmd: 'ArchiveFileEntry_extract',
        args: {
            destDir: args.destDir,
            stripName: args.stripName,
            overwrite: args.overwrite,
            opId: opId,
            handle: this._handle
        }
    }).then({
        success: function () {
            if (args.onsuccess) {
                args.onsuccess.call(null);
            }
        },
        error: function (e) {
            if (args.onerror) {
                args.onerror.call(
                    null,
                    new tizen.WebAPIException(e.code, e.name, e.message)
                );
            }
        },
        progress: function (data) {
            if (args.onprogress) {
                args.onprogress.call(
                    null,
                    opId,
                    data.value,
                    data.filename
                );
            }
        }
    });

    return opId;
};

/**
 * The ArchiveManager interface provides methods for global operations related to ArchiveFile.
 */

/**
 * ArchiveFile interface provides access to member files of the archive file.
 * This constructor is for internal use only.
 * It should be prohibited to call this constructor by user.
 */
function ArchiveFile(data) {
    if (!(this instanceof ArchiveFile)) {
        return new ArchiveFile(data);
    }

    if (data === null || typeof data !== 'object') {
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
    }

    propertyFactory_(this, 'mode'            , data.mode             || "r", Property.E);
    propertyFactory_(this, 'decompressedSize', data.decompressedSize || 0,   Property.E);
    propertyFactory_(this, '_handle',          data.handle           || -1 );
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
    ]),
    opId = getNextOpId();

    bridge.async({
        cmd: 'ArchiveFile_add',
        args: {
            sourceFile: args.sourceFile,
            options: args.options || null,
            opId: opId,
            handle: this._handle
        }
    }).then({
        success: function () {
            if (args.onsuccess) {
                args.onsuccess.call(null);
            }
        },
        error: function (e) {
            if (args.onerror) {
                args.onerror.call(
                    null,
                    new tizen.WebAPIException(e.code, e.name, e.message)
                );
            }
        },
        progress: function (data) {
            if (args.onprogress) {
                args.onprogress.call(
                    null,
                    opId,
                    data.value,
                    data.filename
                );
            }
        }
    });

    return opId;
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
    ]),
    opId = getNextOpId();

    bridge.async({
        cmd: 'ArchiveFile_extractAll',
        args: {
            destDir: args.destDir,
            options: args.options || null,
            opId: opId,
            handle: this._handle
        }
    }).then({
        success: function () {
            if (args.onsuccess) {
                args.onsuccess.call(null);
            }
        },
        error: function (e) {
            if (args.onerror) {
                args.onerror.call(
                    null,
                    new tizen.WebAPIException(e.code, e.name, e.message)
                );
            }
        },
        progress: function (data) {
            if (args.onprogress) {
                args.onprogress.call(
                    null,
                    opId,
                    data.value,
                    data.filename
                );
            }
        }
    });

    return opId;
};

/**
 * Retrieves information about the member files in ArchiveFile.
 */
ArchiveFile.prototype.getEntries = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "onsuccess", type: types_.FUNCTION },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true }
    ]),
    opId = getNextOpId();

    bridge.async({
        cmd: 'ArchiveFile_getEntries',
        args: {
            opId: opId,
            handle: this._handle
        }
    }).then({
        success: function (data) {
            var entries = [];
            data.forEach(function (e) {
                entries.push(new ArchiveFileEntry(e));
            });
            args.onsuccess.call(null, entries);
        },
        error: function (e) {
            if (args.onerror) {
                args.onerror.call(
                    null,
                    new tizen.WebAPIException(e.code, e.name, e.message)
                );
            }
        }
    });

    return opId;
};

/**
 * Retrieves information about ArchiveFileEntry with the specified name in ArchiveFile.
 */
ArchiveFile.prototype.getEntryByName = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "name", type: types_.STRING },
        { name: "onsuccess", type: types_.FUNCTION },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true }
    ]),
    opId = getNextOpId();

    bridge.async({
        cmd: 'ArchiveFile_getEntryByName',
        args: {
            name: args.name,
            opId: opId,
            handle: this._handle
        }
    }).then({
        success: function (data) {
            args.onsuccess.call(null, new ArchiveFileEntry(data));
        },
        error: function (e) {
            if (args.onerror) {
                args.onerror.call(
                    null,
                    new tizen.WebAPIException(e.code, e.name, e.message)
                );
            }
        }
    });

    return opId;
};

/**
 * Closes the ArchiveFile.
 */
ArchiveFile.prototype.close = function () {
    bridge.sync({
        cmd: 'ArchiveFile_close',
        args: {
            handle: this._handle
        }
    });
};

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
    ]),
    opId = getNextOpId();

    bridge.async({
        cmd: 'ArchiveManager_open',
        args: {
            file: args.file,
            mode: args.mode,
            options: args.options || null,
            opId: opId
        }
    }).then({
        success: function (data) {
            args.successCallback.call(null, new ArchiveFile(data));
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new tizen.WebAPIException(e.code, e.name, e.message)
                );
            }
        }
    });

    return opId;
};

/**
 * Cancels an operation with the given identifier.
 */
ArchiveManager.prototype.abort = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "opId", type: types_.LONG }
    ]);

    bridge.sync({
        cmd: 'ArchiveManager_abort',
        args: {
            opId: args.opId
        }
    });
};

//Exports
exports = new ArchiveManager();
