var sys = require('sys'), bcrypt = require('./bCrypt');

var salt = (new bcrypt.bCrypt).gensalt(10);
sys.puts(crypt.hashpw("test", salt));