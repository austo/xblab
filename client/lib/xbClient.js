var util = require('util'),
  xblab = require('./xblab.wrapper'),
  path = require('path'),
  constants = require('../public/js/constants');

exports.Client = xbClient;

function sendCallback(err) {
  if (err) console.error("send() error: " + err);
}

// TODO: cleanup and expose more to clients
function xbClient (cfg, ws){
  var self = this;
  self.wsClient = ws;

  cfg.group = ws.group;

  self.xbClient = new xblab.Client(cfg);

  // Shared by any connection this user
  // happens to make. Don't pollute group field.
  delete cfg.group;

  self.xbClient.connect(function(err){
    if (err) {
      console.log(err);
    }
    else {
      console.log('xblab relay client connected');
    }
  });

  self.wsClient.on('message', function (message) {
    if (message.type === 'utf8') {
      if (cfg.debug){
          console.log(
            "Received utf-8 message of %s characters. Message: '%s'.",
              message.utf8Data.length, message.utf8Data);
      }
      var userMessage = JSON.parse(message.utf8Data);
      if (userMessage && userMessage.type){
        switch(userMessage.type){
          case 'CRED':
            self.xbClient.sendCredential(
              userMessage,
              function (err){
                console.log(err);
              }
            );
            break; 
        }
      }  
    }
    // We're using JSON between local server
    // and page, so this should never happen.
    else if (message.type === 'binary') {
      if (cfg.debug){
        console.error("Received binary message of %s bytes.",
          message.binaryData.length);
      }
    }
  });

  self.wsClient.on('close', function (reasonCode, description) {
    if (cfg.debug){
      console.log('%s xblab presentation client at\n%s disconnected...\n\
        Why am I still running?',
        new Date(),
        self.wsClient.remoteAddress);
    }
  });    
  
  self.wsClient.on('error', function (error) {
    console.error(
      'Connection error for xblab presentation client at %s.\n Details: %s',
      self.wsClient.remoteAddress, error
    );
  });


  // xbClient events
  self.xbClient.on(xblab.events.needCred, function (buf) {
    console.log(buf);

    self.wsClient.send(JSON.stringify(
      { state: constants.needCred, payload: 'connected' }
    ));
  });

  self.xbClient.on(xblab.events.groupEntry, function (buf) {
    console.log(buf);

    // 'status' and 'state' are kind of silly properties
    self.wsClient.send(JSON.stringify(
      { state: constants.groupEntry, payload: buf }
    ));
  });

  self.xbClient.on(xblab.events.error, function(err) {
    console.error(err);
  });

  self.xbClient.on(xblab.events.end, function (buf) {
    console.log(buf);
  });
};
