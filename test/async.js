var bcrypt = require('../bcrypt');

module.exports = {
    test_salt_length: function(assert) {
        assert.expect(1);
        bcrypt.genSalt(10, function(err, salt) {
            assert.equals(29, salt.length, "Salt isn't the correct length.");
            assert.done();
        });
    },
    test_salt_no_params: function(assert) {
        assert.throws(function() {bcrypt.genSalt();}, "Should throw an Error. genSalt requires a callback.");
        assert.done();
    },
    test_salt_only_cb: function(assert) {
        assert.doesNotThrow(function() {bcrypt.genSalt(function(err, salt) {});}, "Should not throw an Error. Rounds and seed length are optional.");
        assert.done();
    },
    test_salt_rounds_is_string_number: function(assert) {
        assert.throws(function() {bcrypt.genSalt('10');}, "Should throw an Error. No params.");
        assert.done();
    },
    test_salt_rounds_is_NaN: function(assert) {
        assert.throws(function() {bcrypt.genSalt('b');}, "Should throw an Error. genSalt requires rounds to be a number.");
        assert.done();
    },
    test_hash: function(assert) {
        assert.expect(1);
        bcrypt.genSalt(10, function(err, salt) {
            bcrypt.hash('password', salt, function(err, res) {
                assert.ok(res, "Res should be defined.");
                assert.done();
            });
        });
    },
    test_hash_rounds: function(assert) {
        assert.expect(1);
        bcrypt.hash('bacon', 8, function(err, hash) {
          assert.equals(bcrypt.getRounds(hash), 8, "Number of rounds should be that specified in the function call.");
          assert.done();
        });
    },
    test_hash_empty_strings: function(assert) {
        assert.expect(2);
        bcrypt.genSalt(10, function(err, salt) {
            bcrypt.hash('', salt, function(err, res) {
                assert.ok(res, "Res should be defined even with an empty pw.");
                bcrypt.hash('', '', function(err, res) {
                  if (err) {
                    assert.ok(err);
                  } else {
                    assert.fail();
                  }

                  assert.done();
                });
            });
        });
    },
    test_hash_no_params: function(assert) {
        assert.throws(function() {bcrypt.hash();}, "Should throw an Error. No Params.");
        assert.done();
    },
    test_hash_one_param: function(assert) {
        assert.throws(function() {bcrypt.hash('password');}, "Should throw an Error. No salt.");
        assert.done();
    },
    test_hash_not_hash_str: function(assert) {
        assert.throws(function() {bcrypt.hash('password', 1);}, "Should throw an Error. hash should be a string.");
        assert.done();
    },
    test_hash_salt_validity: function(assert) {
        assert.expect(3);
        bcrypt.hash('password', '$2a$10$somesaltyvaluertsetrse', function(err, enc) {
            assert.equal(err, undefined);
            bcrypt.hash('password', 'some$value', function(err, enc) {
                assert.notEqual(err, undefined);
                assert.equal(err.message, "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue");
                assert.done();
            });
        });
    },
    test_verify_salt: function(assert) {
        assert.expect(2);
        bcrypt.genSalt(10, function(err, salt) {
            var split_salt = salt.split('$');
            assert.ok(split_salt[1], '2a');
            assert.ok(split_salt[2], '10');
            assert.done();
        });
    },
    test_verify_salt_min_rounds: function(assert) {
        assert.expect(2);
        bcrypt.genSalt(1, function(err, salt) {
            var split_salt = salt.split('$');
            assert.ok(split_salt[1], '2a');
            assert.ok(split_salt[2], '4');
            assert.done();
        });
    },
    test_verify_salt_max_rounds: function(assert) {
        assert.expect(2);
        bcrypt.genSalt(100, function(err, salt) {
            var split_salt = salt.split('$');
            assert.ok(split_salt[1], '2a');
            assert.ok(split_salt[2], '31');
            assert.done();
        });
    },
    test_hash_compare: function(assert) {
        assert.expect(3);
        bcrypt.genSalt(10, function(err, salt) {
            assert.equals(29, salt.length, "Salt isn't the correct length.");
            bcrypt.hash("test", salt, function(err, hash) {
                bcrypt.compare("test", hash, function(err, res) {
                    assert.equal(res, true, "These hashes should be equal.");
                    bcrypt.compare("blah", hash, function(err, res) {
                        assert.equal(res, false, "These hashes should not be equal.");
                        assert.done();
                    });
                });
            });
        });
    },
    test_hash_compare_empty_strings: function(assert) {
        assert.expect(2);
        var hash = bcrypt.hashSync("test", bcrypt.genSaltSync(10));

        bcrypt.compare("", hash, function(err, res) {
          assert.equal(res, false, "These hashes should be equal.");
          bcrypt.compare("", "", function(err, res) {
            assert.equal(res, false, "These hashes should be equal.");
            assert.done();
          });
        });
    }
};
