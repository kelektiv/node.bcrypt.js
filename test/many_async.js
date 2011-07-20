var testCase = require('nodeunit').testCase,
    bcrypt = require('../bcrypt');

var EXPECTED = 2500; //number of times to iterate these tests...

module.exports = testCase({
  test_salt_length: function(assert) {
    assert.expect(EXPECTED);
    var n = 0;
    for (var i = 0; i < EXPECTED; i++) {
      var start = new Date().getTime();
      (function(start) {
        bcrypt.gen_salt(10, function(err, salt) {
          assert.equals(29, salt.length, "Salt ("+salt+") isn't the correct length. It is: " + salt.length);
          n++;
        });
      })(start);
    }

    function checkVal() {
        if (n == EXPECTED) {
          assert.done();
        } else {
          setTimeout(checkVal, 100);
        }
    }
    setTimeout(checkVal, 100);
  },
  test_encrypt_length: function(assert) {
    assert.expect(EXPECTED);
    var SALT = '$2a$04$TnjywYklQbbZjdjBgBoA4e';
    var n = 0;
    for (var i = 0; i < EXPECTED; i++) {
      bcrypt.encrypt('test', SALT, function(err, crypted) {
        assert.equals(60, crypted.length, "Encrypted ("+crypted+") isn't the correct length. It is: " + crypted.length);
        n++;
      });
    }

    function checkVal() {
      if (n == EXPECTED) {
        assert.done();
      } else {
        setTimeout(checkVal, 100);
      }
    }
    setTimeout(checkVal, 100);
  },
  test_compare: function(assert) {
    assert.expect(EXPECTED);
    var HASH = '$2a$04$TnjywYklQbbZjdjBgBoA4e9G7RJt9blgMgsCvUvus4Iv4TENB5nHy';
    var n = 0;
    for (var i = 0; i < EXPECTED; i++) {
      bcrypt.compare('test', HASH, function(err, match) {
        assert.equal(true, match, "No match.");
        n++;
      });
    }

    function checkVal() {
      if (n == EXPECTED) {
        assert.done();
      } else {
        setTimeout(checkVal, 100);
      }
    }
    setTimeout(checkVal, 100);
  }
});
