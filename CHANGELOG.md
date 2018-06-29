# UNRELEASED

  * Drop support for NodeJS <= 4

# 2.0.1 (2018-04-20)

  * Update `node-pre-gyp` to allow downloading prebuilt modules

# 2.0.0 (2018-04-07)

  * Make `2b` the default bcrypt version

# 1.0.2 (2016-12-31)

  * Fix `compare` promise rejection with invalid arguments

# 1.0.1 (2016-12-07)

  * Fix destructuring imports with promises

# 1.0.0 (2016-12-04)

  * add Promise support (commit 2488473)

# 0.8.7 (2016-06-09)

  * update nan to 2.3.5 for improved node v6 support

# 0.8.6 (2016-04-20)

  * update nan for node v6 support

# 0.8.5 (2015-08-12)

  * update to nan v2 (adds support for iojs 3)

# 0.8.4 (2015-07-24)

  * fix deprecation warning for the Encode API

# 0.8.3 (2015-05-06)

  * update nan to 1.8.4 for iojs 2.x support

# 0.8.2 (2015-03-28)

  * always use callback for generating random bytes to avoid blocking

# 0.8.1 (2015-01-18)
  * update NaN to 1.5.0 for iojs support

# 0.8.0 (2014-08-03)
  * migrate to NAN for bindings

# v0.5.0
  * Fix for issue around empty string params throwing Errors.
  * Method deprecation.
  * Upgrade from libeio/ev to libuv. (shtylman)
  ** --- NOTE --- Breaks 0.4.x compatability
  * EV_MULTIPLICITY compile flag.

# v0.4.1
  * Thread safety fix around OpenSSL (GH-32). (bnoordhuis - through node)
  * C++ code changes using delete and new instead of malloc and free. (shtylman)
  * Compile options for speed, zoom. (shtylman)
  * Move much of the type and variable checking to the JS. (shtylman)

# v0.4.0
  * Added getRounds function that will tell you the number of rounds within a hash/salt

# v0.3.2
  * Fix api issue with async salt gen first param

# v0.3.1
  * Compile under node 0.5.x

# v0.3.0
  * Internal Refactoring
  * Remove pthread dependencies and locking
  * Fix compiler warnings and a memory bug

# v0.2.4
  * Use threadsafe functions instead of pthread mutexes
  * salt validation to make sure the salt is of the correct size and format

# v0.2.3
  * cygwin support

# v0.2.2
  * Remove dependency on libbsd, use libssl instead

# v0.2.0
  * Added async functionality
  * API changes
    * hashpw -> encrypt
    * all old sync methods now end with _sync
  * Removed libbsd(arc4random) dependency...now uses openssl which is more widely spread

# v0.1.2
  * Security fix. Wasn't reading rounds in properly and was always only using 4 rounds
