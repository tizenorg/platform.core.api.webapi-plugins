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

var _global = window || global || {};
 
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

  if (data instanceof _global.Object) {
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

  validator_.isConstructorCall(this, Calendar);

  if (arguments[0] instanceof InternalCalendar) {
    _data = arguments[0];
  } else {
    var _accountId = converter_.toLong(accountId);
    var _name = converter_.toString(name);
    var _type = converter_.toString(type);

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
      value: converter_.toLong(_data.accountId),
      writable: false,
      enumerable: true
    },
    id: {
      get: function() {
        return converter_.toString(_data.id, true);
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args;
  if (this.type === CalendarType.TASK) {
    if (!parseInt(id) || parseInt(id) <= 0) {
      throw new WebAPIException(WebAPIException.NOT_FOUND_ERR);
    }
    args = validator_.validateArgs(arguments, [{
      name: 'id',
      type: types_.STRING
    }]);
  } else {
    args = validator_.validateArgs(arguments, [{
      name: 'id',
      type: types_.PLATFORM_OBJECT,
      values: tizen.CalendarEventId
    }]);
  }

  var result = native_.callSync('Calendar_get', {
    calendarId: this.id,
    id: args.id
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  _edit.allow();
  var item;
  var _item = native_.getResultObject(result);

  if (this.type === CalendarType.TASK) {
    item = new CalendarTask(_itemConverter.toTizenObject(_item, _item.isAllDay));
  } else {
    item = new CalendarEvent(_itemConverter.toTizenObject(_item, _item.isAllDay));
  }
  _edit.disallow();

  return item;
};

Calendar.prototype.add = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_WRITE);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'item',
      type: types_.PLATFORM_OBJECT,
      values: [CalendarEvent, CalendarTask]
    }
  ]);

  if ((this.type === CalendarType.EVENT && !(args.item instanceof CalendarEvent)) ||
      (this.type === CalendarType.TASK && !(args.item instanceof CalendarTask))) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
        'Invalid item type.');
  }

  var tmp = _itemConverter.fromTizenObject(args.item);
  tmp.calendarId = this.id;

  var result = native_.callSync('Calendar_add', {
    item: tmp,
    type: this.type
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var _id = native_.getResultObject(result);

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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_WRITE);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'items',
      type: types_.ARRAY,
      values: (this.type === CalendarType.EVENT)
                ? tizen.CalendarEvent : tizen.CalendarTask
    }, {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    }, {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
      _edit.allow();
      var _ids = native_.getResultObject(result);
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
      native_.callIfPossible(args.successCallback, args.items);
    }
  }.bind(this);

  var tmp = [];
  var tmpItem;
  for (var i = 0; i < args.items.length; i++) {
    if ((this.type === CalendarType.EVENT && !(args.items[i] instanceof CalendarEvent)) ||
            (this.type === CalendarType.TASK && !(args.items[i] instanceof CalendarTask))) {
      throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
        'Invalid item type.');
    }
    tmpItem = _itemConverter.fromTizenObject(args.items[i]);
    tmpItem.calendarId = this.id;
    tmp.push(tmpItem);
  }

  native_.call('Calendar_addBatch', {
    type: this.type,
    items: tmp
  }, callback);

};

Calendar.prototype.update = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_WRITE);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'item',
      type: types_.PLATFORM_OBJECT,
      values: [tizen.CalendarEvent, tizen.CalendarTask]
    },
    {
      name: 'updateAllInstances',
      type: types_.BOOLEAN,
      optional: true,
      nullable: true
    }
  ]);

  if ((this.type === CalendarType.EVENT && !(args.item instanceof CalendarEvent)) ||
      (this.type === CalendarType.TASK && !(args.item instanceof CalendarTask))) {
    throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
      'Invalid item type.');
  }

  var tmp = _itemConverter.fromTizenObject(args.item);
  tmp.calendarId = this.id;

  var result = native_.callSync('Calendar_update', {
    item: tmp,
    type: this.type,
    updateAllInstances: (args.has.updateAllInstances)
            ? converter_.toBoolean(args.updateAllInstances, true)
            : true
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }

  var _item = native_.getResultObject(result);
  _edit.allow();
  for (var prop in _item) {
    if (args.item.hasOwnProperty(prop)) {
      args.item[prop] = _item[prop];
    }
  }
  _edit.disallow();

};

Calendar.prototype.updateBatch = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_WRITE);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'items',
      type: types_.ARRAY,
      values: (this.type === CalendarType.EVENT)
                ? tizen.CalendarEvent : tizen.CalendarTask
    },
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'updateAllInstances',
      type: types_.BOOLEAN,
      optional: true,
      nullable: true
    }
  ]);

  var calendarType = this.type;

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
      return;
    }

    native_.callIfPossible(args.successCallback);
  }.bind(this);

  var tmp = [];
  var tmpItem;
  for (var i = 0; i < args.items.length; i++) {
    if ((calendarType === CalendarType.EVENT && !(args.items[i] instanceof CalendarEvent)) ||
            (calendarType === CalendarType.TASK && !(args.items[i] instanceof CalendarTask))) {
      throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR,
        'Invalid item type.');
    }
    tmpItem = _itemConverter.fromTizenObject(args.items[i]);
    tmp.push(tmpItem);
  }

  native_.call('Calendar_updateBatch', {
    type: this.type,
    items: tmp,
    updateAllInstances: (args.has.updateAllInstances)
            ? converter_.toBoolean(args.updateAllInstances, true)
            : true
  }, callback);

};

