var testCase = require('nodeunit').testCase,
    bcrypt = require('../bcrypt');

module.exports = testCase({
    test_salt_length: function(assert) {
        assert.expect(1);
        bcrypt.gen_salt(10, function(err, salt) {
            assert.equals(29, salt.length, "Salt isn't the correct length.");
            assert.done();
        });
    },
    test_salt_no_params: function(assert) {
        assert.throws(function() {bcrypt.gen_salt();}, "Should throw an Error. gen_salt requires a callback.");
        assert.done();
    },
    test_salt_only_cb: function(assert) {
        assert.doesNotThrow(function() {bcrypt.gen_salt(function(err, salt) {});}, "Should not throw an Error. Rounds and seed length are optional.");
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
    test_encrypt: function(assert) {
        assert.expect(1);
        bcrypt.gen_salt(10, function(err, salt) {
            bcrypt.encrypt('password', salt, function(err, res) {
                assert.ok(res, "Res should be defined.");
                assert.done();
            });
        });
    },
    test_encrypt_no_params: function(assert) {
        assert.throws(function() {bcrypt.encrypt();}, "Should throw an Error. No Params.");
        assert.done();
    },
    test_encrypt_one_param: function(assert) {
        assert.throws(function() {bcrypt.encrypt('password');}, "Should throw an Error. No salt.");
        assert.done();
    },
    test_encrypt_not_hash_str: function(assert) {
        assert.throws(function() {bcrypt.encrypt('password', 1);}, "Should throw an Error. hash should be a string.");
        assert.done();
    },
    test_encrypt_salt_validity: function(assert) {
        assert.expect(3);
        bcrypt.encrypt('password', '$2a$10$somesaltyvaluertsetrse', function(err, enc) {
            assert.equal(err, undefined);
            bcrypt.encrypt('password', 'some$value', function(err, enc) {
                assert.notEqual(err, undefined);
                assert.equal(err.message, "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue");
                assert.done();
            });
        });
    },
    test_verify_salt: function(assert) {
        assert.expect(2);
        bcrypt.gen_salt(10, function(err, salt) {
            var split_salt = salt.split('$');
            assert.ok(split_salt[1], '2a');
            assert.ok(split_salt[2], '10');
            assert.done();
        });
    },
    test_verify_salt_min_rounds: function(assert) {
        assert.expect(2);
        bcrypt.gen_salt(1, function(err, salt) {
            var split_salt = salt.split('$');
            assert.ok(split_salt[1], '2a');
            assert.ok(split_salt[2], '4');
            assert.done();
        });
    },
    test_verify_salt_max_rounds: function(assert) {
        assert.expect(2);
        bcrypt.gen_salt(100, function(err, salt) {
            var split_salt = salt.split('$');
            assert.ok(split_salt[1], '2a');
            assert.ok(split_salt[2], '31');
            assert.done();
        });
    },
    test_hash_compare: function(assert) {
        assert.expect(3);
        bcrypt.gen_salt(10, function(err, salt) {
            assert.equals(29, salt.length, "Salt isn't the correct length.");
            bcrypt.encrypt("test", salt, function(err, hash) {
                bcrypt.compare("test", hash, function(err, res) {
                    assert.equal(res, true, "These hashes should be equal.");
                    bcrypt.compare("blah", hash, function(err, res) {
                        assert.equal(res, false, "These hashes should not be equal.");
                        assert.done();
                    });
                });
            });
        });
    }
});
