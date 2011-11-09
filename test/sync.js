var testCase = require('nodeunit').testCase,
    bcrypt = require('../bcrypt');

module.exports = testCase({
    test_salt_length: function(assert) {
        var salt = bcrypt.gen_salt_sync(10);
        assert.equals(29, salt.length, "Salt isn't the correct length.");
        assert.done();
    },
    test_salt_no_params: function(assert) {
        // same as test_verify_salt except using default rounds of 10
        var salt = bcrypt.gen_salt_sync();
        var split_salt = salt.split('$');
        assert.ok(split_salt[1], '2a');
        assert.ok(split_salt[2], '10');
        assert.done();
    },
    test_salt_rounds_is_string_number: function(assert) {
        assert.throws(function() {bcrypt.gen_salt_sync('10');}, "Should throw an Error. No params.");
        assert.done();
    },
    test_salt_rounds_is_NaN: function(assert) {
        assert.throws(function() {bcrypt.gen_salt_sync('b');}, "Should throw an Error. gen_salt requires rounds to be a number.");
        assert.done();
    },
    test_encrypt: function(assert) {
        assert.ok(bcrypt.encrypt_sync('password', bcrypt.gen_salt_sync(10)), "Shouldn't throw an Error.");
        assert.done();
    },
    test_hash_pw_no_params: function(assert) {
        assert.throws(function() {bcrypt.encrypt_sync();}, "Should throw an Error. No Params.");
        assert.done();
    },
    test_hash_pw_one_param: function(assert) {
        assert.throws(function() {bcrypt.encrypt_sync('password');}, "Should throw an Error. No salt.");
        assert.done();
    },
    test_hash_pw_not_hash_str: function(assert) {
        assert.throws(function() {bcrypt.encrypt_sync('password', 1);}, "Should throw an Error. hash should be a string.");
        assert.done();
    },
    test_encrypt_salt_validity: function(assert) {
        assert.expect(2);
        assert.ok(bcrypt.encrypt_sync('password', '$2a$10$somesaltyvaluertsetrse'));
        assert.throws(function() { 
            bcrypt.encrypt('password', 'some$value');
        });
        assert.done();
    },
    test_verify_salt: function(assert) {
        var salt = bcrypt.gen_salt_sync(10);
        var split_salt = salt.split('$');
        assert.ok(split_salt[1], '2a');
        assert.ok(split_salt[2], '10');
        assert.done();
    },
    test_verify_salt_min_rounds: function(assert) {
        var salt = bcrypt.gen_salt_sync(1);
        var split_salt = salt.split('$');
        assert.ok(split_salt[1], '2a');
        assert.ok(split_salt[2], '4');
        assert.done();
    },
    test_verify_salt_max_rounds: function(assert) {
        var salt = bcrypt.gen_salt_sync(100);
        var split_salt = salt.split('$');
        assert.ok(split_salt[1], '2a');
        assert.ok(split_salt[2], '31');
        assert.done();
    },
    test_hash_compare: function(assert) {
        var salt = bcrypt.gen_salt_sync(10);
        assert.equals(29, salt.length, "Salt isn't the correct length.");
        var hash = bcrypt.encrypt_sync("test", salt);
        assert.ok(bcrypt.compare_sync("test", hash), "These hashes should be equal.");
        assert.ok(!(bcrypt.compare_sync("blah", hash)), "These hashes should not be equal.");
        assert.done();
    },
    test_get_rounds: function(assert) {
        var hash = bcrypt.encrypt_sync("test", bcrypt.gen_salt_sync(9));
        assert.equals(9, bcrypt.get_rounds(hash), "get_rounds can't extract rounds");
        assert.done();
    }
});
