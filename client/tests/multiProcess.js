var fork = require('child_process').fork,
  users = require('./users'),
  group = process.argv[2];

for (u in users) {
  (function(user) {
    console.log('username: %s, password: %s', users[u].username,
      users[u].password);

    var proc = fork('./tests/worker.js',
      [users[u].username, users[u].password, group]);    

  })(u); 
}