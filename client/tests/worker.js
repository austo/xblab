var cfg = require('../xblab.config'),
  xblab = require('../lib/xblab.wrapper');

var un = process.argv[2],
  pw = process.argv[3];
cfg.group = process.argv[4];

// console.log("username: %s, password: %s", un, pw);

var client = new xblab.Client(cfg);

client.connect(function(err){
  if (err) {
    console.log('Error: ' + err);
  }
});

client.on('needCred', function (buf) {
  console.log(buf);
  client.sendCredential({
    username: un,
    password: pw
  },
  function(err){
    if (err) {
      console.log(err);
    }
  });
});

client.on('groupEntry', function (buf) {
  console.log('groupEntry: ' + buf);
});

client.on('beginChat', function (buf) {
  console.log('beginChat: ' + buf);
});

client.on('end', function (buf) {
  console.log('end: ' + buf);
});

client.on('error', function(err) {
  console.error('Error: ' + err);
});