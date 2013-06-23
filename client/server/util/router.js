var constants = require('../../public/js/constants'),
    xblab = require('../../tests/xblab.client'),
    cfg = require('../../xblab.config');


exports.handleWsRequest = function(request) {
    
    // We only want connections from the xblab client    
    if (request.origin.replace(/^https?:\/\//, '') === request.webSocketRequest.host){

        var connection = request.accept(request.origin);

        console.log('%s %s connection accepted from %s at %s - protocol version %s.',
            new Date(), request.protocol, request.origin, connection.remoteAddress,
            connection.webSocketVersion);
        
        // TODO: Make host and port configurable
        var xClient = new xblab.Client(cfg, connection);
    }
    else {
        request.reject(403, "Bad origin");
    }
};