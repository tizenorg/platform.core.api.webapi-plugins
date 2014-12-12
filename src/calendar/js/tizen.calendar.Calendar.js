/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

var CalendarType = {
  EVENT: 'EVENT',
  TASK: 'TASK'
};


/**
 * For internal use only.
 */
var InternalCalendar = function(data) {
  Object.defineProperties(this, {
    accountId: {
      value: -1,
      writable: true,
      enumerable: true
    },
    id: {
      value: null,
      writable: true,
      enumerable: true
    },
    name: {
      value: null,
      writable: true,
      enumerable: true
    },
    type: {
      value: '',
      writable: true,
      enumerable: true
    },
    isUnified: {
      value: false,
      writable: true,
      enumerable: true
    }
  });

  if (data instanceof Object) {
    for (var prop in data) {
      if (this.hasOwnProperty(prop)) {
        this[prop] = data[prop];
      }
    }
  }
};


// class Calendar
var Calendar = function(accountId, name, type) {
  var _data;

  AV.validateConstructorCall(this, Calendar);

  if (arguments[0] instanceof InternalCalendar) {
    _data = arguments[0];
  } else {
    var _accountId = Converter.toLong(accountId);
    var _name = Converter.toString(name);
    var _type = Converter.toString(type);

    if (arguments.length < 3) {
      _data = new InternalCalendar();
    } else {
      _data = new InternalCalendar({
                accountId: _accountId,
                name: _name,
                type: _type
      });
    }
  }

  Object.defineProperties(this, {
    accountId: {
      value: _data.accountId,
      writable: false,
      enumerable: true
    },
    id: {
      get: function() {
        return Converter.toString(_data.id, true);
      },
      set: function(v) {
        if (v instanceof InternalCalendar) {
          _data.id = v.id;
        }
      },
      enumerable: true
    },
    name: {
      value: _data.name,
      writable: false,
      enumerable: true
    },
    type: {
      value: _data.type,
      writable: false,
      enumerable: false
    },
    isUnified: {
      value: _data.isUnified,
      writable: false,
      enumerable: false
    }
  });
};

Calendar.prototype.get = function(id) {
  var args;
  if (this.type === CalendarType.TASK) {
    if (!parseInt(id) || parseInt(id) <= 0) {
      throw C.throwNotFound();
    }
    args = AV.validateMethod(arguments, [{
      name: 'id',
      type: AV.Types.STRING
    }]);
  } else {
    args = AV.validateMethod(arguments, [{
      name: 'id',
      type: AV.Types.PLATFORM_OBJECT,
      values: tizen.CalendarEventId
    }]);
  }

  var result = _callSync('Calendar_get', {
    calendarId: this.id,
    id: args.id
  });

  if (C.isFailure(result)) {
    throw C.getErrorObject(result);
  }

  _edit.allow();
  var item;
  var _item = C.getResultObject(result);

  if (this.type === CalendarType.TASK) {
    item = new CalendarTask(_itemConverter.toTizenObject(_item, _item.isAllDay));
  } else {
    item = new CalendarEvent(_itemConverter.toTizenObject(_item, _item.isAllDay));
  }
  _edit.disallow();

  return item;
};

Calendar.prototype.add = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'item',
      type: AV.Types.PLATFORM_OBJECT,
      values: [CalendarEvent, CalendarTask]
    }
  ]);

  if ((this.type === CalendarType.EVENT && !(args.item instanceof CalendarEvent)) ||
      (this.type === CalendarType.TASK && !(args.item instanceof CalendarTask))) {
    C.throwTypeMismatch('Invalid item type.');
  }

  var tmp = _itemConverter.fromTizenObject(args.item);
  tmp.calendarId = this.id;

  var result = _callSync('Calendar_add', {
    item: tmp,
    type: this.type
  });

  if (C.isFailure(result)) {
    throw C.getErrorObject(result);
  }

  var _id = C.getResultObject(result);

  _edit.allow();
  args.item.calendarId = this.id;

  switch (this.type) {
    case CalendarType.EVENT:
      args.item.id = new CalendarEventId(_id.uid, _id.rid);
      break;
    case CalendarType.TASK:
      args.item.id = _id.uid;
      break;
  }
  _edit.disallow();
};

