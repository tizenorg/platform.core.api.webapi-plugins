## Implementation - C++

### Lifecycle and plugin state

All plugins instances are created by runtime on application launch.
It's important to not initialize any database/service connections and platform
handlers in instance constructor. All resources should be "lazy" initialized
just before first use, to keep starting time as short as possible.
Initialized resources can be referenced to instance and kept for further usage.
Instance destructor is called on application termination and should release all
used resources to prevent memory leaks.

Native layer should be considered as stateless. It means that there is
no strict reference between JavaScript and native data.<br>
Example: If operation should change some object retrieved from platform,
identifier should be passed again and additional check if object still exists
should be made.

### Privileges

Some of the API methods require privilege access. In Tizen 3.0 checking if application has privilege
to call method has been moved to C++ layer. It's the first step which should be checked.

Below can be found example of using Privilege in Alarm API:

```cpp
const std::string kPrivilegeAlarm = "http://tizen.org/privilege/alarm.get";

// inside add, remove, removeAll methods:
CHECK_PRIVILEGE_ACCESS(kPrivilegeAlarm, &out);
```
### Namespace and entry points

Extension namespace and other objects exported by JavaScript layer are defined
inside ```<pluginname>_extension.cc``` file.

```cpp
SetExtensionName("tizen.notification"); //exported in JS as new NotificationManager();
const char* entry_points[] = {"tizen.StatusNotification",
                              "tizen.NotificationDetailInfo",
                              NULL};
```

### Plugin structure

In general ```Instance``` class (```<pluginname>_instance.cc```) should be
treated as command dispatcher and should be as small as possible (similar to Controller in MVC).
It's responsibility should be limited to reading/validating arguments,
forwarding call to business logic component and passing result to JavaScript layer.
Business logic should be implemented in additional classes with
[SOLID][1] principles in mind.

Commands callable from JavaScript layer should be registered in constructor of
```<PluginName>Instance``` class which extends ```common::ParsedInstance```.

```cpp
// <pluginname>_instance.h
class MediaControllerInstance : public common::ParsedInstance {}
```

Currently there is no difference in registering sync and async commands.
But good practice is to separate them for readability and maintainability.
Common practice is to define two macros and call ```RegisterSyncHandler```
method from ```common::ParsedInstance```.

```cpp
// <pluginname>_instance.cc
MediaControllerInstance::MediaControllerInstance() {
  #define REGISTER_SYNC(c, x) \
      RegisterSyncHandler(c, std::bind(&MediaControllerInstance::x, this, _1, _2));
  #define REGISTER_ASYNC(c, x) \
      RegisterSyncHandler(c, std::bind(&MediaControllerInstance::x, this, _1, _2));

  REGISTER_SYNC("MediaControllerManager_getClient",
      MediaControllerManagerGetClient);
  REGISTER_ASYNC("MediaControllerClient_findServers",
      MediaControllerClientFindServers);

  // ... other commands

  #undef REGISTER_SYNC
  #undef REGISTER_ASYNC
}
```

Static method registered as a handler must have proper signature:
```cpp
void InstanceClass::HandlerName(const picojson::value& args, picojson::object& out);
```

- ```args``` - object containing arguments passed from JavaScript layer
- ```out``` - object containing response data returned synchronously to JavaScript
  layer.

```ReportSuccess()```, ```ReportError()``` or ```LogAndReportError()``` helpers should be used to ensure
proper structure of ```out``` object. ```LogAndReportError()``` expands ```ReportError()``` function.
It sets error in ```out``` object but additionally prints message if ```ERROR``` log priority is on.

```cpp
picojson::value data = picojson::value<picojson::object());
const PlatformResult& result = model_->DoSomethingWithData(&data);
if (!result) {
  LogAndReportError(result, &out);
  return;
}

ReportSuccess(data, out);
```

### Asynchronous calls

To perform asynchronous request ```common::TaskQueue``` component should be used.
You should use lambda expression which calls business logic and passes result to
JavaScript layer by calling ```PostMessage(const char* msg)```.

Asynchronous response is not matched to request automatically. You should pass
```callbackId``` received from JavaScript layer as an argument. It allows to
call the appropriate user callback in JS async message handler.

```cpp
auto search = [this, args]() -> void {

  // business logic
  picojson::value servers = picojson::value(picojson::array());
  PlatformResult result = client_->FindServers(&servers.get<picojson::array>());

  // response object
  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();
  response_obj["callbackId"] = args.get("callbackId");
  if (result) {
    ReportSuccess(servers, response_obj);
  } else {
    LogAndReportError(result, &response_obj);
  }

  // post JSON string to JS layer
  PostMessage(response.serialize().c_str());
};

TaskQueue::GetInstance().Async(search);
```

### Listeners

Sending events from platform listeners is very similar to sending asynchronous
responses. ```PostMessage(const char* msg)``` should be called with ```listenerId```
passed from JavaScript layer.

```cpp
auto listener = [this, args](picojson::value* data) -> void {

  if (!data) {
    LoggerW("No data passed to json callback");
    return;
  }

  picojson::object& request_o = data->get<picojson::object>();
  request_o["listenerId"] = args.get("listenerId");

  PostMessage(data->serialize().c_str());
};
```

### Logger

Logger is available from ```common/logger.h```
Available log priorities are: ```DEBUG```, ```INFO```, ```WARN``` and ```ERROR```. Each priority
has macro which should be used to filter messages based on level of importance:
* ```LoggerD()``` prints message with DEBUG priority
* ```LoggerI()``` prints message with INFO priority
* ```LoggerW()``` prints message with WARNING priority
* ```LoggerE()``` prints message with ERROR priority

 Example:

```cpp
LoggerD("Entered to getAll() function");
LoggerI("Data size: %d", object->data->size);
LoggerW("Data in %s object is empty", object->name);
LoggerE("Error occurred while getting data [%d]", error);
```

### Error handling

Regarding to [Google C++ Style Guide][2] we do not use Exceptions.

To deliver error conditions to JavaScript layer, that can occur in the platform,
```LogAndReportError``` or ```ReportError``` method should be used. All available error codes are defined
in ```common/platform_result.h```

LogAndReportError can be used anywhere in native layer. It sets PlatformResult error object in response
object which should be converted to exception and thrown in JavaScript layer:

```cpp
// C++ layer
LogAndReportError(PlatformResult(ErrorCode::NOT_FOUND_ERR, "Cannot remove notification error"), &out);
LogAndReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get notification id error"), &out);
```

```js
// JavaScript layer
var native_ = new xwalk.utils.NativeManager(extension);
if (native_.isFailure(ret)) {
  throw native_.getErrorObject(ret);
}
```

  [1]: http://en.wikipedia.org/wiki/SOLID_(object-oriented_design)
  [2]: http://google-styleguide.googlecode.com/svn/trunk/cppguide.html#Exceptions