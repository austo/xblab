var net = require('net'),
    util = require('util'),
    xblab = require('../build/Debug/xblab'),
    xbcfg = require('../xblab.config'),
    path = require('path');

    //TODO: better way to get applicion root
    //console.log(process.cwd());
    xbcfg.pubKeyFile = path.join(process.cwd(), xbcfg.pubKeyFile);

    xblab.config(xbcfg); //should be async


var client = net.connect({port: 5000},
    function() { //'connect' listener
        console.log('client connected');
        // client.write('world!\r\n');
});
client.on('data', function(data) {
    //console.log(data.toString());
    console.log(typeof data);
    console.log(util.inspect(data, {colors: true}));
    console.log(data.length);

    // xblab.parseConnectionBuffer(data, function(err, buf){
    //     if (err){
    //         console.log(err);        
    //     }
    //     else{
    //         if (buf){
    //             console.log(buf);
    //             //socket.write(buf);
    //         }
    //     }
    // });

    //client.end();
});
client.on('end', function() {
    console.log('client disconnected');
});