Calendar.prototype.addBatch = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'items',
      type: AV.Types.ARRAY,
      values: (this.type === CalendarType.EVENT)
                ? tizen.CalendarEvent : tizen.CalendarTask
    }, {
      name: 'successCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    }, {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var callback = function(result) {
    if (C.isFailure(result)) {
      C.callIfPossible(args.errorCallback, C.getErrorObject(result));
    } else {
      _edit.allow();
      var _ids = C.getResultObject(result);
      for (var i = 0; i < args.items.length; i++) {
        args.items[i].calendarId = this.id;
        switch (this.type) {
          case CalendarType.EVENT:
            args.items[i].id = new CalendarEventId(_ids[i].uid, _ids[i].rid);
            break;
          case CalendarType.TASK:
            args.items[i].id = _ids[i].uid;
            break;
        }
      }
      _edit.disallow();
      C.callIfPossible(args.successCallback, args.items);
    }
  }.bind(this);

  var tmp = [];
  var tmpItem;
  for (var i = 0; i < args.items.length; i++) {
    if ((this.type === CalendarType.EVENT && !(args.items[i] instanceof CalendarEvent)) ||
            (this.type === CalendarType.TASK && !(args.items[i] instanceof CalendarTask))) {
      C.throwTypeMismatch('Invalid item type.');
    }
    tmpItem = _itemConverter.fromTizenObject(args.items[i]);
    tmpItem.calendarId = this.id;
    tmp.push(tmpItem);
  }

  var result = _call('Calendar_addBatch', {
    type: this.type,
    items: tmp
  }, callback);

  if (C.isFailure(result)) {
    throw C.getErrorObject(result);
  }
};

Calendar.prototype.update = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'item',
      type: AV.Types.PLATFORM_OBJECT,
      values: [tizen.CalendarEvent, tizen.CalendarTask]
    },
    {
      name: 'updateAllInstances',
      type: AV.Types.BOOLEAN,
      optional: true,
      nullable: true
    }
  ]);

  if ((this.type === CalendarType.EVENT && !(args.item instanceof CalendarEvent)) ||
      (this.type === CalendarType.TASK && !(args.item instanceof CalendarTask))) {
    C.throwTypeMismatch('Invalid item type.');
  }

  var tmp = _itemConverter.fromTizenObject(args.item);
  tmp.calendarId = this.id;

  var result = _callSync('Calendar_update', {
    item: tmp,
    type: this.type,
    updateAllInstances: (args.has.updateAllInstances)
            ? Converter.toBoolean(args.updateAllInstances, true)
            : true
  });

  if (C.isFailure(result)) {
    throw C.getErrorObject(result);
  }

  var _item = C.getResultObject(result);
  _edit.allow();
  for (var prop in _item) {
    if (args.item.hasOwnProperty(prop)) {
      args.item[prop] = _item[prop];
    }
  }
  _edit.disallow();

};

Calendar.prototype.updateBatch = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'items',
      type: AV.Types.ARRAY,
      values: (this.type === CalendarType.EVENT)
                ? tizen.CalendarEvent : tizen.CalendarTask
    },
    {
      name: 'successCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'updateAllInstances',
      type: AV.Types.BOOLEAN,
      optional: true,
      nullable: true
    }
  ]);

  var calendarType = this.type;

  var callback = function(result) {
    if (C.isFailure(result)) {
      C.callIfPossible(args.errorCallback, C.getErrorObject(result));
      return;
    }

    C.callIfPossible(args.successCallback);
  }.bind(this);

  var tmp = [];
  var tmpItem;
  for (var i = 0; i < args.items.length; i++) {
    if ((calendarType === CalendarType.EVENT && !(args.items[i] instanceof CalendarEvent)) ||
            (calendarType === CalendarType.TASK && !(args.items[i] instanceof CalendarTask))) {
      C.throwTypeMismatch('Invalid item type.');
    }
    tmpItem = _itemConverter.fromTizenObject(args.items[i]);
    tmp.push(tmpItem);
  }

  var result = _call('Calendar_updateBatch', {
    type: this.type,
    items: tmp,
    updateAllInstances: (args.has.updateAllInstances)
            ? Converter.toBoolean(args.updateAllInstances, true)
            : true
  }, callback);

  if (C.isFailure(result)) {
    throw C.getErrorObject(result);
  }
};

