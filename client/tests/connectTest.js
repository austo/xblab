var cfg = require('../xblab.config');
var xb = require('../lib/xblab.wrapper');
cfg.group = 'xchat';

var xbc = new xb.XbClient(cfg);

xbc.connect(function(err){
    if (err) {
    	console.log(err);
    }
});

xbc.on('needCred', function (buf){
	console.log(buf);
	xbc.sendCredential({
		username: 'moraustin@gmail.com',
		password: 'aardvark'
	},
	function(err){
		if (err) {
			console.log(err);
		}
	});
});