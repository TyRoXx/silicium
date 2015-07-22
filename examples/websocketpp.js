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