Calendar.prototype.remove = function(id) {
  var args;
  if (this.type === CalendarType.TASK) {
    if (!parseInt(id) || parseInt(id) <= 0) {
      throw C.throwNotFound();
    }
    args = AV.validateMethod(arguments, [{
      name: 'id',
      type: AV.Types.STRING
    }]);
  } else {
    args = AV.validateMethod(arguments, [{
      name: 'id',
      type: AV.Types.PLATFORM_OBJECT,
      values: tizen.CalendarEventId
    }]);
  }

  var result = _callSync('Calendar_remove', {
    type: this.type,
    id: args.id
  });

  if (C.isFailure(result)) {
    throw C.getErrorObject(result);
  }
};

Calendar.prototype.removeBatch = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'ids',
      type: AV.Types.ARRAY,
      values: (this.type === CalendarType.EVENT)
                ? tizen.CalendarEventId : undefined
    },
    {
      name: 'successCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var callback = function(result) {
    if (C.isFailure(result)) {
      C.callIfPossible(args.errorCallback, C.getErrorObject(result));
    } else {
      C.callIfPossible(args.successCallback);
    }
  };

  var result = _call('Calendar_removeBatch', {
    type: this.type,
    ids: args.ids
  }, callback);

  if (C.isFailure(result)) {
    throw C.getErrorObject(result);
  }
};

Calendar.prototype.find = function(successCallback, errorCallback, filter, sortMode) {
  var args = AV.validateMethod(arguments, [
    {
      name: 'successCallback',
      type: AV.Types.FUNCTION
    },
    {
      name: 'errorCallback',
      type: AV.Types.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'filter',
      type: AV.Types.PLATFORM_OBJECT,
      values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter],
      optional: true,
      nullable: true
    },
    {
      name: 'sortMode',
      type: AV.Types.PLATFORM_OBJECT,
      values: tizen.SortMode,
      optional: true,
      nullable: true
    }
  ]);

  var calendarType = this.type;

  var callback = function(result) {
    if (C.isFailure(result)) {
      C.callIfPossible(args.errorCallback, C.getErrorObject(result));
    } else {
      var _items = C.getResultObject(result);
      var c = [];
      _edit.allow();
      _items.forEach(function(i) {
        if (calendarType === CalendarType.TASK) {
          c.push(new CalendarTask(_itemConverter.toTizenObject(i, i.isAllDay)));
        } else {
          c.push(new CalendarEvent(_itemConverter.toTizenObject(i, i.isAllDay)));
        }
      });
      _edit.disallow();
      //TODO: Move sorting and filtering to native code
      c = C.filter(c, args.filter); // NYA: due to commit dependency
      c = C.sort(c, args.sortMode);
      args.successCallback(c);

    }
  };

  var result = _call('Calendar_find', {
    calendarId: this.id,
    filter: args.filter,
    sortMode: args.sortMode
  }, callback);

  if (C.isFailure(result)) {
    throw C.getErrorObject(result);
  }

};

var _listeners = {};
// @todo this could be replaced by _taskListeners & _eventListeners
var _nativeListeners = {};
var _nextId = 0;

