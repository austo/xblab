xblab
=====
An encrypted chat server inspired by David Chaum's "Dining Cryptographers"
--------------------------------------------------------------------------

xblab is basically a Node.js project implemented wherever possible in C++.
While the wisdom of such an endeavor is open to debate, it may have some value to other Node.js addon authors.

### Dependencies ###

xblab relies on the following libraries, none of which are *not* included in this repo.

*   ##### Cryptography #####
    Cryptographic functionality comes from the excellent [Botan](http://botan.randombits.net/) library by Jack Lloyd.

*   ##### Message Protocol #####   
    Prior to encryption, messages are serialized using [Google Protocol Buffers](https://developers.google.com/protocol-buffers/).

*   ##### Database #####
    xblab stores user and group data in [PostgreSQL](http://www.postgresql.org/), but that's not really a requirement. The Db class uses [pqxx](http://pqxx.org/development/libpqxx/), which depends on [libpq](http://www.postgresql.org/docs/9.2/static/libpq.html). You can swap in your own data layer by implementing db.h.

*   ##### Build #####
    Builds are managed using [node-gyp](https://github.com/TooTallNate/node-gyp).

If you've found this repo and have suggestions or would like information about getting it up and running on your system, please open an issue or send a pull request.