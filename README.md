bcrypt-node
=============

Lib to help you hash passwords.  
[bcrypt on wikipedia][bcryptwiki]

Catalyst: [How To Safely Store A Password][codahale]


Dependencies
=============

* NodeJS
* OpenSSL

From NPM
============

npm install bcrypt


From Source
============

I run K/Ubuntu. I was able to pull in the bsd libs using:  
    sudo apt-get install libbsd-dev  
_Eventually I'd like to get it so that the libs are all built in to the same package. But, as a work in progress, this gets the job done for now._

What is required is the file `stdlib.h` within `/usr/includes/bsd/` and the compiled bsd libs in `/usr/lib/`. These have to match the conf.check.

Assuming you've already built node, you can run the waf script:  
    node-waf configure
    node-waf build
    npm link

Usage - Sync
============

To hash a password:  
    var bcrypt = require('bcrypt');  
    var salt = bcrypt.gen_salt_sync(10);  
    var hash = bcrypt.encrypt_sync("B4c0/\/", salt);

To check a password:  
    var bcrypt = require('bcrypt');  
    var salt = bcrypt.gen_salt_sync(10);  
    var hash = bcrypt.encrypt_sync("B4c0/\/", salt);
    bcrypt.compare_sync("B4c0/\/", hash); // true    
    bcrypt.compare_sync("not_bacon", hash); // false

Usage - Async
============

To hash a password:  
    var bcrypt = require('bcrypt');  
    bcrypt.gen_salt(10, function(err, salt) {
        bcrypt.encrypt("B4c0/\/", salt, function(err, hash) {
            //something
        });
    });

To check a password:  
    var bcrypt = require('bcrypt');
    bcrypt.gen_salt(10, function(err, salt) {
        bcrypt.encrypt("B4c0/\/", salt, function(err, hash) {
            bcrypt.compare("B4c0/\/", hash, function(err, res) {
                // res == true    
            });
            bcrypt.compare("not_bacon", hash, function(err, res) {
                // res = false
            });
        });
    });

Hash Info
============

The characters that comprise passwords are `./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789$`.

Testing
============

I am using nodeunit. I like the way you write tests with it and I like the default output. As such you'll need it to run the tests. I suspect my tests would run on an older version, but these were written and worked against 0.5.1
   npm install nodeunit@0.5.1
   nodeunit test/

Credits
============

The code for this comes from a few sources:

* blowfish.cc - OpenBSD
* bcrypt.cc - OpenBSD
* bcrypt::gen_salt - [gen_salt inclusion to bcrypt][bcryptgs]
* bcrypt_node.cc - me

Contributors
============

[Antonio Salazar Cardozo][shadowfiend]

License
============

Unless stated elsewhere, file headers or otherwise, the license as stated in the LICENSE file.



[bcryptwiki]: http://en.wikipedia.org/wiki/Crypt_(Unix)#Blowfish-based_scheme  
[bcryptgs]: http://mail-index.netbsd.org/tech-crypto/2002/05/24/msg000204.html
[codahale]: http://codahale.com/how-to-safely-store-a-password/

[shadowfiend]:https://github.com/Shadowfiend
