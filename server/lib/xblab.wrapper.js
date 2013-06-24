// Adapted from code written by Ben Noordhuis <info@bnoordhuis.nl>

// JS shim that lets our object inherit from node.EventEmitter
var xblab = require('../build/Debug/xblab'),
    // util = require('util'),
    // assert = require('assert'),
    events = require('events');   


var m = xblab.Manager;
inherits(m, events.EventEmitter);

for (var f in xblab){
    if (f === 'Manager'){
        exports[f] = m;        
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