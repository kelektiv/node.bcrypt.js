'use strict';

var binary = require('node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname, './package.json')));
var bindings = require(binding_path);

var crypto = require('crypto');

/// generate a salt (sync)
/// @param {Number} [rounds] number of rounds (default 10)
/// @param {Buffer} [saltbase] 16-byte Buffer with unencoded salt as base (default crypto.randomBytes(16))
/// @return {String} salt
module.exports.genSaltSync = function(rounds, saltbase) {
    // default 10 rounds
    if (!rounds) {
        rounds = 10;
    } else if (typeof rounds !== 'number') {
        throw new Error('rounds must be a number');
    }
    // seed default is random data
    if (!saltbase) {
        saltbase = crypto.randomBytes(16);
    } else if (!Buffer.isBuffer(saltbase) || saltbase.length !== 16) {
        throw new Error('saltbase must be a 16-byte Buffer');
    }
    return bindings.gen_salt_sync(rounds, saltbase);
};

/// generate a salt
/// @param {Number} [rounds] number of rounds (default 10)
/// @param {Buffer} [saltbase] 16-byte Buffer with unencoded salt as base (default crypto.randomBytes(16))
/// @param {Function} cb callback(err, salt)
module.exports.genSalt = function(rounds, saltbase, cb) {
    // if callback is first argument, then use defaults for others
    if (typeof arguments[0] === 'function') {
        // have to set callback first otherwise arguments are overriden
        cb = arguments[0];
        rounds = undefined;
    // callback is second argument
    } else if (typeof arguments[1] === 'function') {
        // have to set callback first otherwise arguments are overriden
        cb = arguments[1];
        saltbase = undefined;
    }

    if (!cb) {
        throw new Error('no callback given');
    }

    // default 10 rounds
    if (!rounds) {
        rounds = 10;
    } else if (typeof rounds !== 'number') {
        // callback error asynchronously
        return process.nextTick(function() {
            cb(new Error('rounds must be a number'));
        });
    }

    if (!saltbase) {
        crypto.randomBytes(16, function(error, randomBytes) {
            if (error) {
                cb(error);
                return;
            }
            bindings.gen_salt(rounds, randomBytes, cb);
        });
    } else if (!Buffer.isBuffer(saltbase) || saltbase.length !== 16) {
        // callback error asynchronously
        return process.nextTick(function() {
            cb(new Error('saltbase must be a 16-byte Buffer'));
        });
    } else {
        bindings.gen_salt(rounds, saltbase, cb);
    }
};

/// hash data using a salt
/// @param {String} data the data to encrypt
/// @param {String} salt the salt to use when hashing
/// @return {String} hash
module.exports.hashSync = function(data, salt) {
    if (data == null || salt == null) {
        throw new Error('data and salt arguments required');
    }

    if (typeof data !== 'string' || (typeof salt !== 'string' && typeof salt !== 'number')) {
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
module.exports.hash = function(data, salt, cb) {
    if (typeof data === 'function') {
        return process.nextTick(function() {
            data(new Error('data must be a string and salt must either be a salt string or a number of rounds'));
        });
    }

    if (typeof salt === 'function') {
        return process.nextTick(function() {
            salt(new Error('data must be a string and salt must either be a salt string or a number of rounds'));
        });
    }

    if (data == null || salt == null) {
        return process.nextTick(function() {
            cb(new Error('data and salt arguments required'));
        });
    }

    if (typeof data !== 'string' || (typeof salt !== 'string' && typeof salt !== 'number')) {
        return process.nextTick(function() {
            cb(new Error('data must be a string and salt must either be a salt string or a number of rounds'));
        });
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
module.exports.compareSync = function(data, hash) {
    if (data == null || hash == null) {
        throw new Error('data and hash arguments required');
    }

    if (typeof data !== 'string' || typeof hash !== 'string') {
        throw new Error('data and hash must be strings');
    }

    return bindings.compare_sync(data, hash);
};

/// compare raw data to hash
/// @param {String} data the data to hash and compare
/// @param {String} hash expected hash
/// @param {Function} cb callback(err, matched) - matched is true if hashed data matches hash
module.exports.compare = function(data, hash, cb) {
    if (data == null || hash == null) {
        return process.nextTick(function() {
            cb(new Error('data and hash arguments required'));
        });
    }

    if (typeof data !== 'string' || typeof hash !== 'string') {
        return process.nextTick(function() {
            cb(new Error('data and hash must be strings'));
        });
    }

    if (!cb || typeof cb !== 'function') {
        return;
    }

    return bindings.compare(data, hash, cb);
};

/// @param {String} hash extract rounds from this hash
/// @return {Number} the number of rounds used to encrypt a given hash
module.exports.getRounds = function(hash) {
    if (hash == null) {
        throw new Error('hash argument required');
    }

    if (typeof hash !== 'string') {
        throw new Error('hash must be a string');
    }

    return bindings.get_rounds(hash);
};
