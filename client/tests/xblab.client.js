var net = require('net'),
    util = require('util'),
    xblab = require('./xblab.wrapper'),
    path = require('path');

exports.Client = xblabClient;

function sendCallback(err) {
    if (err) console.error("send() error: " + err);
}


// TODO: cleanup and expose more to clients
function xblabClient (cfg, ws){
    var self = this;
    self.wsClient = ws;

    // TODO: move!!
    cfg.pubKeyFile = path.join(process.cwd(), cfg.pubKeyFile);
    xblab.config(cfg); //should be async, not hard coded in ctor

    self.participant = new xblab.Participant();

    self.socket = net.connect({port: cfg.remotePort},
        function() {
            console.log('xblab relay client connected');
        });

    self.participant.on('cred', function(buf){
        console.log(buf);

        // TODO: get credentials from the user
        self.wsClient.send(JSON.stringify({status: 'connected', state: 'NEEDCRED'}));
    });

    self.socket.on('data', function(data) {
        self.participant.digestBuffer(data, function(err){
            console.log(err);
        });
        
    });

    self.wsClient.on('message', function (message) {
        if (message.type === 'utf8') {
            if (cfg.debug){
                console.log("Received utf-8 message of %s characters.",
                    message.utf8Data.length);
            }
        }
        else if (message.type === 'binary') {
            if (cfg.debug){
                console.log("Received binary message of %s bytes.",
                    message.binaryData.length);
            }
        }
    });

    self.wsClient.on('close', function(reasonCode, description) {
        if (cfg.debug){
            console.log('%s xblab presentation client at\n%s disconnected...\n\
                Why am I still running?', new Date(), self.wsClient.remoteAddress);
        }
    });    
    
    self.wsClient.on('error', function(error) {
        console.log('Connection error for xblab presentation client at %s.\n Details: %s',
        self.wsClient.remoteAddress, error);
    });

    self.socket.on('end', function() {
        console.log('xblab relay client disconnected');
    });
};


//                
