silicium
========

Linux: [![Build Status](https://travis-ci.org/TyRoXx/silicium.svg?branch=master)](https://travis-ci.org/TyRoXx/silicium)

Windows: [![Build status](https://ci.appveyor.com/api/projects/status/c3g0m66oe3t6e6ct/branch/master?svg=true)](https://ci.appveyor.com/project/TyRoXx/silicium/branch/master)

[very work-in-progress, incomplete and usually outdated Doxygen](http://tyroxx.github.io/silicium/annotated.html)

compilers
=========

* GCC 4.8 (older versions may work)
* Visual C++ 2012
    * but many features require 2013 or even 2015
* recent versions of Clang will probably work, too

Regularly tested configurations at the moment:
* GCC 4.8 on Ubuntu 14.04 with Boost 1.54
* Visual C++ 2015 with Boost 1.59

There will be CI eventually to ensure compatibility with all supported compilers.

dependencies
============

This installs the dependencies on Ubuntu 14.04:

```
sudo apt-get install libboost-all-dev liburiparser-dev zlib1g-dev
```

On ancient operating systems, you may have to install some of the
libraries from a recent source release.

Boost 1.49 or later (required)
------------------------------

Many components of the library are only available with more recent versions of Boost though.

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

important
---------

* continuous integration
  * [x] create .travis.yml and let travis-ci run it
  * [x] travis support finished
  * [ ] travis upgrade: http://docs.travis-ci.com/user/migrating-from-legacy/?utm_source=legacy-notice&utm_medium=banner&utm_campaign=legacy-upgrade
  * [ ] use travis for OSX
  * [ ] appveyor integration finished
* measure test coverage
* allow for recursive variants
* check whether variant&lt;T &amp;&gt; makes sense
* Asio-style async_wait for the end of a child process
* async read and write for anonymous pipes on Win32
* clean up noexcept annotations
* prevent all integer overflows in the library
* noexcept function to load a dynamic library

nice to have
------------

* static analysis
* move the web server stuff to a separate library (including the uriparser dependency)
* move the zlib wrapper to a separate library
* reduce sizeof(fast_variant&lt;T&gt;) to sizeof(T)
* reduce sizeof(fast_variant&lt;char&gt;) to (sizeof(char) * 2)
* move everything HTTP into a separate library
* do not #include <boost/concept_check.hpp> just to get boost::ignore_unused_variable_warning
* play with Visual C++ 2015 await: http://blogs.msdn.com/b/vcblog/archive/2014/11/12/resumable-functions-in-c.aspx
* define operator<< for basic_ostream, not ostream
* optimize safe integer arithmetic
  * GCC/Clang intrinsics?
  * avoid the div in operator*

probably outside of scope
-------------------------

* Asio-style websockets
* investigate async read and write for files
* further investigation of container wrappers like Si::vector
