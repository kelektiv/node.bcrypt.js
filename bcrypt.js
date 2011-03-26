var bcrypt_lib = require('./bcrypt_lib'),
    emitter = require('events').Emitter,
    exports = module.exports;

//function BCrypt() {
    //emitter.call(this);
    //this.lib = new bcrypt_lib.BCrypt();

//}
//sys.inherits(BCrypt, emitter);

//module.exports = BCrypt;
module.exports = new bcrypt_lib.BCrypt();
