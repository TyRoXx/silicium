/*
This is actually a C++ header file that contains a raw string literal with Javascript inside.
The .js extension enables Javascript syntax highlighting in IDEs automatically, which is nice.
The .cpp file #includes this file to add it to the HTML document.
*/
R"QQQ"(

var uri = ((window.location.protocol === "https:") ? "wss://" : "ws://") + window.location.hostname + (((window.location.port != 80) && (window.location.port != 443)) ? ":" + window.location.port : "") + window.location.pathname;
var socket = new WebSocket(uri);
socket.onopen = function (e) {
    socket.send("Hello server");
};
socket.onerror = function (e) {
    alert(e);
};
socket.onmessage = function (e) {
    alert(e.data);
};
socket.onclose = function (e) {
    alert("Goodbye server");
};

)QQQ""
