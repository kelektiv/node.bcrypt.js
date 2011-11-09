try {
  var bindings = require('./build/default/bcrypt_lib');
} catch(e) {
  //update for v0.5.5+
  var bindings = require('./build/Release/bcrypt_lib');
}

// return the salt
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

// cb (err, sault)
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

module.exports.encrypt_sync = function(data, salt) {
    if (!data || !salt) {
        throw new Error('data and salt arguments required');
    } else if (typeof data !== 'string' || typeof salt !== 'string') {
        throw new Error('data and salt must be strings');
    }

    return bindings.encrypt_sync(data, salt);
};

// cb (err, hash)
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

// return true if matched
module.exports.compare_sync = function(data, hash) {
    if (!data || !hash) {
        throw new Error('data and hash arguments required');
    } else if (typeof data !== 'string' || typeof hash !== 'string') {
        throw new Error('data and hash must be strings');
    }

    return bindings.compare_sync(data, hash);
};

// cb (err, same)
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

module.exports.get_rounds = function(hash) {
    if (!hash) {
        throw new Error('hash argument required');
    } else if (typeof hash !== 'string') {
        throw new Error('hash must be a string');
    }

    return bindings.get_rounds(hash);
};

