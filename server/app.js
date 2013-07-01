var net = require('net'),
    util = require('util'),
    fs = require('fs'),
    cfg = require('./xblab.config'),
    xblab = require('./lib/xblab.wrapper'),
    path = require('path'),
    port = cfg.serverPort || 8888;    

xblab.config(cfg, function (err){
    if (err){
        console.log(err);
        process.exit(1);
    }
});

var mgrs = [];

net.createServer(function (socket) { 
    /*
        Binary message - we don't know how
        to parse it here, send to C++.
    */
    xblab.getConnectionBuffer(function (err, buf){
        if (err){
            console.log(err);        
        }
        else {
            if (buf && buf.nonce && buf.buffer){
                socket.lastNonce = buf.nonce;
                console.log('%s - sending NEEDCRED buffer to %s',
                    new Date(), socket.remoteAddress);
                socket.write(buf.buffer);
            }               
        }
    });

    /*
        TODO:
        Need some way to either give socket to correct
        manager or move the whole thing to C++.
        Otherwise we have to determine correct group
        for each 'on data' event.
    */
    socket.on('data', function (data) {
        console.log(data);

        xblab.digestBuffer({
            nonce: socket.lastNonce,
            buffer: data
        },
        function (err, buf){
            if (err){
                console.log(err);
                socket.end()
            }
            else {
                if (buf && buf.nonce && buf.mgr){
                    socket.lastNonce = buf.nonce;
                }
            }
        });
    });

    socket.on('error', function(err){
        console.log(err);
    })
 
    socket.on('end', function () {

    });   
 
}).listen(port);
 
console.log("xblab server running at port %d.\n", port);
