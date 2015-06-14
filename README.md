silicium
========

compilers
=========

* GCC 4.8 (older versions may work)
* Visual C++ 2013 Update 3 (older versions will not work)
* recent versions of Clang will probably work, too

dependencies
============

This installs the dependencies on Ubuntu 14.04:

```
sudo apt-get install libboost-all-dev liburiparser-dev zlib1g-dev
```

On ancient operating systems, you may have to install some of the
libraries from a recent source release.

Boost 1.54 or later (required)
------------------------------

http://www.boost.org/users/download/

```
sudo apt-get install libboost-all-dev
```

UriParser 0.7 or later (optional)
---------------------------------

http://uriparser.sourceforge.net/#download

```
sudo apt-get install liburiparser-dev
```

zlib (optional)
---------------

http://zlib.net/

```
sudo apt-get install zlib1g-dev
```

to do
=====

* measure test coverage
* static analysis
* move the web server stuff to a separate library (including the uriparser dependency)
* move the zlib wrapper to a separate library
* rename fast_variant to either flat_variant or variant
* allow for recursive variants
* reduce sizeof(fast_variant<T>) to sizeof(T)
* reduce sizeof(fast_variant<char>) to (sizeof(char) * 2)
* check whether fast_variant<T &> makes sense
* Asio-style async_wait for the end of a child process
* async read and write for anonymous pipes on Win32
* further investigation of container wrappers like Si::vector
* move everything HTTP into a separate library
* Asio-style websockets
* investigate async read and write for files
