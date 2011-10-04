var bcrypt = require('../bcrypt');

var start = Date.now();
bcrypt.gen_salt(10, function(err, salt) {
  console.log('salt: ' + salt);
  console.log('salt cb end: ' + (Date.now() - start) + 'ms');
  bcrypt.encrypt('test', salt, function(err, crypted) {
    console.log('crypted: ' + crypted);
    console.log('crypted cb end: ' + (Date.now() - start) + 'ms');
    console.log('rounds used from hash:', bcrypt.get_rounds(crypted));
    bcrypt.compare('test', crypted, function(err, res) {
      console.log('compared true: ' + res);
      console.log('compared true cb end: ' + (Date.now() - start) + 'ms');
    });
    bcrypt.compare('bacon', crypted, function(err, res) {
      console.log('compared false: ' + res);
      console.log('compared false cb end: ' + (Date.now() - start) + 'ms');
    });
  });
})
console.log('end: ' + (Date.now() - start) + 'ms');
