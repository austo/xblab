var constants = require('../../public/js/constants'),
    WebSocketClient = require('websocket').client;


exports.handleWsRequest = function(request) {
    var debug = true;

    //We only want connections from the xblab client    
    if (request.origin.replace(/^https?:\/\//, '') === request.webSocketRequest.host){

        var connection = request.accept(request.origin);

        //Add a websocket client for connection to xblab server
        connection.client = new WebSocketClient();

        console.log((new Date()) + " " + request.protocol +  
            " connection accepted from " + request.origin + " at " + connection.remoteAddress + 
                    " - Protocol Version " + connection.webSocketVersion);

        function sendCallback(err) {
            if (err) console.error("send() error: " + err);
        }

        connection.on('message', function(message) {
            //If client isn't connected, connect and transmit message (use readyState === 1)
            console.log(connection.client);
            if (message.type === 'utf8') {
                if (debug){
                    console.log("Received utf-8 message of " + message.utf8Data.length + " characters.");
                    console.log("message: ", message.utf8Data);
                }
                connection.sendUTF(message.utf8Data, sendCallback);
            }
            else if (message.type === 'binary') {
                if (debug){
                    console.log("Received Binary Message of " + message.binaryData.length + " bytes");
                }
                connection.sendBytes(message.binaryData, sendCallback);
            }
        });

        connection.on('close', function(reasonCode, description) {
            if (debug){
                console.log((new Date()) + " Peer " + connection.remoteAddress + " disconnected.");
            }
        });    
        
        connection.on('error', function(error) {
            console.log("Connection error for peer " + connection.remoteAddress + ": " + error);
        });
    }
    else{
        request.reject(403, "Bad origin");
    }
};