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
var utils_ = xwalk.utils;
var native_ = new utils_.NativeManager(extension);
var T_ = utils_.type;


function ConvergenceManager() {
  // constructor of ConvergenceManager

}

ConvergenceManager.prototype.startDiscovery = function(successCallback, errorCallback, timeout) {
  console.log('Entered ConvergenceManager.startDiscovery()');
  var args = validator_.validateArgs(arguments, [
    {name: 'successCallback', type: types_.LISTENER, values: ['onfound', 'onfinished']},
    {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
    {name: 'timeout', type: types_.LONG, optional: true}
  ]);

  if(!args.timeout)
    args.timeout = 0; // Default value of timeout

  try {

    args.listenerId = 'CONVERGENCELISTENER';

    native_.addListener(args.listenerId, function(data) {
      console.log('ON DISCOVERY LISTENER');

      var argjson = JSON.stringify(data);
      console.log(argjson);
    });

    native_.callSync('ConvergenceManager_startDiscovery',
      {'timeout' : args.timeout, 'listenerId': args.listenerId});

   } catch (e) {
    console.log('DISCOVERY EXCEPTION');
    throw e;
  }
};

exports = new ConvergenceManager();

