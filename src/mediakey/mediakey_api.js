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
