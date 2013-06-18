// Load the TCP Library
var net = require('net'),
    util = require('util'),
    fs = require('fs'),
    xbcfg = require('./xblab.config'),
    xblab = require('./build/Debug/xblab'),
    path = require('path');

    //TODO: better way to get applicion root
    console.log(process.cwd());
    xbcfg.pubKeyFile = path.join(process.cwd(), xbcfg.pubKeyFile);
    xbcfg.privKeyFile = path.join(process.cwd(), xbcfg.privKeyFile);

    xblab.config(xbcfg); //should be async


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
                console.log(buf);
                socket.write(buf);
            }
        }
    });

    socket.on('data', function (data) {
        // send to xblab        
    });
 
    socket.on('end', function () {

    });   
 
}).listen(5000);
 
console.log("xblab server running at port 5000\n");
