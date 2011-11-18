var bcrypt = require('../bcrypt');

while (true) {
  bcrypt.gen_salt(10, function(err, salt) {
    console.log('salt: ' + salt);
  });
}
