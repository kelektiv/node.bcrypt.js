var bcrypt = require('../bcrypt');

module.exports = {
    test_encrypt: function(assert) {
        assert.expect(1);
        bcrypt.gen_salt(10, function(err, salt) {
            bcrypt.encrypt('password', salt, function(err, res) {
                assert.ok(res, "Res should be defined.");
                assert.done();
            });
        });
    },
    test_gen_salt_sync: function(assert) {
        var salt = bcrypt.gen_salt_sync(10);
        assert.equals(29, salt.length, "Salt isn't the correct length.");
        var split_salt = salt.split('$');
        assert.ok(split_salt[1], '2a');
        assert.ok(split_salt[2], '10');
        assert.done();
    },
    test_encrypt_compare_sync: function(assert) {
        var salt = bcrypt.gen_salt_sync(10);
        assert.equals(29, salt.length, "Salt isn't the correct length.");
        var hash = bcrypt.encrypt_sync("test", salt);
        assert.ok(bcrypt.compare_sync("test", hash), "These hashes should be equal.");
        assert.ok(!(bcrypt.compare_sync("blah", hash)), "These hashes should not be equal.");
        assert.done();
    },
    test_get_rounds_sync: function(assert) {
        var hash = bcrypt.encrypt_sync("test", bcrypt.gen_salt_sync(9));
        assert.equals(9, bcrypt.get_rounds(hash), "get_rounds can't extract rounds");
        assert.done();
    }
}
