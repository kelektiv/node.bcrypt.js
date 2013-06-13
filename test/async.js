var bcrypt = require('../bcrypt');

module.exports = {
    test_salt_length: function(assert) {
        assert.expect(1);
        bcrypt.genSalt(10, function(err, salt) {
            assert.equals(29, salt.length, "Salt isn't the correct length.");
            assert.done();
        });
    },
    test_salt_only_cb: function(assert) {
        assert.doesNotThrow(function() {bcrypt.genSalt(function(err, salt) {});}, "Should not throw an Error. Rounds and seed length are optional.");
        assert.done();
    },
    test_salt_rounds_is_string_number: function(assert) {
        bcrypt.genSalt('10', void 0, function (err, salt) {
            assert.ok((err instanceof Error), "Should be an Error. genSalt requires round to be of type number.");
            assert.done();
        });
    },
    test_salt_rounds_is_string_non_number: function(assert) {
        bcrypt.genSalt('b', function (err, salt) {
            assert.ok((err instanceof Error), "Should throw an Error. genSalt requires rounds to of type number.");
            assert.done();
        });
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
        bcrypt.hash(function (err, hash) {
            assert.ok(err, "Should be an error. No params.");
            assert.done();
        });
    },
    test_hash_one_param: function(assert) {
        bcrypt.hash('password', function (err, hash) {
            assert.ok(err, "Should be an Error. No salt.");
            assert.done();
        });
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
    },
    test_hash_compare_invalid_strings: function(assert) {
      var fullString = 'envy1362987212538';
      var hash = '$2a$10$XOPbrlUPQdwdJUpSrIF6X.LbE14qsMmKGhM1A8W9iqaG3vv1BD7WC';
      var wut = ':';
      bcrypt.compare(fullString, hash, function(err, res) {
        assert.ok(res);
        bcrypt.compare(fullString, wut, function(err, res) {
          assert.ok(!res);
          assert.done();
        });
      });
    }
};
