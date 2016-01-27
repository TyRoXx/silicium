silicium
========

A modern C++ library by Kevin Wichmann (TyRoXx). It contains generic
components he needs in other personal projects and experiments that will
be moved to their own libraries or removed eventually.

Nothing in here is well tested or ready for production yet. Any API can
change at any time before version ``1.0`` (see ``<silicium/version.hpp>``
for the current version).

compilers
=========

* [![Build Status](https://travis-ci.org/TyRoXx/silicium.svg?branch=master)](https://travis-ci.org/TyRoXx/silicium)
  the library is continuously tested with several versions of GCC on Ubuntu 12.04 (Boost 1.54 and 1.55)
  * the oldest release of GCC compatible with this library is 4.6
* [![Build status](https://ci.appveyor.com/api/projects/status/c3g0m66oe3t6e6ct/branch/master?svg=true)](https://ci.appveyor.com/project/TyRoXx/silicium/branch/master)
  Visual Studio 2015 is tested with Boost 1.56
* Clang is not being tested regularly yet
* OSX, BSDs and architectures other than x86 or AMD64 have not been tried yet

documentation
=============

There will be documentation when the APIs have stabilized to some extent.

[very work-in-progress, incomplete and usually outdated Doxygen](http://tyroxx.github.io/silicium/annotated.html)

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
  * use travis for OSX
  * appveyor improvements
    * Visual Studio 2012?
    * Visual Studio 2013?
    * multiple Boost verions
  * get valgrind to work without false positives
  * measure test coverage
* check whether variant&lt;T &amp;&gt; makes sense
* Asio-style async_wait for the end of a child process
* async read and write for anonymous pipes on Win32
* clean up noexcept annotations
* prevent all integer overflows in the library
* noexcept function to load a dynamic library
* replace the "trait" concept with the new "delegator"
* try precompiled headers
* rename error_or&lt;T, boost::system::error_code&gt; to system_result&lt;T&gt;?
* move everything HTTP into a separate library
* move the web server stuff to a separate library (including the uriparser dependency)

nice to have
------------

* static analysis
* move the zlib wrapper to a separate library
* define operator<< for basic_ostream, not ostream
* optimize safe integer arithmetic
  * GCC/Clang intrinsics?
  * avoid the div in operator*
* emulate variadic templates so that variant works on old compilers for a limited number of arguments
* drop-in replacement for boost::asio::spawn that works properly with old Boost versions
  * spawn does not support immediate completion prior to Boost 1.58
* a generic, ASIO-style "AsyncSource" concept which replaces Observable

probably outside of scope
-------------------------

* ASIO-style websockets
* investigate async read and write for files
* further investigation of container wrappers like Si::vector
* play with Visual C++ 2015 await: http://blogs.msdn.com/b/vcblog/archive/2014/11/12/resumable-functions-in-c.aspx
