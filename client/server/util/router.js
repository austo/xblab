var constants = require('../../public/js/constants'),
  xblab = require('../../lib/xbClient'),
  cfg = require('../../xblab.config');

var httpRe = /^https?:\/\//,
  groupRe = /\/(\w+)/;

exports.handleWsRequest = function(request) {
  
  // We only want connections from the xblab client    
  if (request.origin.replace(httpRe, '') === request.webSocketRequest.host){

    if (groupRe.test(request.resource)){
      var connection = request.accept(request.origin);
      connection.group = groupRe.exec(request.resource)[1];

      console.log('%s %s connection accepted from %s at %s - protocol version %s.',
        new Date(), request.protocol, request.origin, connection.remoteAddress,
        connection.webSocketVersion);
      
      var xClient = new xblab.Client(cfg, connection);
    }
  }    
  else {
    request.reject(403, "Bad origin");
  }
  //console.log(cfg);
};