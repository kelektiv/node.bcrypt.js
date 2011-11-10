try {
  var bindings = require('./build/default/bcrypt_lib');
} catch(e) {
  //update for v0.5.5+
  var bindings = require('./build/Release/bcrypt_lib');
}

/// generate a salt (sync)
/// @param {Number} [rounds] number of rounds (default 10)
/// @param {Number} [seed_length] number of random bytes (default 20)
/// @return {String} salt
module.exports.gen_salt_sync = function(rounds, seed_length) {

    // default 10 rounds
    if (!rounds) {
        rounds = 10;
    } else if (typeof rounds !== 'number') {
        throw new Error('rounds must be a number');
    }

    // default length 20
    if (!seed_length) {
        seed_length = 20;
    } else if (typeof seed_length !== 'number') {
        throw new Error('seed_length must be a number');
    }

    return bindings.gen_salt_sync(rounds, seed_length);
};

/// generate a salt
/// @param {Number} [rounds] number of rounds (default 10)
/// @param {Number} [seed_length] number of random bytes (default 20)
/// @param {Function} cb callback(err, salt)
module.exports.gen_salt = function(rounds, seed_length, cb) {

    // if callback is first argument, then use defaults for others
    if (typeof arguments[0] === 'function') {
        // have to set callback first otherwise arguments are overriden
        cb = arguments[0];
        rounds = 10;
        seed_length = 20;
    // callback is second argument
    } else if (typeof arguments[1] === 'function') {
        // have to set callback first otherwise arguments are overriden
        cb = arguments[1];
        seed_length = 20;
    }

    // default 10 rounds
    if (!rounds) {
        rounds = 10;
    } else if (typeof rounds !== 'number') {
        throw new Error('rounds must be a number');
    }

    // default length 20
    if (!seed_length) {
        seed_length = 20;
    } else if (typeof seed_length !== 'number') {
        throw new Error('seed_length must be a number');
    }

    if (!cb) {
        throw new Error('callback required for gen_salt');
    }

    return bindings.gen_salt(rounds, seed_length, cb);
};

/// hash data using a salt
/// @param {String} data the data to encrypt
/// @param {String} salt the salt to use when hashing
/// @return {String} hash
module.exports.encrypt_sync = function(data, salt) {
    if (!data || !salt) {
        throw new Error('data and salt arguments required');
    } else if (typeof data !== 'string' || typeof salt !== 'string') {
        throw new Error('data and salt must be strings');
    }

    return bindings.encrypt_sync(data, salt);
};

/// hash data using a salt
/// @param {String} data the data to encrypt
/// @param {String} salt the salt to use when hashing
/// @param {Function} cb callback(err, hash)
module.exports.encrypt = function(data, salt, cb) {
    if (!data || !salt) {
        throw new Error('data and salt arguments required');
    } else if (typeof data !== 'string' || typeof salt !== 'string') {
        throw new Error('data and salt must be strings');
    }

    if (!cb) {
        throw new Error('callback required for async compare');
    } else if (typeof cb !== 'function') {
        throw new Error('callback must be a function');
    }

    return bindings.encrypt(data, salt, cb);
};

/// compare raw data to hash
/// @param {String} data the data to hash and compare
/// @param {String} hash expected hash
/// @return {bool} true if hashed data matches hash
module.exports.compare_sync = function(data, hash) {
    if (!data || !hash) {
        throw new Error('data and hash arguments required');
    } else if (typeof data !== 'string' || typeof hash !== 'string') {
        throw new Error('data and hash must be strings');
    }

    return bindings.compare_sync(data, hash);
};

/// compare raw data to hash
/// @param {String} data the data to hash and compare
/// @param {String} hash expected hash
/// @param {Function} cb callback(err, matched) - matched is true if hashed data matches hash
module.exports.compare = function(data, hash, cb) {
    if (!data || !hash) {
        throw new Error('data and hash arguments required');
    } else if (typeof data !== 'string' || typeof hash !== 'string') {
        throw new Error('data and hash must be strings');
    }

    if (!cb) {
        throw new Error('callback required for async compare');
    } else if (typeof cb !== 'function') {
        throw new Error('callback must be a function');
    }

    return bindings.compare(data, hash, cb);
};

/// @param {String} hash extract rounds from this hash
/// @return {Number} the number of rounds used to encrypt a given hash
module.exports.get_rounds = function(hash) {
    if (!hash) {
        throw new Error('hash argument required');
    } else if (typeof hash !== 'string') {
        throw new Error('hash must be a string');
    }

    return bindings.get_rounds(hash);
};