function _CalendarEventChangeCallback(event) {
  _CalendarChangeCallback('EVENT', event);
}
function _CalendarTaskChangeCallback(event) {
  _CalendarChangeCallback('TASK', event);
}
function _CalendarChangeCallback(type, event) {
  var invokeListeners = function(listeners, callbackName, items) {
    var result = [];
    _edit.allow();
    for (var i = 0, length = items.length; i < length; i++) {
      var item;

      if (callbackName === 'onitemsremoved') {
        if (type === 'EVENT') {
          item = new CalendarEventId(items[i].id, null);
        } else {
          item = Converter.toString(items[i].id);
        }
      } else {
        item = _itemConverter.toTizenObject(items[i], items[i].isAllDay);
        if (type === 'EVENT') {
          item = new CalendarEvent(item);
        } else {
          item = new CalendarTask(item);
        }
      }

      result.push(item);
    }
    _edit.disallow();

    for (var watchId in listeners) {
      if (listeners.hasOwnProperty(watchId)) {
        C.callIfPossible(listeners[watchId][callbackName], result);
      }
    }
  }.bind(this);

  var groupItemsByCalendar = function(items) {
    var grouped = {};

    for (var i = 0, length = items.length; i < length; i++) {
      var item = items[i];

      // skip item if we are not listening on this calendarId
      if (!_listeners.hasOwnProperty(item.calendarId)) {
        continue;
      }

      if (!grouped.hasOwnProperty(item.calendarId)) {
        grouped[item.calendarId] = [];
      }
      grouped[item.calendarId].push(item);
    }

    return grouped;
  }.bind(this);

  var e = JSON.parse(event);
  var actions = ['added', 'updated', 'removed'];
  for (var i = 0; i < actions.length; i++) {
    var action = actions[i];
    var callback = 'onitems' + action;

    if (e.hasOwnProperty(action) && T.isArray(e[action]) && e[action].length) {

      // invoke listeners for unified calendars
      if (_listeners.hasOwnProperty(type)) {
        invokeListeners(_listeners[type], callback, e[action]);
      }

      var groupedItems = groupItemsByCalendar(e[action]);
      for (var calendarId in groupedItems) {
        if (groupedItems.hasOwnProperty(calendarId)) {
          invokeListeners(_listeners[calendarId], callback, groupedItems[calendarId]);
        }
      }
    }
  }
}

Calendar.prototype.addChangeListener = function() {
  var args = AV.validateMethod(arguments, [{
    name: 'successCallback',
    type: AV.Types.LISTENER,
    values: ['onitemsadded', 'onitemsupdated', 'onitemsremoved']
  }]);

  var listenerId = 'CalendarChangeCallback_' + this.type;

  if (!_nativeListeners.hasOwnProperty(listenerId)) {
    var result = _callSync('Calendar_addChangeListener', {
      type: this.type,
      listenerId: listenerId
    });
    if (C.isFailure(result)) {
      throw C.getErrorObject(result);
    }

    native.addListener(listenerId, (this.type === 'EVENT')
            ? _CalendarEventChangeCallback
            : _CalendarTaskChangeCallback);
    _nativeListeners[listenerId] = this.type;
  }

  // we can't use id in case of unified calendar - which is null for both calendar types
  var calendarId = (this.isUnified) ? this.type : this.id;
  if (!_listeners.hasOwnProperty(calendarId)) {
    _listeners[calendarId] = {};
  }

  var watchId = ++_nextId;
  _listeners[calendarId][watchId] = args.successCallback;

  return watchId;
};

Calendar.prototype.removeChangeListener = function() {
  var args = AV.validateMethod(arguments, [
    {
      name: 'watchId',
      type: AV.Types.LONG
    }
  ]);

  var watchId = Converter.toString(args.watchId);
  var calendarId = (this.isUnified) ? this.type : this.id;

  if (!_listeners[calendarId] || !_listeners[calendarId][watchId]) {
    return;
  }

  delete _listeners[calendarId][watchId];

  if (T.isEmptyObject(_listeners[calendarId])) {
    delete _listeners[calendarId];
  }

  if (T.isEmptyObject(_listeners)) {

    var result;
    // @todo consider listener unregister when we are not listening on this.type of calendar
    var fail = false;
    for (var listenerId in _nativeListeners) {
      if (_nativeListeners.hasOwnProperty(listenerId)) {
        result = _callSync('Calendar_removeChangeListener', {
          type: _nativeListeners[listenerId]
        });
        if (C.isFailure(result)) {
          fail = C.getErrorObject(result);
        }
        native.removeListener(listenerId, (this.type === 'EVENT')
                    ? _CalendarEventChangeCallback
                    : _CalendarTaskChangeCallback);

        delete _nativeListeners[listenerId];
      }
    }

    if (fail) {
      throw fail;
    }
  }
};

tizen.Calendar = Calendar;
