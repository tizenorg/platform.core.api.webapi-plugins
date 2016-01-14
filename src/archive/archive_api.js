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
 
var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

function CommonFS() {};

CommonFS.cacheVirtualToReal = {};

function _initializeCache() {
    try {
        var result = native_.callSync('Archive_fetchVirtualRoots', {});

        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }

        result = native_.getResultObject(result);
        for (var i = 0; i < result.length; ++i) {
            CommonFS.cacheVirtualToReal[result[i].name] = {
                path: result[i].path
            };
        }
    } catch(e) {
        console.log("Exception while getting widget paths was thrown: " + e);
    }
}

_initializeCache();

CommonFS.toRealPath = function(aPath) {
    var _fileRealPath = '', _uriPrefix = 'file://', i;
    if (aPath.indexOf(_uriPrefix) === 0) {
        _fileRealPath = aPath.substr(_uriPrefix.length);
    } else if (aPath[0] != '/') {
        // virtual path$
        var _pathTokens = aPath.split('/');
        if (this.cacheVirtualToReal[_pathTokens[0]]
                && (this.cacheVirtualToReal[_pathTokens[0]].state === undefined || this.cacheVirtualToReal[_pathTokens[0]].state === 'MOUNTED')) {
            _fileRealPath = this.cacheVirtualToReal[_pathTokens[0]].path;
            for (i = 1; i < _pathTokens.length; ++i) {
                _fileRealPath += '/' + _pathTokens[i];
            }
        } else {
            _fileRealPath = aPath;
        }
    } else {
        _fileRealPath = aPath;
    }
    console.log("REAL PATH:" + _fileRealPath);
    if (_fileRealPath === "undefined" || _fileRealPath === "null") {
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
    }
    return _fileRealPath;
};

CommonFS.isVirtualPath = function(aPath) {
    var root = aPath.split("/")[0];

    return this.cacheVirtualToReal[root] != undefined;
};

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

