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

var Base64 = {
  _b64: 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=',
  encodeString: function(data) {
    data = this._utf8_encode(data);
    var input = [];
    for (var i = 0; i < data.length; ++i) {
      input.push(data.charCodeAt(i));
    }
    return this.encode(input);
  },
  encode: function(data) {
    var output = '';
    var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
    var i = 0;

    while (i < data.length) {

      chr1 = data[i++];
      chr2 = data[i++];
      chr3 = data[i++];

      enc1 = chr1 >> 2;
      enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
      enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
      enc4 = chr3 & 63;

      if (isNaN(chr2)) {
        enc3 = enc4 = 64;
      } else if (isNaN(chr3)) {
        enc4 = 64;
      }

      output += this._b64.charAt(enc1) + this._b64.charAt(enc2) +
                this._b64.charAt(enc3) + this._b64.charAt(enc4);

    }

    return output;
  },
  decodeString: function(data) {
    data = this.decode(data);
    var output = '';
    for (var i = 0; i < data.length; ++i) {
      output += String.fromCharCode(data[i]);
    }
    return this._utf8_decode(output);
  },
  decode: function(data) {
    var output = [];
    var chr1, chr2, chr3;
    var enc1, enc2, enc3, enc4;
    var i = 0;

    data = data.replace(/[^A-Za-z0-9\+\/\=]/g, '');

    while (i < data.length) {

      enc1 = this._b64.indexOf(data.charAt(i++));
      enc2 = this._b64.indexOf(data.charAt(i++));
      enc3 = this._b64.indexOf(data.charAt(i++));
      enc4 = this._b64.indexOf(data.charAt(i++));

      chr1 = (enc1 << 2) | (enc2 >> 4);
      chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
      chr3 = ((enc3 & 3) << 6) | enc4;

      output.push(chr1);

      if (enc3 !== 64) {
        output.push(chr2);
      }
      if (enc4 !== 64) {
        output.push(chr3);
      }

    }

    return output;
  },
  _utf8_encode: function(str) {
    str = str.replace(/\r\n/g, '\n');
    var utftext = '';

    for (var n = 0; n < str.length; n++) {

      var c = str.charCodeAt(n);

      if (c < 128) {
        utftext += String.fromCharCode(c);
      }
      else if ((c > 127) && (c < 2048)) {
        utftext += String.fromCharCode((c >> 6) | 192);
        utftext += String.fromCharCode((c & 63) | 128);
      }
      else {
        utftext += String.fromCharCode((c >> 12) | 224);
        utftext += String.fromCharCode(((c >> 6) & 63) | 128);
        utftext += String.fromCharCode((c & 63) | 128);
      }

    }

    return utftext;
  },
  _utf8_decode: function(utftext) {
    var str = '';
    var i = 0, c = 0, c1 = 0, c2 = 0;

    while (i < utftext.length) {

      c = utftext.charCodeAt(i);

      if (c < 128) {
        str += String.fromCharCode(c);
        i++;
      }
      else if ((c > 191) && (c < 224)) {
        c1 = utftext.charCodeAt(i + 1);
        str += String.fromCharCode(((c & 31) << 6) | (c1 & 63));
        i += 2;
      }
      else {
        c1 = utftext.charCodeAt(i + 1);
        c2 = utftext.charCodeAt(i + 2);
        str += String.fromCharCode(((c & 15) << 12) | ((c1 & 63) << 6) | (c2 & 63));
        i += 3;
      }

    }

    return str;
  }
};
