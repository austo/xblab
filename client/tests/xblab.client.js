// Adapted from code written by Ben Noordhuis <info@bnoordhuis.nl>

// javascript shim that lets our object inherit from EventEmitter
var util = require('util'),
    xblab = require('../build/Debug/xblab'),
    path = require('path'),
    events = require('events');   

console.log(util.inspect(xblab, { colors: true, depth: null }));

var p = xblab.Participant;

inherits(p, events.EventEmitter);

console.log(util.inspect(p.prototype, { colors: true, depth: null }));

exports.Participant = p;
exports.config = xblab.config;

// extend prototype
function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}