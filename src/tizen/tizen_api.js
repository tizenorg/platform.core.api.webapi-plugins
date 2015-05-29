// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Tizen API Specification:
// https://developer.tizen.org/dev-guide/2.3.0/org.tizen.mobile.web.device.apireference/tizen/tizen.html


// WebAPIException and WebAPIError definition moved to src/utils/utils_api.js
// for compliance reasons. You can find more info there.


/**
 * Filter match flags.
 * @enum {string}
 */
var FilterMatchFlag = {
  EXACTLY: 'EXACTLY',
  FULLSTRING: 'FULLSTRING',
  CONTAINS: 'CONTAINS',
  STARTSWITH: 'STARTSWITH',
  ENDSWITH: 'ENDSWITH',
  EXISTS: 'EXISTS'
};


/**
 * An enumerator that indicates the sorting order.
 * @enum {string}
 */
var SortModeOrder = {
  ASC: 'ASC',
  DESC: 'DESC'
};


/**
 * An enumerator that indicates the type of composite filter.
 * @enum {string}
 */
var CompositeFilterType = {
  UNION: 'UNION',
  INTERSECTION: 'INTERSECTION'
};

// Tizen Filters
// either AttributeFilter, AttributeRangeFilter, or CompositeFilter
function is_tizen_filter(f) {
  return (f instanceof tizen.AttributeFilter) ||
         (f instanceof tizen.AttributeRangeFilter) ||
         (f instanceof tizen.CompositeFilter);
}

//Extract property by string
function _extractProperty(obj, attributeName) {
  var props = attributeName.split('.');
  for (var i = 0; i < props.length; ++i) {
    if (obj instanceof Array) {
      var ret = [];
      for (var j = 0; j < obj.length; ++j)
      {
        ret.push(_extractProperty(obj[j], props.slice(i).join('.')));
      }
      return ret;
    }
    if (!obj.hasOwnProperty(props[i])) {
      if (i === props.length - 1) {
        throw new WebAPIException('InvalidValuesError', 'Property "' + attributeName +
                '" is not valid');
      }
      return null;
    }
    obj = obj[props[i]];
  }
  return obj;
}


/**
 * This is a common interface used by different types of object filters.
 */
var AbstractFilter = function() {};


/**
 * Represents a set of filters.
 */
exports.AttributeFilter = function(attrName, matchFlag, matchValue) {

  xwalk.utils.validator.isConstructorCall(this, exports.AttributeFilter);

  var name_ = '';
  var flag_ = 'EXACTLY';
  var value_ = null;

  function attributeNameSetter(name) {
    name_ = String(name);
  }

  if (arguments.length > 0)
    attributeNameSetter(attrName);

  function matchFlagSetter(flag) {
    if (Object.keys(FilterMatchFlag).indexOf(flag) >= 0)
      flag_ = flag;
  }

  function matchValueSetter(value) {
    value_ = value;
  }

  if (arguments.length > 2) {
    matchFlagSetter(matchFlag);
    matchValueSetter(matchValue);
  } else if (arguments.length > 1) {
    // if matchValue is not used then matchFlag is set to 'EXISTS'.
    matchFlagSetter('EXISTS');
  }

  Object.defineProperties(this, {
    attributeName: {
      enumerable: true,
      set: attributeNameSetter,
      get: function() {
        return name_;
      }
    },
    matchFlag: {
      enumerable: true,
      set: matchFlagSetter,
      get: function() {
        return flag_;
      }
    },
    matchValue: {
      enumerable: true,
      set: matchValueSetter,
      get: function() {
        return value_;
      }
    }
  });
};
exports.AttributeFilter.prototype = new AbstractFilter();

