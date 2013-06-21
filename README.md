# node.bcrypt.js

[![Build Status](https://secure.travis-ci.org/ncb000gt/node.bcrypt.js.png)](http://travis-ci.org/#!/ncb000gt/node.bcrypt.js) 

Lib to help you hash passwords.
[bcrypt on wikipedia][bcryptwiki]

Catalyst for this module: [How To Safely Store A Password][codahale]

## If You Are Submitting Bugs/Issues

Because we can't magically know what you are doing to expose an issue, it is best if you provide a snippet of code. This snippet need not include your secret sauce, but it must replicate the issue you are describing. The issues that get closed without resolution tend to be the ones without code examples. Thanks.


## Version Compatability

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

Windows users should make sure to have at least node 0.8.5 installed and version >= 0.7.1 of this module.  
`node-gyp` only works with stable/released versions of node. Since the `bcrypt` module uses `node-gyp` to build and install you'll need a stable version of node to use bcrypt. If you do not you'll likely see an error that starts with:

    gyp ERR! stack Error: "pre" versions of node cannot be installed, use the --nodedir flag instead



## Security Issues/Concerns

As should be the case with any security tool, this library should be scrutinized by anyone using it. If you find or suspect an issue with the code- please bring it to my attention and I'll spend some time trying to make sure that this tool is as secure as possible.

To make it easier for people using this tool to analyze what has been surveyed, here is a list of BCrypt related security issues/concerns as they've come up.

* An [issue with passwords][jtr] was found with a version of the Blowfish algorithm developed for John the Ripper. This is not present in the OpenBSD version and is thus not a problem for this module. HT [zooko][zooko].

## Dependencies

* NodeJS
* OpenSSL (Development Libraries (header files) for compilation)
 * For Windows you'll need http://slproweb.com/products/Win32OpenSSL.html installed to the default location of `C:\OpenSSL-Win32`
  * When installing OpenSSL, you must tell it to put DLLs in `The Windows system directory` to avoid `The specified module could not be found.` errors.
  * Please note that for this to build properly you'll need the Normal version of OpenSSL-Win<arch>, not the Light version. The reason for this is that we need to be able to compile the code using the header files that exist in the Normal version.
  * For 64 bit use the 64 bit version and install to `C:\OpenSSL-Win64`
* `node-gyp`
 * Please check the dependencies for this tool at: https://github.com/TooTallNate/node-gyp/
  * Windows users will need the options for c# and c++ installed with their visual studio instance.
  * Python 2.x

## Install via NPM
```
npm install bcrypt
```

## Usage

### async (recommended)

To hash a password:

```javascript
var bcrypt = require('bcrypt');
bcrypt.genSalt(10, function(err, salt) {
    bcrypt.hash("B4c0/\/", salt, function(err, hash) {
        // Store hash in your password DB.
    });
});
```

To check a password:

```javascript
// Load hash from your password DB.
bcrypt.compare("B4c0/\/", hash, function(err, res) {
    // res == true
});
bcrypt.compare("not_bacon", hash, function(err, res) {
    // res = false
});
```

Auto-gen a salt and hash:

```javascript
bcrypt.hash('bacon', 8, function(err, hash) {
});
```


### sync

To hash a password:

```javascript
var bcrypt = require('bcrypt');
var salt = bcrypt.genSaltSync(10);
var hash = bcrypt.hashSync("B4c0/\/", salt);
// Store hash in your password DB.
```

To check a password:

```javascript
// Load hash from your password DB.
bcrypt.compareSync("B4c0/\/", hash); // true
bcrypt.compareSync("not_bacon", hash); // false
```

Auto-gen a salt and hash:

```javascript
var hash = bcrypt.hashSync('bacon', 8);
```

## API

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
    * `salt` - [REQUIRED] - the salt to be used to hash the password. if specified as a number then a salt will be generated and used (see examples).
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


## Hash Info

The characters that comprise the resultant hash are `./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789$`.

## Testing

If you create a pull request, tests better pass :)

```
npm install
npm test
```

## Credits

The code for this comes from a few sources:

* blowfish.cc - OpenBSD
* bcrypt.cc - OpenBSD
* bcrypt::gen_salt - [gen_salt inclusion to bcrypt][bcryptgs]
* bcrypt_node.cc - me

## Contributors

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
* [Nate Rajlich][tootallnate] - Bindings and build process.
* [Sean McArthur][seanmonstar] - Windows Support
* [Fanie Oosthuysen][weareu] - Windows Support

## License
Unless stated elsewhere, file headers or otherwise, the license as stated in the LICENSE file.

[bcryptwiki]: http://en.wikipedia.org/wiki/Bcrypt
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
[bnoordhuis]:https://github.com/bnoordhuis
[tootallnate]:https://github.com/tootallnate
[seanmonstar]:https://github.com/seanmonstar
[weareu]:https://github.com/weareu
