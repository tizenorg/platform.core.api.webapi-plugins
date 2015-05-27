// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator = xwalk.utils.validator;
var type = xwalk.utils.type;
var native = new xwalk.utils.NativeManager(extension);
var converter_ = xwalk.utils.converter;

var ON_PRESSED_MEDIA_KEY_EVENT_CALLBACK = 'onPressedMediaKeyEventCallback';
var ON_RELEASED_MEDIA_KEY_EVENT_CALLBACK = 'onReleasedMediaKeyEventCallback';

function MediaKeyManager() {
  validator.isConstructorCall(this, MediaKeyManager);
}

MediaKeyManager.prototype.setMediaKeyEventListener = function(callback) {
    var args = validator.validateArgs(arguments, [
      {
        name: 'callback',
        type: validator.Types.LISTENER,
        values: ['onpressed', 'onreleased']
      }
    ]);

    var ret = native.callSync('MediaKeyManager_setMediaKeyEventListener', {});

    if (native.isFailure(ret)) {
        throw native.getErrorObject(ret);
    }

    native.removeListener(ON_PRESSED_MEDIA_KEY_EVENT_CALLBACK);
    native.removeListener(ON_RELEASED_MEDIA_KEY_EVENT_CALLBACK);

    native.addListener(ON_PRESSED_MEDIA_KEY_EVENT_CALLBACK, function(msg) {
        native.callIfPossible(args.callback.onpressed, msg.type);
    });
    native.addListener(ON_RELEASED_MEDIA_KEY_EVENT_CALLBACK, function(msg) {
        native.callIfPossible(args.callback.onreleased, msg.type);
    });

};

MediaKeyManager.prototype.unsetMediaKeyEventListener = function() {
    var ret = native.callSync('MediaKeyManager_unsetMediaKeyEventListener',{});

    if (native.isFailure(ret)) {
        throw native.getErrorObject(ret);
    }

    native.removeListener(ON_PRESSED_MEDIA_KEY_EVENT_CALLBACK);
    native.removeListener(ON_RELEASED_MEDIA_KEY_EVENT_CALLBACK);
};

exports = new MediaKeyManager();
