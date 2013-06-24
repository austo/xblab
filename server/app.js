var net = require('net'),
    util = require('util'),
    fs = require('fs'),
    cfg = require('./xblab.config'),
    xblab = require('./lib/xblab.wrapper'),
    path = require('path'),
    port = 8888;    

    xblab.config(cfg); //should be async


net.createServer(function (socket) { 
    /*
        This should be a 100% binary message.
        We don't know how to parse it at this level,
        so send it down to C++ land
    */

    xblab.getConnectionBuffer(function(err, buf){
        if (err){
            console.log(err);        
        }
        else{
            if (buf){
                // console.log(buf);
                if (buf.nonce && buf.buffer){
                    socket.lastNonce = buf.nonce;
                    console.log('%s - sending NEEDCRED buffer to %s',
                        new Date(), socket.remoteAddress);
                    socket.write(buf.buffer);
                }
                
                // console.log(socket);
            }
        }
    });

    socket.on('data', function (data) {
        console.log(data);     

        xblab.digestBuffer({
            nonce: socket.lastNonce,
            buffer: data
        },
        function (err){
            console.log(err);
        });
    });
 
    socket.on('end', function () {

    });   
 
}).listen(port);
 
console.log("xblab server running at port %d.\n", port);
