var xbcfg = require('../xblab.config'),
    xblab = require('../lib/xblab.wrapper'),
    util = require('util'),
    assert = require('assert');


xblab.config(xbcfg);

// xblab.CreateManager() is a stand in for new Manager()
var mgr = new xblab.Manager('xchat');

assert.equal(typeof mgr.on, 'function',
    'xblab.Manager not inheriting from EventEmitter');

mgr.on('decrypted', function(str){
    console.log('Emitted: %s', str);
});

mgr.sayHello();

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