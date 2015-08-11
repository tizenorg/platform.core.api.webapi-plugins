/*
 * Copyright (c) 2015, Ace (Ajax.org Cloud9 Editor)
 * All rights reserved.
 *
 * Licensed under the BSD;
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

ace.define("ace/ext/spellcheck",["require","exports","module","ace/lib/event","ace/editor","ace/config"],function(e,t,n){var r=e("../lib/event");t.contextMenuHandler=function(e){var t=e.target,n=t.textInput.getElement();if(!t.selection.isEmpty())return;var i=t.getCursorPosition(),s=t.session.getWordRange(i.row,i.column),o=t.session.getTextRange(s);t.session.tokenRe.lastIndex=0;if(!t.session.tokenRe.test(o))return;var u="",a=o+" "+u;n.value=a,n.setSelectionRange(o.length+1,o.length+1),n.setSelectionRange(0,0);var f=!1;r.addListener(n,"keydown",function l(){r.removeListener(n,"keydown",l),f=!0}),t.textInput.setInputHandler(function(e){console.log(e,a,n.selectionStart,n.selectionEnd);if(e==a)return"";if(e.lastIndexOf(a,0)===0)return e.slice(a.length);if(e.substr(n.selectionEnd)==a)return e.slice(0,-a.length);if(e.slice(-2)==u){var r=e.slice(0,-2);if(r.slice(-1)==" ")return f?r.substring(0,n.selectionEnd):(r=r.slice(0,-1),t.session.replace(s,r),"")}return e})};var i=e("../editor").Editor;e("../config").defineOptions(i.prototype,"editor",{spellcheck:{set:function(e){var n=this.textInput.getElement();n.spellcheck=!!e,e?this.on("nativecontextmenu",t.contextMenuHandler):this.removeListener("nativecontextmenu",t.contextMenuHandler)},value:!0}})})