var testCase = require('nodeunit').testCase,
    bcrypt = require('../bcrypt');

module.exports = testCase({
  test_salt_length: function(assert) {
    var EXPECTED = 500;
    assert.expect(EXPECTED);
    var n = 0;
    for (var i = 0; i < EXPECTED; i++) {
      bcrypt.gen_salt(4, function(err, salt) {
        console.log('salt: ' + salt);
        assert.equals(29, salt.length, "Salt ("+salt+") isn't the correct length.");
        //bcrypt.encrypt('test', salt, function(err, crypted) {
        //console.log('crypted: ' + crypted);
        //console.log('crypted cb end: ' + (Date.now() - start) + 'ms');
        //});
        n++;
        console.log(n);
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