Calendar.prototype.remove = function(id) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_WRITE);

  var args;
  if (this.type === CalendarType.TASK) {
    if (!parseInt(id) || parseInt(id) <= 0) {
      throw new WebAPIException(WebAPIException.NOT_FOUND_ERR);
    }
    args = validator_.validateArgs(arguments, [{
      name: 'id',
      type: types_.STRING
    }]);
  } else {
    args = validator_.validateArgs(arguments, [{
      name: 'id',
      type: types_.PLATFORM_OBJECT,
      values: tizen.CalendarEventId
    }]);
  }

  var result = native_.callSync('Calendar_remove', {
    type: this.type,
    id: args.id
  });

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
};

Calendar.prototype.removeBatch = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_WRITE);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'ids',
      type: types_.ARRAY,
      values: (this.type === CalendarType.EVENT)
                ? tizen.CalendarEventId : undefined
    },
    {
      name: 'successCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    }
  ]);

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
      native_.callIfPossible(args.successCallback);
    }
  };

  native_.call('Calendar_removeBatch', {
    type: this.type,
    ids: args.ids
  }, callback);

};

Calendar.prototype.find = function(successCallback, errorCallback, filter, sortMode) {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'successCallback',
      type: types_.FUNCTION
    },
    {
      name: 'errorCallback',
      type: types_.FUNCTION,
      optional: true,
      nullable: true
    },
    {
      name: 'filter',
      type: types_.PLATFORM_OBJECT,
      values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter],
      optional: true,
      nullable: true
    },
    {
      name: 'sortMode',
      type: types_.PLATFORM_OBJECT,
      values: tizen.SortMode,
      optional: true,
      nullable: true
    }
  ]);
  args.filter = utils_.repackFilter(args.filter);
  var calendarType = this.type;

  var callback = function(result) {
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.errorCallback, native_.getErrorObject(result));
    } else {
      var _items = native_.getResultObject(result);
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
      args.successCallback(c);

    }
  };
  native_.call('Calendar_find', {
    calendarId: this.id,
    filter: args.filter,
    sortMode: args.sortMode || null
  }, callback);

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
          item = converter_.toString(items[i].id);
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
        native_.callIfPossible(listeners[watchId][callbackName], result);
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

  var actions = ['added', 'updated', 'removed'];
  for (var i = 0; i < actions.length; i++) {
    var action = actions[i];
    var callback = 'onitems' + action;

    if (event.hasOwnProperty(action) && type_.isArray(event[action]) && event[action].length) {

      // invoke listeners for unified calendars
      if (_listeners.hasOwnProperty(type)) {
        invokeListeners(_listeners[type], callback, event[action]);
      }

      var groupedItems = groupItemsByCalendar(event[action]);
      for (var calendarId in groupedItems) {
        if (groupedItems.hasOwnProperty(calendarId)) {
          invokeListeners(_listeners[calendarId], callback, groupedItems[calendarId]);
        }
      }
    }
  }
}

Calendar.prototype.addChangeListener = function() {
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args = validator_.validateArgs(arguments, [{
    name: 'successCallback',
    type: types_.LISTENER,
    values: ['onitemsadded', 'onitemsupdated', 'onitemsremoved']
  }]);

  var listenerId = 'CalendarChangeCallback_' + this.type;

  if (!_nativeListeners.hasOwnProperty(listenerId)) {
    var result = native_.callSync('Calendar_addChangeListener', {
      type: this.type,
      listenerId: listenerId
    });
    if (native_.isFailure(result)) {
      throw native_.getErrorObject(result);
    }

    native_.addListener(listenerId, (this.type === 'EVENT')
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
  xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.CALENDAR_READ);

  var args = validator_.validateArgs(arguments, [
    {
      name: 'watchId',
      type: types_.LONG
    }
  ]);

  var watchId = converter_.toString(args.watchId);
  var calendarId = (this.isUnified) ? this.type : this.id;

  if (!_listeners[calendarId] || !_listeners[calendarId][watchId]) {
    return;
  }

  delete _listeners[calendarId][watchId];

  if (type_.isEmptyObject(_listeners[calendarId])) {
    delete _listeners[calendarId];
  }

  if (type_.isEmptyObject(_listeners)) {

    var result;
    // @todo consider listener unregister when we are not listening on this.type of calendar
    var fail = false;
    for (var listenerId in _nativeListeners) {
      if (_nativeListeners.hasOwnProperty(listenerId)) {
        result = native_.callSync('Calendar_removeChangeListener', {
          type: _nativeListeners[listenerId]
        });
        if (native_.isFailure(result)) {
          fail = native_.getErrorObject(result);
        }
        native_.removeListener(listenerId, (this.type === 'EVENT')
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
