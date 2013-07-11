// Adapted from code written by Ben Noordhuis <info@bnoordhuis.nl>

// JS shim that lets our object inherit from node.EventEmitter
var xblab = require('../build/Debug/xblab'),
    // assert = require('assert'),
    // util = require('util').
    events = require('events');   


var p = xblab.XbClient;
inherits(p, events.EventEmitter);

for (var f in xblab){
    if (f === 'XbClient'){
        exports[f] = p;        
    }
    else {
        exports[f] = xblab[f];
    }
}

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}