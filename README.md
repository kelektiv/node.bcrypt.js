node.bcrypt.js
=============

[![Build Status](https://secure.travis-ci.org/ncb000gt/node.bcrypt.js.png)](http://travis-ci.org/#!/ncb000gt/node.bcrypt.js) 

Lib to help you hash passwords.
[bcrypt on wikipedia][bcryptwiki]

Catalyst for this module: [How To Safely Store A Password][codahale]


Version Compatability
=============

<table>
<tr>
<td>Node Ver</td><td>Bcrypt Version</td>
</tr>
<tr>
<td>&lt;= 0.4.x</td><td>&lt;= 0.4.x</td>
</tr>
<tr>
<td>&gt;= 0.6.x</td><td>&gt;= 0.5.x</td>
</tr>
</table>


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

Assuming you've already built node, you can compile the module with `node-gyp`:

    git clone git://github.com/ncb000gt/node.bcrypt.js.git
    cd node.bcrypt.js
    node-gyp configure
    node-gyp build

Usage - Sync
============

To hash a password:  

    var bcrypt = require('bcrypt');  
    var salt = bcrypt.genSaltSync(10);  
    var hash = bcrypt.hashSync("B4c0/\/", salt);
    // Store hash in your password DB.

To check a password:  

    // Load hash from your password DB.
    bcrypt.compareSync("B4c0/\/", hash); // true    
    bcrypt.compareSync("not_bacon", hash); // false

Usage - Async
============

To hash a password:  

    var bcrypt = require('bcrypt');  
    bcrypt.genSalt(10, function(err, salt) {
        bcrypt.hash("B4c0/\/", salt, function(err, hash) {
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

  * `genSaltSync(rounds, seed_length)`
    * `rounds` - [OPTIONAL] - the number of rounds to process the data for. (default - 10)
    * `seed_length` - [OPTIONAL] - RAND_bytes wants a length. to make that a bit flexible, you can specify a seed_length. (default - 20)
  * `genSalt(rounds, seed_length, cb)`
    * `rounds` - [OPTIONAL] - the number of rounds to process the data for. (default - 10)
    * `seed_length` - [OPTIONAL] - RAND_bytes wants a length. to make that a bit flexible, you can specify a seed_length. (default - 20)
    * `cb` - [REQUIRED] - a callback to be fired once the salt has been generated. uses eio making it asynchronous.
      * `err` - First parameter to the callback detailing any errors.
      * `salt` - Second parameter to the callback providing the generated salt.
  * `hashSync(data, salt)`
    * `data` - [REQUIRED] - the data to be encrypted.
    * `salt` - [REQUIRED] - the salt to be used in encryption.
  * `hash(data, salt, cb)`
    * `data` - [REQUIRED] - the data to be encrypted.
    * `salt` - [REQUIRED] - the salt to be used in encryption.
    * `cb` - [REQUIRED] - a callback to be fired once the data has been encrypted. uses eio making it asynchronous.
      * `err` - First parameter to the callback detailing any errors.
      * `encrypted` - Second parameter to the callback providing the encrypted form.
  * `compareSync(data, encrypted)`
    * `data` - [REQUIRED] - data to compare.
    * `encrypted` - [REQUIRED] - data to be compared to.
  * `compare(data, encrypted, cb)`
    * `data` - [REQUIRED] - data to compare.
    * `encrypted` - [REQUIRED] - data to be compared to.
    * `cb` - [REQUIRED] - a callback to be fired once the data has been compared. uses eio making it asynchronous.
      * `err` - First parameter to the callback detailing any errors.
      * `same` - Second parameter to the callback providing whether the data and encrypted forms match [true | false].
  * `getRounds(encrypted)` - return the number of rounds used to encrypt a given hash
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
* [Roman Shtylman][shtylman] - Code refactoring, general rot reduction, compile options, better memory management with delete and new, and an upgrade to libuv over eio/ev.
* [Vadim Graboys][vadimg] - Code changes to support 0.5.5+
* [Ben Noordhuis][bnoordhuis] - Fixed a thread safety issue in nodejs that was perfectly mappable to this module.

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
