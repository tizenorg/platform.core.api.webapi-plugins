## Implementation - JavaScript

Each plugin contains JavaScript files. This is the place where user input is
being processed validated before send to C++ layer.

Badge API will be used to show the creation of JavaScript file (lot of content
of this file will be already generated via Stub Generator).

### Interface creation

The WIDL of BadgeManager – main entity that holds all the API methods
looks like following:

```
[NoInterfaceObject] interface BadgeManager {
  readonly attribute long maxBadgeCount
  void setBadgeCount(ApplicationId appId, long count) raises(WebAPIException);
  long getBadgeCount(ApplicationId appId) raises(WebAPIException);
  void addChangeListener(ApplicationId[]appIdList, BadgeChangeCallback successCallback) raises(WebAPIException);
  void removeChangeListener(ApplicationId[] appIdList) raises(WebAPIException);
};
```

### Creating Manager entity

Object that will hold attributes and methods is defined as JavaScript function:

```js
function BadgeManager() {}
```

### Properties definition

Properties are defined within the created JavaScript function like this:

```js
var MAX_BADGE_COUNT = 999;
Object.defineProperties(this, {
  maxBadgeCount: {value: MAX_BADGE_COUNT, emumerable: true, writable: false}
});
```

Because the property was defined as ```const```, writable is set to false.

### Methods definition

In accordance to WIDL BadgeManager contains setBadgeCount method.
To define this method within JavaScript use prototype extension functionality:

```js
BadgeManager.prototype.setBadgeCount = function() {};
```

### Exporting interface

Once the object is created and all the methods and attributes are set it has to
be exported so it will be visible when making call to tizen.badge namespace.
This is done using assigning new object instance to exports variable:

```js
exports = new BadgeManager(); //exported as tizen.badge
exports = new CalendarManager(); //exported as tizen.calendar
```

Other namespaces within the module are exported as below:

```js
tizen.CalendarAttendee = CalendarAttendee;
tizen.CalendarEvent = CalendarEvent;
tizen.CalendarTask = CalendarTask;
```

### Utils

In ```src/utils/utils_api.js``` file there is a lot of useful tools that allow
automatization of certain operations. Most often used tools from utils_api.js
are converter and validator. All tools are available under ```xwalk.utils``` namespace.

#### Converter

A lot of times conversion between JavaScript types will be required.
The converter tool was created in order to make this operation easier.

```js
var converter_ = xwalk.utils.converter;
var number = converter_.toLong(result);
```

#### Validator

When API JavaScript method is called first thing that has to be done in
JavaScript layer of api implementation is to process and validate arguments
given by the user. The process of validation consists of ensuring that the
proper amount of arguments was given and that they were of the expected
type and throwing exception if necessary.

Validator helps to ensure that user sent proper values. Validator is available
at ```xwalk.utils.validator``` and predefined js types at ```xwalk.utils.validator.types```

Below can be found example of using validator inside ```setBadgeCount``` method
that requires appId in form of string and long count value:

```js
var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;

var args = validator_.validateArgs(arguments, [
  {name: 'appId', type: types_.STRING},
  {name: 'count', type: types_.LONG}
]);
```

#### Privileges

Some of the API methods require privilege access, then it's the first step
in JavaScript file which should be checked.

Below can be found example of using Privilege in Alarm API:

```js
var Privilege = xwalk.utils.privilege;

// inside add, remove, removeAll methods:
xwalk.utils.checkPrivilegeAccess(Privilege.ALARM);
```

### Exceptions

At some point whether improper data is received or given to JavaScript might
require to throw exceptions. The example below shows how to throw properly
predefined exceptions:

```js
throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
                          'Incorrect number of arguments');
```

WebAPIException constructor takes as argument the type of error to be thrown.
The second additional argument is error message.

### Synchronous methods

In order to perform synchronous operation (one that does not require callback
and the result is given instantly) callSync() method of Native manager needs
to be called:

```js
var native_ = new xwalk.utils.NativeManager(extension);
var ret = native_.callSync('BadgeManager_setBadgeCount', {
  appId: args.appId,
  count: args.count
});
if (native_.isFailure(ret)) {
  throw native_.getErrorObject(ret);
}
```

The first argument is the command name registered in C++ layer that has to be called,
the second is arguments object that will be passed to this method.
Result is assigned to ret variable.

### Asynchronous methods

In order to work with method that requires callback instead of callSync(),
call() method needs to be called. Apart from the first two arguments that are
exactly the same as in call() method (c++ method binding, object) it takes
additional argument that is a function that will be called when the native
call is processed:

```js
var native_ = new xwalk.utils.NativeManager(extension);
var callback = function(result) {
  if (native_.isFailure(result)) {
    native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
  } else {
    var calendars = native_.getResultObject(result);
    var c = [];
    calendars.forEach(function(i) {
      c.push(new Calendar(new InternalCalendar(i)));
    });
    args.successCallback(c);
  }
};

native_.call('CalendarManager_getCalendars', callArgs, callback);
```

### Listeners

In order to work with listeners NativeManager provides ```addListener``` and
```removeListener``` methods. This method takes two arguments: one is unique
```listenerId``` that will be processed when making a call from C++ to JavaScript.
The second one is the function that is called whenever expected event occurs.

```js
var native_ = new xwalk.utils.NativeManager(extension);
var listenerId = 'PLUGIN_LISTENER_NAME';
native_.addListener(listenerId, function(data) {
  // handle event data
});
native.callSync('Calendar_addChangeListener', {
  type: this.type,
  listenerId: listenerId
});
```
