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
