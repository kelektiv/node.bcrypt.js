var testCase = require('nodeunit').testCase,
    bcrypt = require('../bcrypt');

module.exports = testCase({
    test_salt_length: function(assert) {
        var salt = bcrypt.gen_salt(10);
        assert.equals(29, salt.length, "Salt isn't the correct length.");
        assert.done();
    },
    test_salt_no_params: function(assert) {
        assert.throws(function() {bcrypt.gen_salt();}, "Should throw an Error. gen_salt requires # of rounds.");
        assert.done();
    },
    test_salt_rounds_is_string_number: function(assert) {
        assert.throws(function() {bcrypt.gen_salt('10');}, "Should throw an Error. No params.");
        assert.done();
    },
    test_salt_rounds_is_NaN: function(assert) {
        assert.throws(function() {bcrypt.gen_salt('b');}, "Should throw an Error. gen_salt requires rounds to be a number.");
        assert.done();
    },
    test_hashpw: function(assert) {
        assert.ok(bcrypt.hashpw('password', bcrypt.gen_salt(10)), "Shouldn't throw an Error.");
        assert.done();
    },
    test_hash_pw_no_params: function(assert) {
        assert.throws(function() {bcrypt.hashpw();}, "Should throw an Error. No Params.");
        assert.done();
    },
    test_hash_pw_one_param: function(assert) {
        assert.throws(function() {bcrypt.hashpw('password');}, "Should throw an Error. No salt.");
        assert.done();
    },
    test_hash_pw_not_hash_str: function(assert) {
        assert.throws(function() {bcrypt.hashpw('password', 1);}, "Should throw an Error. hash should be a string.");
        assert.done();
    },
    test_verify_salt: function(assert) {
        var salt = bcrypt.gen_salt(10);
        var split_salt = salt.split('$');
        assert.ok(split_salt[1], '2a');
        assert.ok(split_salt[2], '10');
        assert.done();
    },
    test_verify_salt_min_rounds: function(assert) {
        var salt = bcrypt.gen_salt(1);
        var split_salt = salt.split('$');
        assert.ok(split_salt[1], '2a');
        assert.ok(split_salt[2], '4');
        assert.done();
    },
    test_verify_salt_max_rounds: function(assert) {
        var salt = bcrypt.gen_salt(100);
        var split_salt = salt.split('$');
        assert.ok(split_salt[1], '2a');
        assert.ok(split_salt[2], '31');
        assert.done();
    },
    test_hash_compare: function(assert) {
        var salt = bcrypt.gen_salt(10);
        assert.equals(29, salt.length, "Salt isn't the correct length.");
        var hash = bcrypt.hashpw("test", salt);
        assert.ok(bcrypt.compare("test", hash), "These hashes should be equal.");
        assert.ok(!(bcrypt.compare("blah", hash)), "These hashes should not be equal.");
        assert.done();
    }
});