function checkMode(mode, access)
{   if(access.indexOf(mode) == -1) {
        throw new WebAPIException(WebAPIException.INVALID_ACCESS_ERR, 'Not allowed operation');
    }
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

var onprogressCallbacks = {};
var ARCHIVE_ONPROGRESS_CALLBACK = 'ArchiveOnprogressCallback';

var ArchiveFileProgressCallback = function(msg) {
    if (native_.isFailure(msg)) {
        return;
    }

    var result = native_.getResultObject(msg);
    if ('onprogress' === result.action && onprogressCallbacks.hasOwnProperty(result.opId)) {
        onprogressCallbacks[result.opId](result.opId, result.value, result.filename);
    }
};

native_.addListener(ARCHIVE_ONPROGRESS_CALLBACK, ArchiveFileProgressCallback);

/**
 * The ArchiveFileEntry interface provides access to ArchiveFile member information and file data.
 * This constructor is for internal use only.
 * It should be prohibited to call this constructor by user.
 */
function ArchiveFileEntry(data, priv) {
    if (!(this instanceof ArchiveFileEntry)) {
        return new ArchiveFileEntry(data);
    }

    if (data === null || typeof data !== 'object') {
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
    }

    propertyFactory_(this, 'name',           data.name                      || "",    Property.E);
    propertyFactory_(this, 'size',           data.size                      || 0,     Property.E);
    propertyFactory_(this, 'compressedSize', data.compressedSize            || 0,     Property.E);
    propertyFactory_(this, 'modified',       new Date(data.modified * 1000) || null , Property.E);

    function getHandle() {
        if(priv.handle)
            return priv.handle;
        else throw new WebAPIException(WebAPIException.UNKNOWN_ERR, 'Archive is not opened');
    }

    /**
     * Extracts ArchiveFileEntry to the given location.
     */
    this.extract = function () {
        var args = validator_.validateArgs(arguments, [
            { name: "destinationDirectory", type: types_.FILE_REFERENCE },
            { name: "onsuccess", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "onprogress", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "stripName", type: types_.STRING, optional: true, nullable: true },
            { name: "overwrite", type: types_.BOOLEAN, optional: true, nullable: true }
        ]),
        opId = getNextOpId();

        if (!CommonFS.isVirtualPath(args.destinationDirectory)) {
            throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                    "Destination directory should be virtual path or file.");
        }

        var callArgs = {
            destinationDirectory : CommonFS.toRealPath(args.destinationDirectory),
            stripName : args.stripName || null,
            overwrite : args.overwrite || null,
            opId : opId,
            handle : getHandle(),
            name : this.name
        };

        var callback = function(result) {
            if (native_.isFailure(result)) {
                native_.callIfPossible(args.onerror, native_.getErrorObject(result));
            } else {
                var ret = native_.getResultObject(result);
                delete onprogressCallbacks[opId];
                native_.callIfPossible(args.onsuccess);
            }
        };

        if (args.onprogress) {
            onprogressCallbacks[opId] = args.onprogress;
        }

        var result = native_.call('ArchiveFileEntry_extract', callArgs, callback);
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }

        return opId;
    };
}


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
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
    }

    propertyFactory_(this, 'mode'            , data.mode             || "r", Property.E);
    propertyFactory_(this, 'decompressedSize', data.decompressedSize || 0,   Property.E);

    var priv ={ handle: data.handle };

    function getHandle() {
        if(priv.handle)
            return priv.handle;
        else throw new WebAPIException(WebAPIException.INVALID_STATE_ERR, 'ArchiveFile closed - operation not permitted');
    }

    /**
     * Adds a new member file to ArchiveFile.
     */
    this.add = function () {
        var args = validator_.validateArgs(arguments, [
            { name: "sourceFile", type: types_.FILE_REFERENCE },
            { name: "onsuccess", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "onprogress", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "options", type: types_.DICTIONARY, optional: true, nullable: true }
        ]),
        opId = getNextOpId();

        if (!CommonFS.isVirtualPath(args.sourceFile)) {
            throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                    "sourceFile should be virtual path or file.");
        }

        var optionsAttributes = ["destination", "stripSourceDirectory", "compressionLevel"],
            options = args.options || {};

        for(var i in optionsAttributes) {
            if (!options[optionsAttributes[i]]) {
                options[optionsAttributes[i]] = null;
            }
        }

        checkMode(this.mode, ["w","rw", "a"]);

        var callArgs = {
            sourceFile : CommonFS.toRealPath(args.sourceFile),
            options : options,
            opId : opId,
            handle : getHandle()
        };

        var callback = function(result) {
            if (native_.isFailure(result)) {
                native_.callIfPossible(args.onerror, native_.getErrorObject(result));
            } else {
                delete onprogressCallbacks[opId];
                native_.callIfPossible(args.onsuccess);
            }
        };

        if (args.onprogress) {
            onprogressCallbacks[opId] = args.onprogress;
        }

        var result = native_.call('ArchiveFile_add', callArgs, callback);
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }

        return opId;
    };

    /**
     * Extracts every file from this ArchiveFile to a given directory.
     */
    this.extractAll = function () {
        var args = validator_.validateArgs(arguments, [
            { name: "destinationDirectory", type: types_.FILE_REFERENCE },
            { name: "onsuccess", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "onprogress", type: types_.FUNCTION, optional: true, nullable: true },
            { name: "overwrite", type: types_.BOOLEAN, optional: true, nullable: true }
        ]),
        opId = getNextOpId();

        if (!CommonFS.isVirtualPath(args.destinationDirectory)) {
            throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                    "destinationDirectory should be virtual path or file.");
        }

        checkMode(this.mode, ["r","rw"]);

        var callArgs = {
            destinationDirectory : CommonFS.toRealPath(args.destinationDirectory),
            overwrite : args.overwrite || null,
            opId : opId,
            handle : getHandle()
        };

        var callback = function(result) {
            if (native_.isFailure(result)) {
                native_.callIfPossible(args.onerror, native_.getErrorObject(result));
            } else {
                delete onprogressCallbacks[opId];
                native_.callIfPossible(args.onsuccess);
            }
        };

        if (args.onprogress) {
            onprogressCallbacks[opId] = args.onprogress;
        }

        var result = native_.call('ArchiveFile_extractAll', callArgs, callback);
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }

        return opId;
    };

    /**
     * Retrieves information about the member files in ArchiveFile.
     */
    this.getEntries = function () {
        var args = validator_.validateArgs(arguments, [
            { name: "onsuccess", type: types_.FUNCTION },
            { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true }
        ]),
        opId = getNextOpId();

        checkMode(this.mode, ["r","rw"]);

        var callArgs = {
            opId : opId,
            handle : getHandle()
        };

        var callback = function(result) {
            if (native_.isFailure(result)) {
                native_.callIfPossible(args.onerror, native_.getErrorObject(result));
            } else {
                var entries = [];
                var ret = native_.getResultObject(result);
                ret.forEach(function (e) {
                    entries.push(new ArchiveFileEntry(e, priv));
                });
                args.onsuccess(entries);
          }
        };

        var result = native_.call('ArchiveFile_getEntries', callArgs, callback);
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }

        return opId;
    };

    /**
     * Retrieves information about ArchiveFileEntry with the specified name in ArchiveFile.
     */
    this.getEntryByName = function () {
        var args = validator_.validateArgs(arguments, [
            { name: "name", type: types_.STRING },
            { name: "onsuccess", type: types_.FUNCTION },
            { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true }
        ]),
        opId = getNextOpId();

        checkMode(this.mode, ["r","rw"]);

        var callArgs = {
            name : args.name,
            opId : opId,
            handle : getHandle()
        };

        var callback = function(result) {
            if (native_.isFailure(result)) {
                native_.callIfPossible(args.onerror, native_.getErrorObject(result));
            } else {
                args.onsuccess(new ArchiveFileEntry(native_.getResultObject(result), priv));
            }
        };

        var result = native_.call('ArchiveFile_getEntryByName', callArgs, callback);
        if (native_.isFailure(result)) {
            throw native_.getErrorObject(result);
        }

        return opId;
    };

    /**
     * Closes the ArchiveFile.
     */
    this.close = function () {
        var handle = priv.handle;
        if(priv.handle) {
            delete priv.handle;
            var result = native_.callSync('ArchiveFile_close', {'handle': handle});

            if (native_.isFailure(result)) {
                throw native_.getErrorObject(result);
            }
        }
    };
}


