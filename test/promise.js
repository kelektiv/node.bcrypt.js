var bcrypt = require('../bcrypt');

var fail = function(assert, error) {
    assert.ok(false, error);
    assert.done();
};

// only run these tests if Promise is available
if (typeof Promise !== 'undefined') {
    module.exports = {
        test_salt_returns_promise_on_no_args: function(assert) {
            // make sure test passes with non-native implementations such as bluebird
            // http://stackoverflow.com/questions/27746304/how-do-i-tell-if-an-object-is-a-promise
            assert.ok(typeof bcrypt.genSalt().then === 'function', "Should return a promise");
            assert.done();
        },
        test_salt_length: function(assert) {
            assert.expect(2);
            bcrypt.genSalt(10).then(function(salt) {
                assert.ok(typeof salt !== 'undefined', 'salt must not be undefined');
                assert.equals(29, salt.length, "Salt isn't the correct length.");
                assert.done();
            });
        },
        test_salt_rounds_is_string_number: function(assert) {
            assert.expect(1);
            bcrypt.genSalt('10').then(function() {
                fail(assert, "should not be resolved");
            }).catch(function(err) {
                assert.ok((err instanceof Error), "Should be an Error. genSalt requires round to be of type number.");
            }).then(function() {
                assert.done();
            });
        },
        test_salt_rounds_is_string_non_number: function(assert) {
            assert.expect(1);
            bcrypt.genSalt('b').then(function() {
                fail(assert, "should not be resolved");
            }).catch(function(err) {
                assert.ok((err instanceof Error), "Should be an Error. genSalt requires round to be of type number.");
            }).then(function() {
                assert.done();
            });
        },
        test_hash: function(assert) {
            assert.expect(1);
            bcrypt.genSalt(10).then(function(salt) {
                return bcrypt.hash('password', salt);
            }).then(function(res) {
                assert.ok(res, "Res should be defined.");
                assert.done();
            });
        },
        test_hash_rounds: function(assert) {
            assert.expect(1);
            bcrypt.hash('bacon', 8).then(function(hash) {
                assert.equals(bcrypt.getRounds(hash), 8, "Number of rounds should be that specified in the function call.");
                assert.done();
            });
        },
        test_hash_empty_strings: function(assert) {
            assert.expect(2);
            Promise.all([
                bcrypt.genSalt(10).then(function(salt) {
                    return bcrypt.hash('', salt);
                }).then(function(res) {
                    assert.ok(res, "Res should be defined even with an empty pw.");
                }),
                bcrypt.hash('', '').then(function() {
                    fail(assert, "should not be resolved")
                }).catch(function(err) {
                    assert.ok(err);
                }),
            ]).then(function() {
                assert.done();
            });
        },
        test_hash_no_params: function(assert) {
            assert.expect(1);
            bcrypt.hash().then(function() {
                fail(assert, "should not be resolved");
            }).catch(function(err) {
                assert.ok(err, "Should be an error. No params.");
            }).then(function() {
                assert.done();
            });
        },
        test_hash_one_param: function(assert) {
            assert.expect(1);
            bcrypt.hash('password').then(function() {
                fail(assert, "should not be resolved");
            }).catch(function(err) {
                assert.ok(err, "Should be an error. No salt.");
            }).then(function() {
                assert.done();
            });
        },
        test_hash_salt_validity: function(assert) {
            assert.expect(3);
            Promise.all(
                [
                    bcrypt.hash('password', '$2a$10$somesaltyvaluertsetrse').then(function(enc) {
                        assert.ok(enc, "should be resolved with a value");
                    }),
                    bcrypt.hash('password', 'some$value').then(function() {
                        fail(assert, "should not resolve");
                    }).catch(function(err) {
                        assert.notEqual(err, undefined);
                        assert.equal(err.message, "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue");
                    })
                ]).then(function() {
                assert.done();
            });
        },
        test_verify_salt: function(assert) {
            assert.expect(2);
            bcrypt.genSalt(10).then(function(salt) {
                var split_salt = salt.split('$');
                assert.ok(split_salt[1], '2a');
                assert.ok(split_salt[2], '10');
                assert.done();
            });
        },
        test_verify_salt_min_rounds: function(assert) {
            assert.expect(2);
            bcrypt.genSalt(1).then(function(salt) {
                var split_salt = salt.split('$');
                assert.ok(split_salt[1], '2a');
                assert.ok(split_salt[2], '4');
                assert.done();
            });
        },
        test_verify_salt_max_rounds: function(assert) {
            assert.expect(2);
            bcrypt.genSalt(100).then(function(salt) {
                var split_salt = salt.split('$');
                assert.ok(split_salt[1], '2a');
                assert.ok(split_salt[2], '31');
                assert.done();
            });
        },
        test_hash_compare: function(assert) {
            assert.expect(3);
            bcrypt.genSalt(10).then(function(salt) {
                assert.equals(29, salt.length, "Salt isn't the correct length.");
                return bcrypt.hash("test", salt);
            }).then(function(hash) {
                return Promise.all(
                    [
                        bcrypt.compare("test", hash).then(function(res) {
                            assert.equal(res, true, "These hashes should be equal.");
                        }),
                        bcrypt.compare("blah", hash).then(function(res) {
                            assert.equal(res, false, "These hashes should not be equal.");
                        })
                    ]).then(function() {
                    assert.done();
                });
            });
        },
        test_hash_compare_empty_strings: function(assert) {
            assert.expect(2);
            var hash = bcrypt.hashSync("test", bcrypt.genSaltSync(10));
            bcrypt.compare("", hash).then(function(res) {
                assert.equal(res, false, "These hashes should be equal.");
                return bcrypt.compare("", "");
            }).then(function(res) {
                assert.equal(res, false, "These hashes should be equal.");
                assert.done();
            });
        },
        test_hash_compare_invalid_strings: function(assert) {
            var fullString = 'envy1362987212538';
            var hash = '$2a$10$XOPbrlUPQdwdJUpSrIF6X.LbE14qsMmKGhM1A8W9iqaG3vv1BD7WC';
            var wut = ':';
            Promise.all([
                bcrypt.compare(fullString, hash).then(function(res) {
                    assert.ok(res);
                }),
                bcrypt.compare(fullString, wut).then(function(res) {
                    assert.ok(!res);
                })
            ]).then(function() {
                assert.done();
            });
        }
    };
}