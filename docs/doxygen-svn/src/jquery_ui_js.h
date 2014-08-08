"/*!\n"
" * jQuery UI 1.8.18\n"
" *\n"
" * Copyright 2011, AUTHORS.txt (http://jqueryui.com/about)\n"
" * Dual licensed under the MIT or GPL Version 2 licenses.\n"
" * http://jquery.org/license\n"
" *\n"
" * http://docs.jquery.com/UI\n"
" */\n"
"(function(a,d){a.ui=a.ui||{};if(a.ui.version){return}a.extend(a.ui,{version:\"1.8.18\",keyCode:{ALT:18,BACKSPACE:8,CAPS_LOCK:20,COMMA:188,COMMAND:91,COMMAND_LEFT:91,COMMAND_RIGHT:93,CONTROL:17,DELETE:46,DOWN:40,END:35,ENTER:13,ESCAPE:27,HOME:36,INSERT:45,LEFT:37,MENU:93,NUMPAD_ADD:107,NUMPAD_DECIMAL:110,NUMPAD_DIVIDE:111,NUMPAD_ENTER:108,NUMPAD_MULTIPLY:106,NUMPAD_SUBTRACT:109,PAGE_DOWN:34,PAGE_UP:33,PERIOD:190,RIGHT:39,SHIFT:16,SPACE:32,TAB:9,UP:38,WINDOWS:91}});a.fn.extend({propAttr:a.fn.prop||a.fn.attr,_focus:a.fn.focus,focus:function(e,f){return typeof e===\"number\"?this.each(function(){var g=this;setTimeout(function(){a(g).focus();if(f){f.call(g)}},e)}):this._focus.apply(this,arguments)},scrollParent:function(){var e;if((a.browser.msie&&(/(static|relative)/).test(this.css(\"position\")))||(/absolute/).test(this.css(\"position\"))){e=this.parents().filter(function(){return(/(relative|absolute|fixed)/).test(a.curCSS(this,\"position\",1))&&(/(auto|scroll)/).test(a.curCSS(this,\"overflow\",1)+a.curCSS(this,\"overflow-y\",1)+a.curCSS(this,\"overflow-x\",1))}).eq(0)}else{e=this.parents().filter(function(){return(/(auto|scroll)/).test(a.curCSS(this,\"overflow\",1)+a.curCSS(this,\"overflow-y\",1)+a.curCSS(this,\"overflow-x\",1))}).eq(0)}return(/fixed/).test(this.css(\"position\"))||!e.length?a(document):e},zIndex:function(h){if(h!==d){return this.css(\"zIndex\",h)}if(this.length){var f=a(this[0]),e,g;while(f.length&&f[0]!==document){e=f.css(\"position\");if(e===\"absolute\"||e===\"relative\"||e===\"fixed\"){g=parseInt(f.css(\"zIndex\"),10);if(!isNaN(g)&&g!==0){return g}}f=f.parent()}}return 0},disableSelection:function(){return this.bind((a.support.selectstart?\"selectstart\":\"mousedown\")+\".ui-disableSelection\",function(e){e.preventDefault()})},enableSelection:function(){return this.unbind(\".ui-disableSelection\")}});a.each([\"Width\",\"Height\"],function(g,e){var f=e===\"Width\"?[\"Left\",\"Right\"]:[\"Top\",\"Bottom\"],h=e.toLowerCase(),k={innerWidth:a.fn.innerWidth,innerHeight:a.fn.innerHeight,outerWidth:a.fn.outerWidth,outerHeight:a.fn.outerHeight};function j(m,l,i,n){a.each(f,function(){l-=parseFloat(a.curCSS(m,\"padding\"+this,true))||0;if(i){l-=parseFloat(a.curCSS(m,\"border\"+this+\"Width\",true))||0}if(n){l-=parseFloat(a.curCSS(m,\"margin\"+this,true))||0}});return l}a.fn[\"inner\"+e]=function(i){if(i===d){return k[\"inner\"+e].call(this)}return this.each(function(){a(this).css(h,j(this,i)+\"px\")})};a.fn[\"outer\"+e]=function(i,l){if(typeof i!==\"number\"){return k[\"outer\"+e].call(this,i)}return this.each(function(){a(this).css(h,j(this,i,true,l)+\"px\")})}});function c(g,e){var j=g.nodeName.toLowerCase();if(\"area\"===j){var i=g.parentNode,h=i.name,f;if(!g.href||!h||i.nodeName.toLowerCase()!==\"map\"){return false}f=a(\"img[usemap=#\"+h+\"]\")[0];return !!f&&b(f)}return(/input|select|textarea|button|object/.test(j)?!g.disabled:\"a\"==j?g.href||e:e)&&b(g)}function b(e){return !a(e).parents().andSelf().filter(function(){return a.curCSS(this,\"visibility\")===\"hidden\"||a.expr.filters.hidden(this)}).length}a.extend(a.expr[\":\"],{data:function(g,f,e){return !!a.data(g,e[3])},focusable:function(e){return c(e,!isNaN(a.attr(e,\"tabindex\")))},tabbable:function(g){var e=a.attr(g,\"tabindex\"),f=isNaN(e);return(f||e>=0)&&c(g,!f)}});a(function(){var e=document.body,f=e.appendChild(f=document.createElement(\"div\"));f.offsetHeight;a.extend(f.style,{minHeight:\"100px\",height:\"auto\",padding:0,borderWidth:0});a.support.minHeight=f.offsetHeight===100;a.support.selectstart=\"onselectstart\" in f;e.removeChild(f).style.display=\"none\"});a.extend(a.ui,{plugin:{add:function(f,g,j){var h=a.ui[f].prototype;for(var e in j){h.plugins[e]=h.plugins[e]||[];h.plugins[e].push([g,j[e]])}},call:function(e,g,f){var j=e.plugins[g];if(!j||!e.element[0].parentNode){return}for(var h=0;h<j.length;h++){if(e.options[j[h][0]]){j[h][1].apply(e.element,f)}}}},contains:function(f,e){return document.compareDocumentPosition?f.compareDocumentPosition(e)&16:f!==e&&f.contains(e)},hasScroll:function(h,f){if(a(h).css(\"overflow\")===\"hidden\"){return false}var e=(f&&f===\"left\")?\"scrollLeft\":\"scrollTop\",g=false;if(h[e]>0){return true}h[e]=1;g=(h[e]>0);h[e]=0;return g},isOverAxis:function(f,e,g){return(f>e)&&(f<(e+g))},isOver:function(j,f,i,h,e,g){return a.ui.isOverAxis(j,i,e)&&a.ui.isOverAxis(f,h,g)}})})(jQuery);/*!\n"
" * jQuery UI Widget 1.8.18\n"
" *\n"
" * Copyright 2011, AUTHORS.txt (http://jqueryui.com/about)\n"
" * Dual licensed under the MIT or GPL Version 2 licenses.\n"
" * http://jquery.org/license\n"
" *\n"
" * http://docs.jquery.com/UI/Widget\n"
" */\n"
"(function(b,d){if(b.cleanData){var c=b.cleanData;b.cleanData=function(f){for(var g=0,h;(h=f[g])!=null;g++){try{b(h).triggerHandler(\"remove\")}catch(j){}}c(f)}}else{var a=b.fn.remove;b.fn.remove=function(e,f){return this.each(function(){if(!f){if(!e||b.filter(e,[this]).length){b(\"*\",this).add([this]).each(function(){try{b(this).triggerHandler(\"remove\")}catch(g){}})}}return a.call(b(this),e,f)})}}b.widget=function(f,h,e){var g=f.split(\".\")[0],j;f=f.split(\".\")[1];j=g+\"-\"+f;if(!e){e=h;h=b.Widget}b.expr[\":\"][j]=function(k){return !!b.data(k,f)};b[g]=b[g]||{};b[g][f]=function(k,l){if(arguments.length){this._createWidget(k,l)}};var i=new h();i.options=b.extend(true,{},i.options);b[g][f].prototype=b.extend(true,i,{namespace:g,widgetName:f,widgetEventPrefix:b[g][f].prototype.widgetEventPrefix||f,widgetBaseClass:j},e);b.widget.bridge(f,b[g][f])};b.widget.bridge=function(f,e){b.fn[f]=function(i){var g=typeof i===\"string\",h=Array.prototype.slice.call(arguments,1),j=this;i=!g&&h.length?b.extend.apply(null,[true,i].concat(h)):i;if(g&&i.charAt(0)===\"_\"){return j}if(g){this.each(function(){var k=b.data(this,f),l=k&&b.isFunction(k[i])?k[i].apply(k,h):k;if(l!==k&&l!==d){j=l;return false}})}else{this.each(function(){var k=b.data(this,f);if(k){k.option(i||{})._init()}else{b.data(this,f,new e(i,this))}})}return j}};b.Widget=function(e,f){if(arguments.length){this._createWidget(e,f)}};b.Widget.prototype={widgetName:\"widget\",widgetEventPrefix:\"\",options:{disabled:false},_createWidget:function(f,g){b.data(g,this.widgetName,this);this.element=b(g);this.options=b.extend(true,{},this.options,this._getCreateOptions(),f);var e=this;this.element.bind(\"remove.\"+this.widgetName,function(){e.destroy()});this._create();this._trigger(\"create\");this._init()},_getCreateOptions:function(){return b.metadata&&b.metadata.get(this.element[0])[this.widgetName]},_create:function(){},_init:function(){},destroy:function(){this.element.unbind(\".\"+this.widgetName).removeData(this.widgetName);this.widget().unbind(\".\"+this.widgetName).removeAttr(\"aria-disabled\").removeClass(this.widgetBaseClass+\"-disabled ui-state-disabled\")},widget:function(){return this.element},option:function(f,g){var e=f;if(arguments.length===0){return b.extend({},this.options)}if(typeof f===\"string\"){if(g===d){return this.options[f]}e={};e[f]=g}this._setOptions(e);return this},_setOptions:function(f){var e=this;b.each(f,function(g,h){e._setOption(g,h)});return this},_setOption:function(e,f){this.options[e]=f;if(e===\"disabled\"){this.widget()[f?\"addClass\":\"removeClass\"](this.widgetBaseClass+\"-disabled ui-state-disabled\").attr(\"aria-disabled\",f)}return this},enable:function(){return this._setOption(\"disabled\",false)},disable:function(){return this._setOption(\"disabled\",true)},_trigger:function(e,f,g){var j,i,h=this.options[e];g=g||{};f=b.Event(f);f.type=(e===this.widgetEventPrefix?e:this.widgetEventPrefix+e).toLowerCase();f.target=this.element[0];i=f.originalEvent;if(i){for(j in i){if(!(j in f)){f[j]=i[j]}}}this.element.trigger(f,g);return !(b.isFunction(h)&&h.call(this.element[0],f,g)===false||f.isDefaultPrevented())}}})(jQuery);/*!\n"
" * jQuery UI Mouse 1.8.18\n"
" *\n"
" * Copyright 2011, AUTHORS.txt (http://jqueryui.com/about)\n"
" * Dual licensed under the MIT or GPL Version 2 licenses.\n"
" * http://jquery.org/license\n"
" *\n"
" * http://docs.jquery.com/UI/Mouse\n"
" *\n"
" * Depends:\n"
" *	jquery.ui.widget.js\n"
" */\n"
"(function(b,c){var a=false;b(document).mouseup(function(d){a=false});b.widget(\"ui.mouse\",{options:{cancel:\":input,option\",distance:1,delay:0},_mouseInit:function(){var d=this;this.element.bind(\"mousedown.\"+this.widgetName,function(e){return d._mouseDown(e)}).bind(\"click.\"+this.widgetName,function(e){if(true===b.data(e.target,d.widgetName+\".preventClickEvent\")){b.removeData(e.target,d.widgetName+\".preventClickEvent\");e.stopImmediatePropagation();return false}});this.started=false},_mouseDestroy:function(){this.element.unbind(\".\"+this.widgetName)},_mouseDown:function(f){if(a){return}(this._mouseStarted&&this._mouseUp(f));this._mouseDownEvent=f;var e=this,g=(f.which==1),d=(typeof this.options.cancel==\"string\"&&f.target.nodeName?b(f.target).closest(this.options.cancel).length:false);if(!g||d||!this._mouseCapture(f)){return true}this.mouseDelayMet=!this.options.delay;if(!this.mouseDelayMet){this._mouseDelayTimer=setTimeout(function(){e.mouseDelayMet=true},this.options.delay)}if(this._mouseDistanceMet(f)&&this._mouseDelayMet(f)){this._mouseStarted=(this._mouseStart(f)!==false);if(!this._mouseStarted){f.preventDefault();return true}}if(true===b.data(f.target,this.widgetName+\".preventClickEvent\")){b.removeData(f.target,this.widgetName+\".preventClickEvent\")}this._mouseMoveDelegate=function(h){return e._mouseMove(h)};this._mouseUpDelegate=function(h){return e._mouseUp(h)};b(document).bind(\"mousemove.\"+this.widgetName,this._mouseMoveDelegate).bind(\"mouseup.\"+this.widgetName,this._mouseUpDelegate);f.preventDefault();a=true;return true},_mouseMove:function(d){if(b.browser.msie&&!(document.documentMode>=9)&&!d.button){return this._mouseUp(d)}if(this._mouseStarted){this._mouseDrag(d);return d.preventDefault()}if(this._mouseDistanceMet(d)&&this._mouseDelayMet(d)){this._mouseStarted=(this._mouseStart(this._mouseDownEvent,d)!==false);(this._mouseStarted?this._mouseDrag(d):this._mouseUp(d))}return !this._mouseStarted},_mouseUp:function(d){b(document).unbind(\"mousemove.\"+this.widgetName,this._mouseMoveDelegate).unbind(\"mouseup.\"+this.widgetName,this._mouseUpDelegate);if(this._mouseStarted){this._mouseStarted=false;if(d.target==this._mouseDownEvent.target){b.data(d.target,this.widgetName+\".preventClickEvent\",true)}this._mouseStop(d)}return false},_mouseDistanceMet:function(d){return(Math.max(Math.abs(this._mouseDownEvent.pageX-d.pageX),Math.abs(this._mouseDownEvent.pageY-d.pageY))>=this.options.distance)},_mouseDelayMet:function(d){return this.mouseDelayMet},_mouseStart:function(d){},_mouseDrag:function(d){},_mouseStop:function(d){},_mouseCapture:function(d){return true}})})(jQuery);(function(c,d){c.widget(\"ui.resizable\",c.ui.mouse,{widgetEventPrefix:\"resize\",options:{alsoResize:false,animate:false,animateDuration:\"slow\",animateEasing:\"swing\",aspectRatio:false,autoHide:false,containment:false,ghost:false,grid:false,handles:\"e,s,se\",helper:false,maxHeight:null,maxWidth:null,minHeight:10,minWidth:10,zIndex:1000},_create:function(){var f=this,k=this.options;this.element.addClass(\"ui-resizable\");c.extend(this,{_aspectRatio:!!(k.aspectRatio),aspectRatio:k.aspectRatio,originalElement:this.element,_proportionallyResizeElements:[],_helper:k.helper||k.ghost||k.animate?k.helper||\"ui-resizable-helper\":null});if(this.element[0].nodeName.match(/canvas|textarea|input|select|button|img/i)){this.element.wrap(c('<div class=\"ui-wrapper\" style=\"overflow: hidden;\"></div>').css({position:this.element.css(\"position\"),width:this.element.outerWidth(),height:this.element.outerHeight(),top:this.element.css(\"top\"),left:this.element.css(\"left\")}));this.element=this.element.parent().data(\"resizable\",this.element.data(\"resizable\"));this.elementIsWrapper=true;this.element.css({marginLeft:this.originalElement.css(\"marginLeft\"),marginTop:this.originalElement.css(\"marginTop\"),marginRight:this.originalElement.css(\"marginRight\"),marginBottom:this.originalElement.css(\"marginBottom\")});this.originalElement.css({marginLeft:0,marginTop:0,marginRight:0,marginBottom:0});this.originalResizeStyle=this.originalElement.css(\"resize\");this.originalElement.css(\"resize\",\"none\");this._proportionallyResizeElements.push(this.originalElement.css({position:\"static\",zoom:1,display:\"block\"}));this.originalElement.css({margin:this.originalElement.css(\"margin\")});this._proportionallyResize()}this.handles=k.handles||(!c(\".ui-resizable-handle\",this.element).length?\"e,s,se\":{n:\".ui-resizable-n\",e:\".ui-resizable-e\",s:\".ui-resizable-s\",w:\".ui-resizable-w\",se:\".ui-resizable-se\",sw:\".ui-resizable-sw\",ne:\".ui-resizable-ne\",nw:\".ui-resizable-nw\"});if(this.handles.constructor==String){if(this.handles==\"all\"){this.handles=\"n,e,s,w,se,sw,ne,nw\"}var l=this.handles.split(\",\");this.handles={};for(var g=0;g<l.length;g++){var j=c.trim(l[g]),e=\"ui-resizable-\"+j;var h=c('<div class=\"ui-resizable-handle '+e+'\"></div>');if(/sw|se|ne|nw/.test(j)){h.css({zIndex:++k.zIndex})}if(\"se\"==j){h.addClass(\"ui-icon ui-icon-gripsmall-diagonal-se\")}this.handles[j]=\".ui-resizable-\"+j;this.element.append(h)}}this._renderAxis=function(q){q=q||this.element;for(var n in this.handles){if(this.handles[n].constructor==String){this.handles[n]=c(this.handles[n],this.element).show()}if(this.elementIsWrapper&&this.originalElement[0].nodeName.match(/textarea|input|select|button/i)){var o=c(this.handles[n],this.element),p=0;p=/sw|ne|nw|se|n|s/.test(n)?o.outerHeight():o.outerWidth();var m=[\"padding\",/ne|nw|n/.test(n)?\"Top\":/se|sw|s/.test(n)?\"Bottom\":/^e$/.test(n)?\"Right\":\"Left\"].join(\"\");q.css(m,p);this._proportionallyResize()}if(!c(this.handles[n]).length){continue}}};this._renderAxis(this.element);this._handles=c(\".ui-resizable-handle\",this.element).disableSelection();this._handles.mouseover(function(){if(!f.resizing){if(this.className){var i=this.className.match(/ui-resizable-(se|sw|ne|nw|n|e|s|w)/i)}f.axis=i&&i[1]?i[1]:\"se\"}});if(k.autoHide){this._handles.hide();c(this.element).addClass(\"ui-resizable-autohide\").hover(function(){if(k.disabled){return}c(this).removeClass(\"ui-resizable-autohide\");f._handles.show()},function(){if(k.disabled){return}if(!f.resizing){c(this).addClass(\"ui-resizable-autohide\");f._handles.hide()}})}this._mouseInit()},destroy:function(){this._mouseDestroy();var e=function(g){c(g).removeClass(\"ui-resizable ui-resizable-disabled ui-resizable-resizing\").removeData(\"resizable\").unbind(\".resizable\").find(\".ui-resizable-handle\").remove()};if(this.elementIsWrapper){e(this.element);var f=this.element;f.after(this.originalElement.css({position:f.css(\"position\"),width:f.outerWidth(),height:f.outerHeight(),top:f.css(\"top\"),left:f.css(\"left\")})).remove()}this.originalElement.css(\"resize\",this.originalResizeStyle);e(this.originalElement);return this},_mouseCapture:function(f){var g=false;for(var e in this.handles){if(c(this.handles[e])[0]==f.target){g=true}}return !this.options.disabled&&g},_mouseStart:function(g){var j=this.options,f=this.element.position(),e=this.element;this.resizing=true;this.documentScroll={top:c(document).scrollTop(),left:c(document).scrollLeft()};if(e.is(\".ui-draggable\")||(/absolute/).test(e.css(\"position\"))){e.css({position:\"absolute\",top:f.top,left:f.left})}this._renderProxy();var k=b(this.helper.css(\"left\")),h=b(this.helper.css(\"top\"));if(j.containment){k+=c(j.containment).scrollLeft()||0;h+=c(j.containment).scrollTop()||0}this.offset=this.helper.offset();this.position={left:k,top:h};this.size=this._helper?{width:e.outerWidth(),height:e.outerHeight()}:{width:e.width(),height:e.height()};this.originalSize=this._helper?{width:e.outerWidth(),height:e.outerHeight()}:{width:e.width(),height:e.height()};this.originalPosition={left:k,top:h};this.sizeDiff={width:e.outerWidth()-e.width(),height:e.outerHeight()-e.height()};this.originalMousePosition={left:g.pageX,top:g.pageY};this.aspectRatio=(typeof j.aspectRatio==\"number\")?j.aspectRatio:((this.originalSize.width/this.originalSize.height)||1);var i=c(\".ui-resizable-\"+this.axis).css(\"cursor\");c(\"body\").css(\"cursor\",i==\"auto\"?this.axis+\"-resize\":i);e.addClass(\"ui-resizable-resizing\");this._propagate(\"start\",g);return true},_mouseDrag:function(e){var h=this.helper,g=this.options,m={},q=this,j=this.originalMousePosition,n=this.axis;var r=(e.pageX-j.left)||0,p=(e.pageY-j.top)||0;var i=this._change[n];if(!i){return false}var l=i.apply(this,[e,r,p]),k=c.browser.msie&&c.browser.version<7,f=this.sizeDiff;this._updateVirtualBoundaries(e.shiftKey);if(this._aspectRatio||e.shiftKey){l=this._updateRatio(l,e)}l=this._respectSize(l,e);this._propagate(\"resize\",e);h.css({top:this.position.top+\"px\",left:this.position.left+\"px\",width:this.size.width+\"px\",height:this.size.height+\"px\"});if(!this._helper&&this._proportionallyResizeElements.length){this._proportionallyResize()}this._updateCache(l);this._trigger(\"resize\",e,this.ui());return false},_mouseStop:function(h){this.resizing=false;var i=this.options,m=this;if(this._helper){var g=this._proportionallyResizeElements,e=g.length&&(/textarea/i).test(g[0].nodeName),f=e&&c.ui.hasScroll(g[0],\"left\")?0:m.sizeDiff.height,k=e?0:m.sizeDiff.width;var n={width:(m.helper.width()-k),height:(m.helper.height()-f)},j=(parseInt(m.element.css(\"left\"),10)+(m.position.left-m.originalPosition.left))||null,l=(parseInt(m.element.css(\"top\"),10)+(m.position.top-m.originalPosition.top))||null;if(!i.animate){this.element.css(c.extend(n,{top:l,left:j}))}m.helper.height(m.size.height);m.helper.width(m.size.width);if(this._helper&&!i.animate){this._proportionallyResize()}}c(\"body\").css(\"cursor\",\"auto\");this.element.removeClass(\"ui-resizable-resizing\");this._propagate(\"stop\",h);if(this._helper){this.helper.remove()}return false},_updateVirtualBoundaries:function(g){var j=this.options,i,h,f,k,e;e={minWidth:a(j.minWidth)?j.minWidth:0,maxWidth:a(j.maxWidth)?j.maxWidth:Infinity,minHeight:a(j.minHeight)?j.minHeight:0,maxHeight:a(j.maxHeight)?j.maxHeight:Infinity};if(this._aspectRatio||g){i=e.minHeight*this.aspectRatio;f=e.minWidth/this.aspectRatio;h=e.maxHeight*this.aspectRatio;k=e.maxWidth/this.aspectRatio;if(i>e.minWidth){e.minWidth=i}if(f>e.minHeight){e.minHeight=f}if(h<e.maxWidth){e.maxWidth=h}if(k<e.maxHeight){e.maxHeight=k}}this._vBoundaries=e},_updateCache:function(e){var f=this.options;this.offset=this.helper.offset();if(a(e.left)){this.position.left=e.left}if(a(e.top)){this.position.top=e.top}if(a(e.height)){this.size.height=e.height}if(a(e.width)){this.size.width=e.width}},_updateRatio:function(h,g){var i=this.options,j=this.position,f=this.size,e=this.axis;if(a(h.height)){h.width=(h.height*this.aspectRatio)}else{if(a(h.width)){h.height=(h.width/this.aspectRatio)}}if(e==\"sw\"){h.left=j.left+(f.width-h.width);h.top=null}if(e==\"nw\"){h.top=j.top+(f.height-h.height);h.left=j.left+(f.width-h.width)}return h},_respectSize:function(l,g){var j=this.helper,i=this._vBoundaries,r=this._aspectRatio||g.shiftKey,q=this.axis,t=a(l.width)&&i.maxWidth&&(i.maxWidth<l.width),m=a(l.height)&&i.maxHeight&&(i.maxHeight<l.height),h=a(l.width)&&i.minWidth&&(i.minWidth>l.width),s=a(l.height)&&i.minHeight&&(i.minHeight>l.height);if(h){l.width=i.minWidth}if(s){l.height=i.minHeight}if(t){l.width=i.maxWidth}if(m){l.height=i.maxHeight}var f=this.originalPosition.left+this.originalSize.width,p=this.position.top+this.size.height;var k=/sw|nw|w/.test(q),e=/nw|ne|n/.test(q);if(h&&k){l.left=f-i.minWidth}if(t&&k){l.left=f-i.maxWidth}if(s&&e){l.top=p-i.minHeight}if(m&&e){l.top=p-i.maxHeight}var n=!l.width&&!l.height;if(n&&!l.left&&l.top){l.top=null}else{if(n&&!l.top&&l.left){l.left=null}}return l},_proportionallyResize:function(){var k=this.options;if(!this._proportionallyResizeElements.length){return}var g=this.helper||this.element;for(var f=0;f<this._proportionallyResizeElements.length;f++){var h=this._proportionallyResizeElements[f];if(!this.borderDif){var e=[h.css(\"borderTopWidth\"),h.css(\"borderRightWidth\"),h.css(\"borderBottomWidth\"),h.css(\"borderLeftWidth\")],j=[h.css(\"paddingTop\"),h.css(\"paddingRight\"),h.css(\"paddingBottom\"),h.css(\"paddingLeft\")];this.borderDif=c.map(e,function(l,n){var m=parseInt(l,10)||0,o=parseInt(j[n],10)||0;return m+o})}if(c.browser.msie&&!(!(c(g).is(\":hidden\")||c(g).parents(\":hidden\").length))){continue}h.css({height:(g.height()-this.borderDif[0]-this.borderDif[2])||0,width:(g.width()-this.borderDif[1]-this.borderDif[3])||0})}},_renderProxy:function(){var f=this.element,i=this.options;this.elementOffset=f.offset();if(this._helper){this.helper=this.helper||c('<div style=\"overflow:hidden;\"></div>');var e=c.browser.msie&&c.browser.version<7,g=(e?1:0),h=(e?2:-1);this.helper.addClass(this._helper).css({width:this.element.outerWidth()+h,height:this.element.outerHeight()+h,position:\"absolute\",left:this.elementOffset.left-g+\"px\",top:this.elementOffset.top-g+\"px\",zIndex:++i.zIndex});this.helper.appendTo(\"body\").disableSelection()}else{this.helper=this.element}},_change:{e:function(g,f,e){return{width:this.originalSize.width+f}},w:function(h,f,e){var j=this.options,g=this.originalSize,i=this.originalPosition;return{left:i.left+f,width:g.width-f}},n:function(h,f,e){var j=this.options,g=this.originalSize,i=this.originalPosition;return{top:i.top+e,height:g.height-e}},s:function(g,f,e){return{height:this.originalSize.height+e}},se:function(g,f,e){return c.extend(this._change.s.apply(this,arguments),this._change.e.apply(this,[g,f,e]))},sw:function(g,f,e){return c.extend(this._change.s.apply(this,arguments),this._change.w.apply(this,[g,f,e]))},ne:function(g,f,e){return c.extend(this._change.n.apply(this,arguments),this._change.e.apply(this,[g,f,e]))},nw:function(g,f,e){return c.extend(this._change.n.apply(this,arguments),this._change.w.apply(this,[g,f,e]))}},_propagate:function(f,e){c.ui.plugin.call(this,f,[e,this.ui()]);(f!=\"resize\"&&this._trigger(f,e,this.ui()))},plugins:{},ui:function(){return{originalElement:this.originalElement,element:this.element,helper:this.helper,position:this.position,size:this.size,originalSize:this.originalSize,originalPosition:this.originalPosition}}});c.extend(c.ui.resizable,{version:\"1.8.18\"});c.ui.plugin.add(\"resizable\",\"alsoResize\",{start:function(f,g){var e=c(this).data(\"resizable\"),i=e.options;var h=function(j){c(j).each(function(){var k=c(this);k.data(\"resizable-alsoresize\",{width:parseInt(k.width(),10),height:parseInt(k.height(),10),left:parseInt(k.css(\"left\"),10),top:parseInt(k.css(\"top\"),10)})})};if(typeof(i.alsoResize)==\"object\"&&!i.alsoResize.parentNode){if(i.alsoResize.length){i.alsoResize=i.alsoResize[0];h(i.alsoResize)}else{c.each(i.alsoResize,function(j){h(j)})}}else{h(i.alsoResize)}},resize:function(g,i){var f=c(this).data(\"resizable\"),j=f.options,h=f.originalSize,l=f.originalPosition;var k={height:(f.size.height-h.height)||0,width:(f.size.width-h.width)||0,top:(f.position.top-l.top)||0,left:(f.position.left-l.left)||0},e=function(m,n){c(m).each(function(){var q=c(this),r=c(this).data(\"resizable-alsoresize\"),p={},o=n&&n.length?n:q.parents(i.originalElement[0]).length?[\"width\",\"height\"]:[\"width\",\"height\",\"top\",\"left\"];c.each(o,function(s,u){var t=(r[u]||0)+(k[u]||0);if(t&&t>=0){p[u]=t||null}});q.css(p)})};if(typeof(j.alsoResize)==\"object\"&&!j.alsoResize.nodeType){c.each(j.alsoResize,function(m,n){e(m,n)})}else{e(j.alsoResize)}},stop:function(e,f){c(this).removeData(\"resizable-alsoresize\")}});c.ui.plugin.add(\"resizable\",\"animate\",{stop:function(i,n){var p=c(this).data(\"resizable\"),j=p.options;var h=p._proportionallyResizeElements,e=h.length&&(/textarea/i).test(h[0].nodeName),f=e&&c.ui.hasScroll(h[0],\"left\")?0:p.sizeDiff.height,l=e?0:p.sizeDiff.width;var g={width:(p.size.width-l),height:(p.size.height-f)},k=(parseInt(p.element.css(\"left\"),10)+(p.position.left-p.originalPosition.left))||null,m=(parseInt(p.element.css(\"top\"),10)+(p.position.top-p.originalPosition.top))||null;\n"
"p.element.animate(c.extend(g,m&&k?{top:m,left:k}:{}),{duration:j.animateDuration,easing:j.animateEasing,step:function(){var o={width:parseInt(p.element.css(\"width\"),10),height:parseInt(p.element.css(\"height\"),10),top:parseInt(p.element.css(\"top\"),10),left:parseInt(p.element.css(\"left\"),10)};if(h&&h.length){c(h[0]).css({width:o.width,height:o.height})}p._updateCache(o);p._propagate(\"resize\",i)}})}});c.ui.plugin.add(\"resizable\",\"containment\",{start:function(f,r){var t=c(this).data(\"resizable\"),j=t.options,l=t.element;var g=j.containment,k=(g instanceof c)?g.get(0):(/parent/.test(g))?l.parent().get(0):g;if(!k){return}t.containerElement=c(k);if(/document/.test(g)||g==document){t.containerOffset={left:0,top:0};t.containerPosition={left:0,top:0};t.parentData={element:c(document),left:0,top:0,width:c(document).width(),height:c(document).height()||document.body.parentNode.scrollHeight}}else{var n=c(k),i=[];c([\"Top\",\"Right\",\"Left\",\"Bottom\"]).each(function(p,o){i[p]=b(n.css(\"padding\"+o))});t.containerOffset=n.offset();t.containerPosition=n.position();t.containerSize={height:(n.innerHeight()-i[3]),width:(n.innerWidth()-i[1])};var q=t.containerOffset,e=t.containerSize.height,m=t.containerSize.width,h=(c.ui.hasScroll(k,\"left\")?k.scrollWidth:m),s=(c.ui.hasScroll(k)?k.scrollHeight:e);t.parentData={element:k,left:q.left,top:q.top,width:h,height:s}}},resize:function(g,q){var t=c(this).data(\"resizable\"),i=t.options,f=t.containerSize,p=t.containerOffset,m=t.size,n=t.position,r=t._aspectRatio||g.shiftKey,e={top:0,left:0},h=t.containerElement;if(h[0]!=document&&(/static/).test(h.css(\"position\"))){e=p}if(n.left<(t._helper?p.left:0)){t.size.width=t.size.width+(t._helper?(t.position.left-p.left):(t.position.left-e.left));if(r){t.size.height=t.size.width/i.aspectRatio}t.position.left=i.helper?p.left:0}if(n.top<(t._helper?p.top:0)){t.size.height=t.size.height+(t._helper?(t.position.top-p.top):t.position.top);if(r){t.size.width=t.size.height*i.aspectRatio}t.position.top=t._helper?p.top:0}t.offset.left=t.parentData.left+t.position.left;t.offset.top=t.parentData.top+t.position.top;var l=Math.abs((t._helper?t.offset.left-e.left:(t.offset.left-e.left))+t.sizeDiff.width),s=Math.abs((t._helper?t.offset.top-e.top:(t.offset.top-p.top))+t.sizeDiff.height);var k=t.containerElement.get(0)==t.element.parent().get(0),j=/relative|absolute/.test(t.containerElement.css(\"position\"));if(k&&j){l-=t.parentData.left}if(l+t.size.width>=t.parentData.width){t.size.width=t.parentData.width-l;if(r){t.size.height=t.size.width/t.aspectRatio}}if(s+t.size.height>=t.parentData.height){t.size.height=t.parentData.height-s;if(r){t.size.width=t.size.height*t.aspectRatio}}},stop:function(f,n){var q=c(this).data(\"resizable\"),g=q.options,l=q.position,m=q.containerOffset,e=q.containerPosition,i=q.containerElement;var j=c(q.helper),r=j.offset(),p=j.outerWidth()-q.sizeDiff.width,k=j.outerHeight()-q.sizeDiff.height;if(q._helper&&!g.animate&&(/relative/).test(i.css(\"position\"))){c(this).css({left:r.left-e.left-m.left,width:p,height:k})}if(q._helper&&!g.animate&&(/static/).test(i.css(\"position\"))){c(this).css({left:r.left-e.left-m.left,width:p,height:k})}}});c.ui.plugin.add(\"resizable\",\"ghost\",{start:function(g,h){var e=c(this).data(\"resizable\"),i=e.options,f=e.size;e.ghost=e.originalElement.clone();e.ghost.css({opacity:0.25,display:\"block\",position:\"relative\",height:f.height,width:f.width,margin:0,left:0,top:0}).addClass(\"ui-resizable-ghost\").addClass(typeof i.ghost==\"string\"?i.ghost:\"\");e.ghost.appendTo(e.helper)},resize:function(f,g){var e=c(this).data(\"resizable\"),h=e.options;if(e.ghost){e.ghost.css({position:\"relative\",height:e.size.height,width:e.size.width})}},stop:function(f,g){var e=c(this).data(\"resizable\"),h=e.options;if(e.ghost&&e.helper){e.helper.get(0).removeChild(e.ghost.get(0))}}});c.ui.plugin.add(\"resizable\",\"grid\",{resize:function(e,m){var p=c(this).data(\"resizable\"),h=p.options,k=p.size,i=p.originalSize,j=p.originalPosition,n=p.axis,l=h._aspectRatio||e.shiftKey;h.grid=typeof h.grid==\"number\"?[h.grid,h.grid]:h.grid;var g=Math.round((k.width-i.width)/(h.grid[0]||1))*(h.grid[0]||1),f=Math.round((k.height-i.height)/(h.grid[1]||1))*(h.grid[1]||1);if(/^(se|s|e)$/.test(n)){p.size.width=i.width+g;p.size.height=i.height+f}else{if(/^(ne)$/.test(n)){p.size.width=i.width+g;p.size.height=i.height+f;p.position.top=j.top-f}else{if(/^(sw)$/.test(n)){p.size.width=i.width+g;p.size.height=i.height+f;p.position.left=j.left-g}else{p.size.width=i.width+g;p.size.height=i.height+f;p.position.top=j.top-f;p.position.left=j.left-g}}}}});var b=function(e){return parseInt(e,10)||0};var a=function(e){return !isNaN(parseInt(e,10))}})(jQuery);/*!\n"
" * jQuery hashchange event - v1.3 - 7/21/2010\n"
" * http://benalman.com/projects/jquery-hashchange-plugin/\n"
" * \n"
" * Copyright (c) 2010 \"Cowboy\" Ben Alman\n"
" * Dual licensed under the MIT and GPL licenses.\n"
" * http://benalman.com/about/license/\n"
" */\n"
"(function($,e,b){var c=\"hashchange\",h=document,f,g=$.event.special,i=h.documentMode,d=\"on\"+c in e&&(i===b||i>7);function a(j){j=j||location.href;return\"#\"+j.replace(/^[^#]*#?(.*)$/,\"$1\")}$.fn[c]=function(j){return j?this.bind(c,j):this.trigger(c)};$.fn[c].delay=50;g[c]=$.extend(g[c],{setup:function(){if(d){return false}$(f.start)},teardown:function(){if(d){return false}$(f.stop)}});f=(function(){var j={},p,m=a(),k=function(q){return q},l=k,o=k;j.start=function(){p||n()};j.stop=function(){p&&clearTimeout(p);p=b};function n(){var r=a(),q=o(m);if(r!==m){l(m=r,q);$(e).trigger(c)}else{if(q!==m){location.href=location.href.replace(/#.*/,\"\")+q}}p=setTimeout(n,$.fn[c].delay)}$.browser.msie&&!d&&(function(){var q,r;j.start=function(){if(!q){r=$.fn[c].src;r=r&&r+a();q=$('<iframe tabindex=\"-1\" title=\"empty\"/>').hide().one(\"load\",function(){r||l(a());n()}).attr(\"src\",r||\"javascript:0\").insertAfter(\"body\")[0].contentWindow;h.onpropertychange=function(){try{if(event.propertyName===\"title\"){q.document.title=h.title}}catch(s){}}}};j.stop=k;o=function(){return a(q.location.href)};l=function(v,s){var u=q.document,t=$.fn[c].domain;if(v!==s){u.title=h.title;u.open();t&&u.write('<script>document.domain=\"'+t+'\"<\\/script>');u.close();q.location.hash=v}}})();return j})()})(jQuery,this);\n"
