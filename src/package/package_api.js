// Package

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;


var callbackId = 0;
var callbacks = {};

extension.setMessageListener(function(json) {
    console.log("[Package][extension.setMessageListener] Enter");
    var result = JSON.parse(json);
    var callback = callbacks[result['callbackId']];
    callback(result);
});

function nextCallbackId() {
    return callbackId++;
}

function callNative(cmd, args) {
    console.log("[Package][callNative] Enter");
    
    var json = {'cmd':cmd, 'args':args};
    var argjson = JSON.stringify(json);
    console.log("[Package][callNative] argjson: " + argjson);
    var resultString = extension.internal.sendSyncMessage(argjson)
    console.log("[Package][callNative] resultString: " + resultString);
    var result = JSON.parse(resultString);

    if (typeof result !== 'object') {
        throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR);
    }

    if (result['status'] == 'success') {
        if(result['result']) {
            return result['result'];
        }
        return true;
    } else if (result['status'] == 'error') {
        var err = result['error'];
        if(err) {
            throw new tizen.WebAPIException(err.name, err.message);
        }
        return false;
    }
}


function callNativeWithCallback(cmd, args, callback) {
    console.log("[Package][callNativeWithCallback] Enter");
    
    if(callback) {
        var id = nextCallbackId();
        args['callbackId'] = id;
        callbacks[id] = callback;
        console.log("[Package][callNativeWithCallback] callbackId: " + id);
    }

    return callNative(cmd, args);
}

function SetReadOnlyProperty(obj, n, v){
    if(arguments.length > 2)
        Object.defineProperty(obj, n, {value:v, writable: false});
    else
        Object.defineProperty(obj, n, {writable: false});
}

function PackageInformation(obj) {
    var lastModified = obj.lastModified;
    obj.lastModified = new Date(lastModified);
    SetReadOnlyProperty(obj, 'id'); // read only property 
    SetReadOnlyProperty(obj, 'name'); // read only property 
    SetReadOnlyProperty(obj, 'iconPath'); // read only property 
    SetReadOnlyProperty(obj, 'version'); // read only property 
    SetReadOnlyProperty(obj, 'totalSize'); // read only property 
    SetReadOnlyProperty(obj, 'dataSize'); // read only property 
    SetReadOnlyProperty(obj, 'lastModified'); // read only property 
    SetReadOnlyProperty(obj, 'author'); // read only property 
    SetReadOnlyProperty(obj, 'description'); // read only property 
    SetReadOnlyProperty(obj, 'appIds'); // read only property

    return obj;
}

function PackageManager() {
    // constructor of PackageManager
}