//TODO: Move filtering to native code
exports.AttributeFilter.prototype._filter = function(element) {
  var elemValue = _extractProperty(element, this.attributeName);

  if (!(elemValue instanceof Array)) {
    elemValue = [elemValue];
  }

  var ret = false;
  for (var i = 0; i < elemValue.length; ++i) {
    var elemValueStr = String(elemValue[i]);
    var elemValueStrU = elemValueStr.toUpperCase();
    var matchValueStr = String(this.matchValue);
    var matchValueStrU = matchValueStr.toUpperCase();

    switch (this.matchFlag) {
      case 'EXACTLY':
        ret = elemValue[i] === this.matchValue;
        break;
      case 'FULLSTRING':
        ret = elemValueStrU === matchValueStrU;
        break;
      case 'CONTAINS':
        ret = elemValueStrU.indexOf(matchValueStrU) > -1;
        break;
      case 'STARTSWITH':
        ret = elemValueStrU.indexOf(matchValueStrU) === 0;
        break;
      case 'ENDSWITH':
        ret = elemValueStrU.lastIndexOf(matchValueStrU) +
                matchValueStrU.length === elemValueStrU.length;
        break;
      case 'EXISTS':
        ret = elemValue[i] !== undefined;
        break;
    }
    if (ret) {
      return ret;
    }
  }
  return ret;
};
exports.AttributeFilter.prototype.constructor = exports.AttributeFilter;


/**
 * Represents a filter based on an object attribute which has values that are
 * within a particular range.
 */
exports.AttributeRangeFilter = function(attrName, start, end) {

  xwalk.utils.validator.isConstructorCall(this, exports.AttributeRangeFilter);

  var name_ = '';
  var start_ = null;
  var end_ = null;

  function attributeNameSetter(name) {
    name_ = String(name);
  }

  if (arguments.length > 0)
    attributeNameSetter(attrName);

  function initSetter(init) {
    start_ = init;
  }

  if (arguments.length > 1)
    initSetter(start);

  function endSetter(end) {
    end_ = end;
  }

  if (arguments.length > 2)
    endSetter(end);

  Object.defineProperties(this, {
    attributeName: {
      enumerable: true,
      set: attributeNameSetter,
      get: function() {
        return name_;
      }
    },
    initialValue: {
      enumerable: true,
      set: initSetter,
      get: function() {
        return start_;
      }
    },
    endValue: {
      enumerable: true,
      set: endSetter,
      get: function() {
        return end_;
      }
    }
  });
};

exports.AttributeRangeFilter.prototype = new AbstractFilter();

//TODO: Move filtering to native code
exports.AttributeRangeFilter.prototype._filter = function(element) {
  var elemValue = _extractProperty(element, this.attributeName);

  if (!(elemValue instanceof Array)) {
    elemValue = [elemValue];
  }

  for (var i = 0; i < elemValue.length; ++i) {
    var value = elemValue[i];

    if ((this.initialValue !== undefined && this.initialValue !== null) &&
            (this.endValue !== undefined && this.endValue !== null)) {
      if (value instanceof tizen.TZDate) {
        if (this.initialValue.earlierThan(value) && this.endValue.laterThan(value)) {
          return true;
        }
      } else {
        if (this.initialValue <= value && this.endValue > value) {
          return true;
        }
      }
    } else if ((this.initialValue !== undefined && this.initialValue !== null) &&
            (this.endValue === undefined || this.endValue === null)) {
      if (value instanceof tizen.TZDate) {
        if (this.initialValue.earlierThan(value)) {
          return true;
        }
      } else {
        if (this.initialValue <= value) {
          return true;
        }
      }
    } else if ((this.initialValue === undefined || this.initialValue === null) &&
            (this.endValue !== undefined && this.endValue !== null)) {
      if (value instanceof tizen.TZDate) {
        if (this.endValue.laterThan(value)) {
          return true;
        }
      } else {
        if (this.endValue > value) {
          return true;
        }
      }
    }
  }
  return false;
};

exports.AttributeRangeFilter.prototype.constructor = exports.AttributeRangeFilter;


/**
 * Represents a set of filters.
 */
