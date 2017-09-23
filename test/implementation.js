var bcrypt = require('../bcrypt');

// some tests were adapted from https://github.com/riverrun/bcrypt_elixir/blob/master/test/base_test.exs
// which are under the BSD LICENSE
module.exports = {
    openwall_bcrypt_tests: function(assert) {
        assert.strictEqual(bcrypt.hashSync("U*U", "$2a$05$CCCCCCCCCCCCCCCCCCCCC."), "$2a$05$CCCCCCCCCCCCCCCCCCCCC.E5YPO9kmyuRGyh0XouQYb4YMJKvyOeW");
        assert.strictEqual(bcrypt.hashSync("U*U*", "$2a$05$CCCCCCCCCCCCCCCCCCCCC."), "$2a$05$CCCCCCCCCCCCCCCCCCCCC.VGOzA784oUp/Z0DY336zx7pLYAy0lwK");
        assert.strictEqual(bcrypt.hashSync("U*U*U", "$2a$05$XXXXXXXXXXXXXXXXXXXXXO"), "$2a$05$XXXXXXXXXXXXXXXXXXXXXOAcXxm9kjPGEMsLznoKqmqw7tc8WCx4a");
        assert.strictEqual(bcrypt.hashSync("", "$2a$05$CCCCCCCCCCCCCCCCCCCCC."), "$2a$05$CCCCCCCCCCCCCCCCCCCCC.7uG0VCzI2bS7j6ymqJi9CdcdxiRTWNy");
        assert.strictEqual(bcrypt.hashSync("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", "$2a$05$abcdefghijklmnopqrstuu"), "$2a$05$abcdefghijklmnopqrstuu5s2v8.iXieOjg/.AySBTTZIIVFJeBui");
        assert.done();
    },
    test_long_passwords: function(assert) {
        // bcrypt wrap-around bug in $2a$
        assert.strictEqual(bcrypt.hashSync("012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234", "$2a$05$CCCCCCCCCCCCCCCCCCCCC."), "$2a$05$CCCCCCCCCCCCCCCCCCCCC.6.O1dLNbjod2uo0DVcW.jHucKbPDdHS");
        assert.strictEqual(bcrypt.hashSync("0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345", "$2a$05$CCCCCCCCCCCCCCCCCCCCC."), "$2a$05$CCCCCCCCCCCCCCCCCCCCC.6.O1dLNbjod2uo0DVcW.jHucKbPDdHS");
        assert.done();
    },
    test_shorten_salt_to_128_bits: function(assert) {
        assert.strictEqual(bcrypt.hashSync("test", "$2a$10$1234567899123456789012"), "$2a$10$123456789912345678901u.OtL1A1eGK5wmvBKUDYKvuVKI7h2XBu");
        assert.strictEqual(bcrypt.hashSync("U*U*", "$2a$05$CCCCCCCCCCCCCCCCCCCCCh"), "$2a$05$CCCCCCCCCCCCCCCCCCCCCeUQ7VjYZ2hd4bLYZdhuPpZMUpEUJDw1S");
        assert.strictEqual(bcrypt.hashSync("U*U*", "$2a$05$CCCCCCCCCCCCCCCCCCCCCM"), "$2a$05$CCCCCCCCCCCCCCCCCCCCC.VGOzA784oUp/Z0DY336zx7pLYAy0lwK");
        assert.strictEqual(bcrypt.hashSync("U*U*", "$2a$05$CCCCCCCCCCCCCCCCCCCCCA"), "$2a$05$CCCCCCCCCCCCCCCCCCCCC.VGOzA784oUp/Z0DY336zx7pLYAy0lwK");
        assert.done();
    }
}