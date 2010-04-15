bcrypt-node
=============

Lib to help you hash passwords.
http://en.wikipedia.org/wiki/Crypt_(Unix)#Blowfish-based_scheme


Dependencies
=============

BSD Libs
NodeJS


To Build
============

I run K/Ubuntu. I was able to pull in the bsd libs using:  
`sudo apt-get install libbsd-dev`

What is required is the file `random.h` within `/usr/includes/bsd/` and the compiled bsd libs in `/usr/lib/`. These have to match the conf.check.

Assuming you've already built node, you can run the waf script:  
`node-waf configure  
node-waf build`


Usage
============

To hash a password:  
`var bc = new bcrypt.BCrypt();
var salt = bc.gen_salt(20);
var hash = bc.hashpw("B4c0/\/", salt);`

To check a password:  
`bc.compare("B4c0/\/", hash); // true`  
`bc.compare("not_bacon", hash); // false`


Credits
============

The code for this comes from a few sources:

* blowfish.cc - OpenBSD
* bcrypt.cc - OpenBSD
* bcrypt::gen_salt - http://mail-index.netbsd.org/tech-crypto/2002/05/24/msg000204.html
* bcrypt_node.cc - me


License
============

Unless stated elsewhere, file headers or otherwise, the license is as follows:

/*
 * Copyright (c) 2101, Nicholas Campbell <nicholas.j.campbell@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 *  1. Redistributions of source code must retain the above copyright 
 * notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the <ORGANIZATION> nor the names of its 
 * contributors may be used to endorse or promote products derived from 
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */