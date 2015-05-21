// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var Base64 = {
  _b64: 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=',
  encode: function(data) {
    var output = '';
    var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
    var i = 0;

    data = this._utf8_encode(data);

    while (i < data.length) {

      chr1 = data.charCodeAt(i++);
      chr2 = data.charCodeAt(i++);
      chr3 = data.charCodeAt(i++);

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
  decode: function(data) {
    var output = '';
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

      output += String.fromCharCode(chr1);

      if (enc3 !== 64) {
        output += String.fromCharCode(chr2);
      }
      if (enc4 !== 64) {
        output += String.fromCharCode(chr3);
      }

    }

    output = this._utf8_decode(output);

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
