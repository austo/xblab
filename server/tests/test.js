var xbcfg = require('../xblab.config'),
    xblab = require('../build/Debug/xblab'),
    util = require('util'),
    path = require('path');

//TODO: better way to get applicion root
console.log(process.cwd());
xbcfg.pubKeyFile = path.join(process.cwd(), xbcfg.pubKeyFile);
xbcfg.privKeyFile = path.join(process.cwd(), xbcfg.privKeyFile);

xblab.config(xbcfg);

var mgr = xblab.createManager('xchat');
//mgr.blah = 42;
console.log(util.inspect(mgr, {colors: true}));
console.log(mgr.sayHello());

xblab.getConnectionBuffer(function(err, buf){
    if (err){
        console.log(err);        
    }
    else{
        if (buf){
            console.log(buf);
        }
    }
});