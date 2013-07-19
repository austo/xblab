xblab
=====
### An encrypted chat server inspired by David Chaum's "Dining Cryptographers" ###

xblab started as a Node.js addon and has inched steadily closer to being a standalone server and Node binding+library on the client. It relies heavily on libuv. While I realize no one needs another chat application, some of the functionality may have other uses for projects using Node and libuv.

### Dependencies ###

xblab relies on the following libraries, which are *not* included in this repo:

*   Event Loop and System

    [libuv](https://github.com/joyent/libuv) handles task scheduling, thread interaction, and networking.

*   Cryptography

    Cryptographic functionality comes from the excellent [Botan](http://botan.randombits.net/) library by Jack Lloyd.

*   Message Protocol 

    Prior to encryption, messages are serialized using [Google Protocol Buffers](https://developers.google.com/protocol-buffers/).

*   Database

    The Db class uses [pqxx](http://pqxx.org/development/libpqxx/), which depends on [libpq](http://www.postgresql.org/docs/9.2/static/libpq.html). You can swap in your own data layer by implementing db.h.

*   Misc

    Client builds are managed using [node-gyp](https://github.com/TooTallNate/node-gyp).
    Server configuration files are parsed using [YAJL](https://github.com/lloyd/yajl).
    Outside of JavaScript, regular expressions are handled via [PCRE](http://www.gammon.com.au/pcre/index.html).

If you've found this repo and have suggestions or would like information about getting it up and running on your system, please open an issue or send a pull request.