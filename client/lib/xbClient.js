var util = require('util'),
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

  cfg.group = ws.group;

  self.client = new xblab.XbClient(cfg);

  // referenced object don't pollute! 
  delete cfg.group;

  self.client.connect(function(err){
    if (err) {
      console.log(err);
    }
    else {
      console.log('xblab relay client connected');
    }
  });

  self.client.on('needCred', function (buf){
    console.log(buf);

    // TODO: get credentials from the user
    self.wsClient.send(JSON.stringify({status: 'connected', state: 'NEEDCRED'}));
  });

  // self.client.on('haveCred', function (buf){
  //     console.log(buf);
  //     self.socket.write(buf);
  //     // TODO: get credentials from the user
  //     // self.wsClient.send(JSON.stringify({status: 'connected', state: 'NEEDCRED'}));
  // });

  // self.socket.on('data', function (data) {
  //     self.client.digestBuffer(data, function (err){
  //         console.log(err);
  //     });        
  // });

  // self.wsClient.on('message', function (message) {
  //     if (message.type === 'utf8') {
  //         if (cfg.debug){
  //             console.log("Received utf-8 message of %s characters. Message: '%s'.",
  //                 message.utf8Data.length, message.utf8Data);
  //         }
  //         var userMessage = JSON.parse(message.utf8Data);
  //         if (userMessage && userMessage.type){
  //             switch(userMessage.type){
  //                 case 'CRED':
  //                     self.client.sendCred(userMessage, function (err){
  //                         console.log(err);
  //                     });
  //                     break;      
  //             }
  //         }            
  //     }
  //     // This should never happen as we are using JSON between local server and page
  //     else if (message.type === 'binary') {
  //         if (cfg.debug){
  //             console.log("Received binary message of %s bytes.",
  //                 message.binaryData.length);
  //         }
  //     }
  // });

  self.wsClient.on('close', function (reasonCode, description) {
    if (cfg.debug){
      console.log('%s xblab presentation client at\n%s disconnected...\n\
        Why am I still running?', new Date(), self.wsClient.remoteAddress);
    }
  });    
  
  self.wsClient.on('error', function (error) {
    console.log('Connection error for xblab presentation client at %s.\n Details: %s',
    self.wsClient.remoteAddress, error);
  });

  self.client.on('end', function () {
    console.log('xblab relay client disconnected');        
  });

  self.client.on('error', function(err){
    console.log(err);
  })
};


//                