PackageManager.prototype.install = function(packageFileURI, progressCallback) {
    console.log("[Package][install] Enter");

    var args = validator_.validateArgs(arguments, [
        {'name' : 'packageFileURI', 'type': types_.STRING},  
        {'name' : 'progressCallback', 'type': types_.LISTENER, 'values' : ['onprogress', 'oncomplete']},  
        {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
    ]);

    var nativeParam = {
            'packageFileURI': args.packageFileURI
    };
    console.log("[Package][install] packageFileURI: [" + nativeParam.packageFileURI + "]");

    try {
        var syncResult = callNativeWithCallback('PackageManager_install', nativeParam, function(result) {
            console.log("[Package][installCB] Enter");        
            console.log("[Package][installCB] status: " + result.status);

            if (result.status == 'progress') {
                console.log("[Package][installCB] Call onprogress");
                args.progressCallback.onprogress(result.id, result.progress);
            } else if (result.status == 'complete') {
                console.log("[Package][installCB] Call oncomplete");
                args.progressCallback.oncomplete(result.id);
            } else if (result.status == 'error') {
                console.log("[Package][installCB] Call errorCallback");

	        var err = result['error'];
                if(err) {
                    args.errorCallback(new tizen.WebAPIError(err.name, err.message));
                    return;
                }
            }
            
            if(result.status == 'complete' || result.status == 'error') {
                console.log("[Package][installCB] Delete callback");  
                delete callbacks[result['callbackId']];
            }
        });
    } catch(e) {
        throw e;
    }

}

PackageManager.prototype.uninstall = function(id, progressCallback) {
    console.log("[Package][uninstall] Enter");
    
    var args = validator_.validateArgs(arguments, [
        {'name' : 'id', 'type': types_.STRING},  
        {'name' : 'progressCallback', 'type': types_.LISTENER, 'values' : ['onprogress', 'oncomplete']},  
        {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true}
    ]);

    var nativeParam = {
        'id': args.id
    };
    console.log("[Package][uninstall] id: [" + nativeParam.id + "]");

    try {
        var syncResult = callNativeWithCallback('PackageManager_uninstall', nativeParam, function(result) {
            console.log("[Package][uninstallCB] Enter");        
            console.log("[Package][uninstallCB] status: " + result.status);

            if (result.status == 'progress') {
                console.log("[Package][uninstallCB] Call onprogress");
                args.progressCallback.onprogress(result.id, result.progress);
            } else if (result.status == 'complete') {
                console.log("[Package][uninstallCB] Call oncomplete");
                args.progressCallback.oncomplete(result.id);
            } else if (result.status == 'error') {
                console.log("[Package][uninstallCB] Call errorCallback");

	        var err = result['error'];
                if(err) {
                    args.errorCallback(new tizen.WebAPIError(err.name, err.message));
                    return;
                }
            }

            if(result.status == 'complete' || result.status == 'error') {
                console.log("[Package][uninstallCB] Delete callback");  
                delete callbacks[result['callbackId']];
            }
        });
    } catch(e) {
        throw e;
    }

}

PackageManager.prototype.getPackagesInfo = function(successCallback, errorCallback) {
    console.log("[Package][getPackagesInfo] Enter");

    var args = validator_.validateArgs(arguments, [
        {'name' : 'successCallback', 'type': types_.FUNCTION},  
        {'name' : 'errorCallback', 'type': types_.FUNCTION, optional : true, nullable : true} 
    ]);

    var nativeParam = {
    };

    try {
        var syncMsg = callNativeWithCallback('PackageManager_getPackagesInfo', nativeParam, function(result) {
            console.log("[Package][getPackagesInfoCB] Enter");        
            console.log("[Package][getPackagesInfoCB] status: " + result.status);
            
            if (result.status == 'success') {         
                for(var i = 0; i < result.informationArray.length; i++) {
                    result.informationArray[i] = PackageInformation(result.informationArray[i])
                }
                
                console.log("[Package][getPackagesInfo] Call successCallback: " + result.informationArray.length);                
                args.successCallback(result.informationArray);
            } else if(result.status == 'error') {
                console.log("[Package][getPackagesInfo] Call errorCallback");

                var err = result['error'];
                if(err) {
                    args.errorCallback(new tizen.WebAPIError(err.name, err.message));
                    return;
                }
            }

            console.log("[Package][getPackagesInfoCB] Delete callback");
            delete callbacks[result['callbackId']];
        });
    } catch(e) {
        throw e;
    }

}

PackageManager.prototype.getPackageInfo = function() {
    console.log("[Package][getPackageInfo] Enter");
    
    var args = validator_.validateArgs(arguments, [
        {'name' : 'id', 'type': types_.STRING, optional : true, nullable : true} 
    ]);

    var nativeParam = {
    };
    
    if (args['id']) {
        nativeParam['id'] = args.id;
    }
    
    try {
        var resultObject = callNative('PackageManager_getPackageInfo', nativeParam);
        return PackageInformation(resultObject);
    } catch(e) {
        throw e;
    }
}

PackageManager.prototype.setPackageInfoEventListener = function(eventCallback) {
    console.log("[Package][setPackageInfoEventListener] Enter");
    
    var args = validator_.validateArgs(arguments, [
        {'name' : 'eventCallback', 'type': types_.LISTENER, 'values' : ['oninstalled', 'onupdated', 'onuninstalled']} 
    ]);

    var nativeParam = {
    };
    
    try {
        var syncResult = callNativeWithCallback('PackageManager_setPackageInfoEventListener', nativeParam, function(result) {
            console.log("[Package][PackageInfoEventListener] Enter");        
            console.log("[Package][PackageInfoEventListener] status: " + result.status);

            if (result.status == 'installed') {
                args.eventCallback.oninstalled(PackageInformation(result.info));
            }
            if (result.status == 'updated') {
                args.eventCallback.onupdated(PackageInformation(result.info));
            }
            if (result.status == 'uninstalled') {
                args.eventCallback.onuninstalled(result.id);
            }
        });
    } catch(e) {
        throw e;
    }
}

PackageManager.prototype.unsetPackageInfoEventListener = function() {
    console.log("[Package][unsetPackageInfoEventListener] Enter");

    var nativeParam = {
    };

    try {
        var syncResult = callNative('PackageManager_unsetPackageInfoEventListener', nativeParam);
        if(typeof syncResult != 'boolean') {
            console.log("[Package][unsetPackageInfoEventListenerCB] Delete callback " + syncResult);
            delete callbacks[syncResult];
        }
    } catch(e) {
        throw e;
    }
}

exports = new PackageManager();

