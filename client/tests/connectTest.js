var cfg = require('../xblab.config');
var xb = require('../lib/xblab.wrapper');
cfg.group = 'xchat';

var xbc = new xb.XbClient(cfg);

xbc.connect(function(err){
    console.log(err);
});