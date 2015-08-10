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

ace.define("ace/ext/static_highlight",["require","exports","module","ace/edit_session","ace/layer/text"],function(e,t,n){var r=e("../edit_session").EditSession,i=e("../layer/text").Text,s=".ace_editor {font-family: 'Monaco', 'Menlo', 'Droid Sans Mono', 'Courier New', monospace;font-size: 12px;}.ace_editor .ace_gutter { width: 25px !important;display: block;float: left;text-align: right; padding: 0 3px 0 0; margin-right: 3px;}.ace_line { clear: both; }*.ace_gutter-cell {-moz-user-select: -moz-none;-khtml-user-select: none;-webkit-user-select: none;user-select: none;}";t.render=function(e,t,n,o,u){o=parseInt(o||1,10);var a=new r("");a.setMode(t),a.setUseWorker(!1);var f=new i(document.createElement("div"));f.setSession(a),f.config={characterWidth:10,lineHeight:20},a.setValue(e);var l=[],c=a.getLength();for(var h=0;h<c;h++)l.push("<div class='ace_line'>"),u||l.push("<span class='ace_gutter ace_gutter-cell' unselectable='on'>"+(h+o)+"</span>"),f.$renderLine(l,h,!0,!1),l.push("</div>");var p="<div class=':cssClass'>        <div class='ace_editor ace_scroller ace_text-layer'>            :code        </div>    </div>".replace(/:cssClass/,n.cssClass).replace(/:code/,l.join(""));return f.destroy(),{css:s+n.cssText,html:p}}})