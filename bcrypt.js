var bindings = require('bindings')('bcrypt_lib');

/// generate a salt (sync)
/// @param {Number} [rounds] number of rounds (default 10)
/// @param {Number} [seed_length] number of random bytes (default 20)
/// @return {String} salt
module.exports.gen_salt_sync = function(rounds, seed_length) {
  console.log("DEPRECATION WARNING: `gen_salt_sync` has been deprecated. Please use `genSaltSync` instead.");

  return module.exports.genSaltSync(rounds, seed_length);
}

module.exports.genSaltSync = function(rounds, seed_length) {
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
  console.log("DEPRECATION WARNING: `gen_salt` has been deprecated. Please use `genSalt` instead.");

  return module.exports.genSalt(rounds, seed_length, cb);
}

module.exports.genSalt = function(rounds, seed_length, cb) {
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
        return cb(new Error('rounds must be a number'));
    }

    // default length 20
    if (!seed_length) {
        seed_length = 20;
    } else if (typeof seed_length !== 'number') {
        return cb(new Error('seed_length must be a number'));
    }

    if (!cb) {
        return;
    }

    return bindings.gen_salt(rounds, seed_length, cb);
};

/// hash data using a salt
/// @param {String} data the data to encrypt
/// @param {String} salt the salt to use when hashing
/// @return {String} hash
module.exports.encrypt_sync = function(data, salt) {
  console.log("DEPRECATION WARNING: `encrypt_sync` has been deprecated. Please use `hashSync` instead.");

  return module.exports.hashSync(data, salt);
}

module.exports.hashSync = function(data, salt) {
    if (data == null || data == undefined || salt == null || salt == undefined) {
        throw new Error('data and salt arguments required');
    } else if (typeof data !== 'string' && (typeof salt !== 'string' || typeof salt !== 'number')) {
        throw new Error('data must be a string and salt must either be a salt string or a number of rounds');
    }

    if (typeof salt === 'number') {
      salt = module.exports.genSaltSync(salt);
    }

    return bindings.encrypt_sync(data, salt);
};

/// hash data using a salt
/// @param {String} data the data to encrypt
/// @param {String} salt the salt to use when hashing
/// @param {Function} cb callback(err, hash)
module.exports.encrypt = function(data, salt, cb) {
  console.log("DEPRECATION WARNING: `encrypt` has been deprecated. Please use `hash` instead.");

  return module.exports.hash(data, salt, cb);
}

module.exports.hash = function(data, salt, cb) {
    if (typeof data === 'function') {
        return data(new Error('data must be a string and salt must either be a salt string or a number of rounds'));
    }
    if (typeof salt === 'function') {
        return salt(new Error('data must be a string and salt must either be a salt string or a number of rounds'));
    }
    if (data == null || data == undefined || salt == null || salt == undefined) {
        return cb(new Error('data and salt arguments required'));
    } else if (typeof data !== 'string' && (typeof salt !== 'string' || typeof salt !== 'number')) {
        return cb(new Error('data must be a string and salt must either be a salt string or a number of rounds'));
    }

    if (!cb || typeof cb !== 'function') {
        return;
    }

    if (typeof salt === 'number') {
      return module.exports.genSalt(salt, function(err, salt) {
        return bindings.encrypt(data, salt, cb);
      });
    }

    return bindings.encrypt(data, salt, cb);
};

/// compare raw data to hash
/// @param {String} data the data to hash and compare
/// @param {String} hash expected hash
/// @return {bool} true if hashed data matches hash
module.exports.compare_sync = function(data, hash) {
  console.log("DEPRECATION WARNING: `compare_sync` has been deprecated. Please use `compareSync` instead.");

  return module.exports.compareSync(data, hash);
}

module.exports.compareSync = function(data, hash) {
    if (data == null || data == undefined || hash == null || hash == undefined) {
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
    if (data == null || data == undefined || hash == null || hash == undefined) {
        return cb(new Error('data and hash arguments required'));
    } else if (typeof data !== 'string' || typeof hash !== 'string') {
        return cb(new Error('data and hash must be strings'));
    }

    if (!cb || typeof cb !== 'function') {
        return;
    }

    return bindings.compare(data, hash, cb);
};

/// @param {String} hash extract rounds from this hash
/// @return {Number} the number of rounds used to encrypt a given hash
module.exports.get_rounds = function(hash) {
  console.log("DEPRECATION WARNING: `get_rounds` has been deprecated. Please use `getRounds` instead.");

  return module.exports.getRounds(hash);
}

module.exports.getRounds = function(hash) {
    if (hash == null || hash == undefined) {
        throw new Error('hash argument required');
    } else if (typeof hash !== 'string') {
        throw new Error('hash must be a string');
    }

    return bindings.get_rounds(hash);
};

