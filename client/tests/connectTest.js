var cfg = require('../xblab.config');
var xblab = require('../lib/xblab.wrapper');
cfg.group = 'xchat';

var client = new xblab.Client(cfg);

client.connect(function(err){
  if (err) {
    console.log(err);
  }
});

client.on('needCred', function (buf){
  console.log(buf);
  client.sendCredential({
  // test values
    username: 'moraustin@gmail.com',
    password: 'aardvark'
  },
  function(err){
    if (err) {
      console.log(err);
    }
  });
});

client.on('groupEntry', function (buf){
  console.log(buf);
});