var ArchiveManager = function () {
};

/**
 * Opens the archive file. After this operation, it is possible to add or get files to and from the archive.
 */
ArchiveManager.prototype.open = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "file", type: types_.FILE_REFERENCE },
        { name: "mode", type: types_.ENUM, values: ["r", "rw", "w", "a"] },
        { name: "onsuccess", type: types_.FUNCTION },
        { name: "onerror", type: types_.FUNCTION, optional: true, nullable: true },
        { name: "options", type: types_.DICTIONARY, optional: true, nullable: true }
    ]),
    opId = getNextOpId();

    var optionsAttributes = ["overwrite"],
        options = args.options || {};

    for(var i in optionsAttributes) {
        if (!options[optionsAttributes[i]]) {
            options[optionsAttributes[i]] = null;
        }
    }

    if (!CommonFS.isVirtualPath(args.file)) {
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                "file should be virtual path or file.");
    }

    var callArgs = {
        file : CommonFS.toRealPath(args.file),
        mode : args.mode,
        options : options,
        opId : opId,
    };


    var callback = function(result) {
        if (native_.isFailure(result)) {
            native_.callIfPossible(args.onerror, native_.getErrorObject(result));
        } else {
            args.onsuccess(new ArchiveFile(native_.getResultObject(result)));
        }
    };

    var result = native_.call('ArchiveManager_open', callArgs, callback);
    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }

    return opId;
};

/**
 * Cancels an operation with the given identifier.
 */
ArchiveManager.prototype.abort = function () {
    var args = validator_.validateArgs(arguments, [
        { name: "opId", type: types_.LONG }
    ]);

    var result = native_.callSync('ArchiveManager_abort', {opId: args.opId});

    if (native_.isFailure(result)) {
        throw native_.getErrorObject(result);
    }
};

//Exports
exports = new ArchiveManager();
