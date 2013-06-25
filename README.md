xblab
=====
An encrypted chat server inspired by David Chaum's "Dining Cryptographers Protocol"
-----------------------------------------------------------------------------------

xblab is basically a Node.js project implemented wherever possible in C++.
While the wisdom of such an endeavor is open to debate, it may prove useful to other Node.js addon authors.

Cryptography comes from the excellent [Botan](http://botan.randombits.net/) library by Jack Lloyd, which is not included in this repo.

Another dependency is [Google Protocol Buffers](https://developers.google.com/protocol-buffers/), which are used for binary transmission over TCP.

Builds are managed using node-gyp. If you've found this repo and would like information about getting it up and running on your system, please open an issue or send a pull request.