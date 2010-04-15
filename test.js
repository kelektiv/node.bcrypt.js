var sys = require('sys'), bcrypt = require('./bcrypt_node');

var bc = new bcrypt.BCrypt();
var salt = bc.gen_salt(20);
sys.puts(salt);
var hash1 = bc.hashpw("test", salt);
sys.puts('1: '+hash1);
sys.puts('2: '+bc.compare("test", hash1));
sys.puts('3: '+bc.compare("blah", hash1));