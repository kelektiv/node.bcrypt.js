var bcrypt = require('../bcrypt_node');

exports.tests = {
  test_salt_length: function() {
    var bc = new bcrypt.BCrypt();
    var salt = bc.gen_salt(10);
    assertEquals(29, salt.length, "Salt isn't the correct length.");
  },
  test_hash_compare: function() {
    var bc = new bcrypt.BCrypt();
    var salt = bc.gen_salt(10);
    assertEquals(29, salt.length, "Salt isn't the correct length.");
    var hash = bc.hashpw("test", salt);
    assertTrue(bc.compare("test", hash), "These hashes should be equal.");
    assertFalse(bc.compare("blah", hash), "These hashes should not be equal.");
  }
};