exports.CompositeFilter = function(type, filters) {

  xwalk.utils.validator.isConstructorCall(this, exports.CompositeFilter);

  var filterTypes = Object.keys(CompositeFilterType);

  var type_ = filterTypes[0];
  var filters_ = [];

  function typeSetter(filterType) {
    if (filterTypes.indexOf(filterType) >= 0)
      type_ = filterType;
  }

  if (arguments.length > 0)
    typeSetter(type);

  function filtersSetter(filterList) {
    if (!(filterList instanceof Array))
      return;

    for (var i in filterList) {
      var valid = (filterList[i] instanceof AbstractFilter);
      if (!valid)
        return;
    }

    filters_ = filterList.slice(0);
  }

  if (arguments.length > 1)
    filtersSetter(filters);

  Object.defineProperties(this, {
    type: {
      enumerable: true,
      set: typeSetter,
      get: function() {
        return type_;
      }
    },
    filters: {
      enumerable: true,
      set: filtersSetter,
      get: function() {
        return filters_;
      }
    }
  });
};

exports.CompositeFilter.prototype = new AbstractFilter();

//TODO: Move filtering to native code
exports.CompositeFilter.prototype._filter = function(element) {
  var filters = this.filters;
  if (this.type === 'UNION') {
    for (var i = 0; i < filters.length; ++i) {
      if (filters[i]._filter(element)) {
        return true;
      }
    }
    return false;
  } else if (this.type === 'INTERSECTION') {
    if (filters.length === 0)
      return false;
    for (var i = 0; i < filters.length; ++i) {
      if (!filters[i]._filter(element)) {
        return false;
      }
    }
    return true;
  }
};

exports.CompositeFilter.prototype.constructor = exports.CompositeFilter;


/**
 * SortMode is a common interface used for sorting of queried data.
 */
exports.SortMode = function(attrName, order) {

  xwalk.utils.validator.isConstructorCall(this, exports.SortMode);

  var sortModeOrder = Object.keys(SortModeOrder);

  var attributeName_ = '';
  var order_ = 'ASC';

  function nameSetter(name) {
    attributeName_ = String(name);
  }

  if (arguments.length > 0)
    nameSetter(attrName);

  function orderSetter(sortOrder) {
    if (sortModeOrder.indexOf(sortOrder) >= 0)
      order_ = sortOrder;
  }

  if (arguments.length > 1)
    orderSetter(order);

  Object.defineProperties(this, {
    attributeName: {
      enumerable: true, set: nameSetter, get: function() {
        return attributeName_;
      }
    },
    order: {
      enumerable: true, set: orderSetter, get: function() {
        return order_;
      }
    }
  });
};
exports.SortMode.prototype.constructor = exports.SortMode;


/**
 * Represents a point (latitude and longitude) in the map coordinate system.
 */
exports.SimpleCoordinates = function(lat, lng) {

  xwalk.utils.validator.isConstructorCall(this, exports.SimpleCoordinates);

  var latitude = 0;
  var longitude = 0;

  function latSetter(lat) {
    var tmp = Number(lat);
    if (!isNaN(tmp)) {
      if (tmp > 90) tmp = 90;
      else if (tmp < -90) tmp = -90;

      latitude = tmp;
    }
  }

  latSetter(lat);

  function lonSetter(lon) {
    var tmp = Number(lon);
    if (!isNaN(tmp)) {
      if (tmp > 180) tmp = 180;
      else if (tmp < -180) tmp = -180;

      longitude = tmp;
    }
  }

  lonSetter(lng);

  Object.defineProperties(this, {
    latitude: {
      enumerable: true, set: latSetter, get: function() {
        return latitude;
      }
    },
    longitude: {
      enumerable: true, set: lonSetter, get: function() {
        return longitude;
      }
    }
  });
};
exports.SimpleCoordinates.prototype.constructor = exports.SimpleCoordinates;
