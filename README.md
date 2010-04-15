bcrypt-node
=============

Lib to help you hash passwords.  
[bcrypt on wikipedia][bcryptwiki]


Dependencies
=============

* NodeJS
* BSD Libs


To Build
============

I run K/Ubuntu. I was able to pull in the bsd libs using:  
    sudo apt-get install libbsd-dev  
_Eventually I'd like to get it so that the libs are all built in to the same package. But, as a work in progress, this gets the job done for now._

What is required is the file `random.h` within `/usr/includes/bsd/` and the compiled bsd libs in `/usr/lib/`. These have to match the conf.check.

Assuming you've already built node, you can run the waf script:  
    node-waf configure  
    node-waf build


Usage
============

To hash a password:  
    var bc = new bcrypt.BCrypt();  
    var salt = bc.gen_salt(20);  
    var hash = bc.hashpw("B4c0/\/", salt);

To check a password:  
    bc.compare("B4c0/\/", hash); // true    
    bc.compare("not_bacon", hash); // false


Credits
============

The code for this comes from a few sources:

* blowfish.cc - OpenBSD
* bcrypt.cc - OpenBSD
* bcrypt::gen_salt - [gen_salt inclusion to bcrypt][bcryptgs]
* bcrypt_node.cc - me


License
============

Unless stated elsewhere, file headers or otherwise, the license as stated in the LICENSE file.





[bcryptwiki]: http://en.wikipedia.org/wiki/Crypt_(Unix)#Blowfish-based_scheme  
[bcryptgs]: http://mail-index.netbsd.org/tech-crypto/2002/05/24/msg000204.html