var fork = require('child_process').fork,
  users = require('./users'),
  group = process.argv[2];


for (u in users) {
  (function(user) {
    console.log('username: %s, password: %s', users[user].username,
      users[user].password);

    var proc = fork('./tests/worker.js',
      [users[user].username, users[user].password, group, process.argv[3]]);   

  })(u); 
}