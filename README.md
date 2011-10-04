node.bcrypt.js
=============

Lib to help you hash passwords.
[bcrypt on wikipedia][bcryptwiki]

Catalyst for this module: [How To Safely Store A Password][codahale]


Security Issues/Concerns
=============

As should be the case with any security tool, this library should be scrutinized by anyone using it. If you find or suspect an issue with the code- please bring it to my attention and I'll spend some time trying to make sure that this tool is as secure as possible.

To make it easier for people using this tool to analyze what has been surveyed, here is a list of BCrypt related security issues/concerns as they've come up.

* An [issue with passwords][jtr] was found with a version of the Blowfish algorithm developed for John the Ripper. This is not present in the OpenBSD version and is thus not a problem for this module. HT [zooko][zooko].

Dependencies
=============

* NodeJS
* OpenSSL

From NPM
============

npm install bcrypt


From Source
============

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
    // Store hash in your password DB.

To check a password:  

    // Load hash from your password DB.
    bcrypt.compare_sync("B4c0/\/", hash); // true    
    bcrypt.compare_sync("not_bacon", hash); // false

Usage - Async
============

To hash a password:  

    var bcrypt = require('bcrypt');  
    bcrypt.gen_salt(10, function(err, salt) {
        bcrypt.encrypt("B4c0/\/", salt, function(err, hash) {
            // Store hash in your password DB.
        });
    });

To check a password:  

    // Load hash from your password DB.
    bcrypt.compare("B4c0/\/", hash, function(err, res) {
        // res == true    
    });
    bcrypt.compare("not_bacon", hash, function(err, res) {
        // res = false
    });

API
============

`BCrypt.`

  * `gen_salt_sync(rounds, seed_length)`
    * `rounds` - [OPTIONAL] - the number of rounds to process the data for. (default - 10)
    * `seed_length` - [OPTIONAL] - RAND_bytes wants a length. to make that a bit flexible, you can specify a seed_length. (default - 20)
  * `gen_salt(rounds, seed_length, cb)`
    * `rounds` - [OPTIONAL] - the number of rounds to process the data for. (default - 10)
    * `seed_length` - [OPTIONAL] - RAND_bytes wants a length. to make that a bit flexible, you can specify a seed_length. (default - 20)
    * `cb` - [REQUIRED] - a callback to be fired once the salt has been generated. uses eio making it asynchronous.
      * `err` - First parameter to the callback detailing any errors.
      * `salt` - Second parameter to the callback providing the generated salt.
  * `encrypt_sync(data, salt)`
    * `data` - [REQUIRED] - the data to be encrypted.
    * `salt` - [REQUIRED] - the salt to be used in encryption.
  * `encrypt(data, salt, cb)`
    * `data` - [REQUIRED] - the data to be encrypted.
    * `salt` - [REQUIRED] - the salt to be used in encryption.
    * `cb` - [REQUIRED] - a callback to be fired once the data has been encrypted. uses eio making it asynchronous.
      * `err` - First parameter to the callback detailing any errors.
      * `encrypted` - Second parameter to the callback providing the encrypted form.
  * `compare_sync(data, encrypted)`
    * `data` - [REQUIRED] - data to compare.
    * `encrypted` - [REQUIRED] - data to be compared to.
  * `compare(data, encrypted, cb)`
    * `data` - [REQUIRED] - data to compare.
    * `encrypted` - [REQUIRED] - data to be compared to.
    * `cb` - [REQUIRED] - a callback to be fired once the data has been compared. uses eio making it asynchronous.
      * `err` - First parameter to the callback detailing any errors.
      * `same` - Second parameter to the callback providing whether the data and encrypted forms match [true | false].
  * `get_rounds(encrypted)` - return the number of rounds used to encrypt a given hash
    * `encrypted` - [REQUIRED] - hash from which the number of rounds used should be extracted.


Hash Info
============

The characters that comprise the resultant hash are `./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789$`.

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

* [Antonio Salazar Cardozo][shadowfiend] - Early MacOS X support (when we used libbsd)
* [Ben Glow][pixelglow] - Fixes for thread safety with async calls
* [Van Nguyen][thegoleffect] - Found a timing attack in the comparator
* [NewITFarmer][newitfarmer] - Initial Cygwin support
* [David Trejo][dtrejo] - packaging fixes
* [Alfred Westerveld][alfredwesterveld] - packaging fixes
* [Vincent Côté-Roy][vincentr] - Testing around concurrency issues
* [Lloyd Hilaiel][lloyd] - Documentation fixes
* [Roman Shtylman][shtylman] - Code refactoring and general rot reduction
* [Vadim Graboys][vadimg] - Code changes to support 0.5.5+

License
============

Unless stated elsewhere, file headers or otherwise, the license as stated in the LICENSE file.



[bcryptwiki]: http://en.wikipedia.org/wiki/Crypt_(Unix)#Blowfish-based_scheme  
[bcryptgs]: http://mail-index.netbsd.org/tech-crypto/2002/05/24/msg000204.html
[codahale]: http://codahale.com/how-to-safely-store-a-password/
[gh13]: https://github.com/ncb000gt/node.bcrypt.js/issues/13
[jtr]: http://www.openwall.com/lists/oss-security/2011/06/20/2

[shadowfiend]:https://github.com/Shadowfiend
[thegoleffect]:https://github.com/thegoleffect
[pixelglow]:https://github.com/pixelglow
[dtrejo]:https://github.com/dtrejo
[alfredwesterveld]:https://github.com/alfredwesterveld
[newitfarmer]:https://github.com/newitfarmer
[zooko]:https://twitter.com/zooko
[vincentr]:https://twitter.com/vincentcr
[lloyd]:https://github.com/lloyd
[shtylman]:https://github.com/shtylman
[vadimg]:https://github.com/vadimg
