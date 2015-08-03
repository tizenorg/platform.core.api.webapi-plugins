## WIDL

### Conventions

Currently WIDL version that is used in Samsung is described here: http://www.w3.org/TR/WebIDL/.
This is document from 19 April 2012.

WIDL used for plugins creation is closer to previous drafts mainly this from
21 October 2010. It is described here: http://www.w3.org/TR/2010/WD-WebIDL-20101021/.

### Architecture

Each plugin is separated from each other as a different module.
We do this by using module key name.

```
module identifer {
  definitions
}
```

Each module describes space, binding many connected definitions in one namespace.
Inside each module there are sets of **interface** defined.
Most of the time there is one major interface defined, which is **NoInterfaceObject**.
This is manager object which has only one property which is object that actually
implements manager functionality.

```
interface identifier : indentifier-of-inherited-interface {
  interface-member...
};
```

Interface is a definition of an object, which can be realized in a system
(an inheritance and overloading is possible).
In interface definition you can put following members:
* Constants.
* Attribute : Interface member, which represents variable inside object,
  can be changed, if it is not read only.
* Operation: Interface member, which represents method inside object.
  It is a function of programming language, which can be executed and returns a result.
* Special operation: Performs a specific task. i.e. deleter, getter
* Static operation: It is not called for a specific instance of the interface,
  is called for static object regardless of an instance creation.
  It is connected with the interface itself.

```
interface identifier {
  attribute type identifier;
  [extended-attribute] const type identifier = value;
  [extended-attribute] attribute type identifier;
  readonly attribute type identifier;
  attribute type identifier inherits getter; ///Declared to change read only attribute //inherited from interface
  attribute type identifier getraises (NoSuchValue); ///Exception declaration
  return-type identifier(arguments…);
  return-type identifier(argument-type argument-identifier); ///regular operation
  return-type identifier(optional argument);
  special-keywords return-type identifier(arguments); ///special operation
  [extended-attribute]return-type identifier(arguments…); ///A variable number of //arguments
  return-type identifier(arguments) raises (identifier) ///raises exception
  caller return-type identifier(argument);
  caller return-type (argument);
  static return-type identifier(arguments);
};
```

Next step is to connect manager implementation with Tizen object.

```
Tizen implements ManagerObject
```

To provide actual implementations of ManagerObject, instance of its Manager
interface definition has to be made. Inside this Manager interface all attributes
and functions that will be available form manager namespace, should be defined.
There can be attributes which are other interfaces, operations and everything
that interfaces allows.

Additional interface can be available as a standalone types not connected to
global namespace. Those are either obtained from operation of other interfaces
or constructed with theirs constructor method. Interface which are constructible
are described as follows:

```
[Constructor(type arg1, optional type? Arg2)]
Interface ConstructibleInterface {
  attributes
  operations
  an so on...
};
```

As one can see list of parameters is specified for such constructor.
Not all parameters are mandatory, some can be preceded by ```optional```
keyword and ```?``` mark, after type to mark that this is not obligatory argument.
Additionally some operations can be followed by ```raises``` key word to mark that,
described exception type can be thrown during execution of such method.

Because some operations can be asynchronous, it is necessary to provide callbacks
objects that can be executed by such operation. Callback object is special type
of ```interface``` object with ```Callback=FunctionOnly``` extended attribute.

```
[Callback=FunctionOnly, NoInterfaceObject] interface SomeCallback {
  void someMethod(type agr1, ...)
};
```

On the purpose of listeners which accepts dictionaries, there are callbacks that
support more than one method. There is another definition of callback which
lacks of keyword ```FunctionOnly```.

```
[Callback, NoInterfaceObject] interface SomeDictionaryCallback {
  void firstmethod(type somearg1, ... );
  void secondmethod(type somearg2, ... );
  any additional methods...
};
```

### Example

Example of WIDL file:
```
module Sample {

  enum SampleEnums {
    "ENUM1",
    "ENUM2",
    "ENUM3",
  };

  typedef (SampleEnums) SampleType;

  [NoInterfaceObject] interface SampleManagerObject {
    readonly attribute SampleManager sample;
  };

  Tizen implements SampleManagerObject;

  [NoInterfaceObject] interface SampleManager {
    void sampleMethod(SampleType param1, Sample2 param2) raises(WebAPIException);
    double sampleMethod2(SampleType param1) raises(WebAPIException);
    void sampleMethod3(SampleCallback callback) raises(WebAPIException);
  };

  [Callback=FunctionOnly, NoInterfaceObject]
  interface SampleCallback {
    void onsuccess(Sample1 param1, Sample2 param2);
  };
};
```
