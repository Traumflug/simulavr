Download
========

Project homepage is available at http://savannah.nongnu.org/projects/simulavr.
There you'll find also a link to download area. If you want to download other
versions, please use the link to download area!

Secure download
---------------

Releases are secured by gpg signatures. For every package, tarball, document,
which you can download here, you'll find a signature file too. This is a
cryptographic checksum over the released file and helps you to find out, if
this file is unchanged by somebody unauthorized.

For this, you need a ``gpg`` installation and our
`gpg keyring <https://savannah.nongnu.org/project/memberlist-gpgkeys.php?group=simulavr>`__.
Download this keyring and import it to your keyring::
  
  > gpg --import simulavr-keyring.gpg
  
You can list out, what's now in your keyring::
  
  > gpg --list-keys
  
After you have downloaded release file (tarball, document, binary package) together
with the signature file, you can verify, that your download is correct (for
example, you've downloaded ``simulavr-1.0.0.tar.gz`` together with
``simulavr-1.0.0.tar.gz.sig``)::
  
  > gpg --verify simulavr-1.0.0.tar.gz.sig
  
If there is no message, that file is invalid, you can use your downloaded file.
(of course, you can use it also without verifying signature, but on your own
risk!)

Release 1.0.0
-------------

**Release date:** february 12th, 2012.

.. list-table::
   :widths: 10 10
   :header-rows: 1
  
   * - Package
     - Description
   * - `simulavr-1.0.0.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavr-1.0.0.tar.gz>`__ (990k)
       and `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/simulavr-1.0.0.tar.gz.sig>`__
     - Source tarball for all systems, see build instructions for usage
   * - `manual-1.0.pdf <http://download.savannah.nongnu.org/releases/simulavr/manual-1.0.pdf>`__ (1.1M) and
       `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/manual-1.0.pdf.sig>`__
     - Manual in pdf format
   * - `simulavr-1.0.0-binary-linux32.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavr-1.0.0-binary-linux32.tar.gz>`__
       (17M) and `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/simulavr-1.0.0-binary-linux32.tar.gz.sig>`__ 
     - Binary for linux, built on ubuntu 10.04 32bit, but should work on mostly all linux distributions.
       Install it somewhere you want in your home path or with root rights to sytem root to make it
       accessible for all users. Includes api documentation, man pages, header files and examples
   * - `simulavr-1.0.0-binary-win7-32bit.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavr-1.0.0-binary-win7-32bit.tar.gz>`__
       (6.4M) and `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/simulavr-1.0.0-binary-win7-32bit.tar.gz.sig>`__
     - Binary for windows, built with MSys for Windows7 32bit. Install it anywhere in your system, for example in a
       subdirectory on programs path
   * - `simulavr-1.0-api-documentation.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavr-1.0-api-documentation.tar.gz>`__
       (195M) and `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/simulavr-1.0-api-documentation.tar.gz.sig>`__
     - Doxygen API documentation in html format.
   * - `pysimulavr-1.0.0-linux32-py2.6.egg <http://download.savannah.nongnu.org/releases/simulavr/pysimulavr-1.0.0-linux32-py2.6.egg>`__
       (1.2M) and `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/pysimulavr-1.0.0-linux32-py2.6.egg.sig>`__
     - Python egg for python 2.6 on linux, install it with easy_install or pip. Should also work on python 2.4 to
       2.7. Dosn't need simulavr installation. (an egg is a zip container, you can also unpack it with unzip
       anywhere in your python environment)
   * - `pysimulavr-1.0.0-win7-32bit-py2.6-.egg <http://download.savannah.nongnu.org/releases/simulavr/pysimulavr-1.0.0-win7-32bit-py2.6.egg>`__
       (3.7M) and `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/pysimulavr-1.0.0-win7-32bit-py2.6.egg.sig>`__
     - Python egg for python 2.6 on win7, install it with easy_install or pip. Should also work on python 2.4 to
       2.7. Dosn't need simulavr installation. (an egg is a zip container, you can also unpack it with unzip
       anywhere in your python environment)

Older versions
--------------

This are older versions before july 2011. The first two are from the old simulavr, written in C. The second
one a earlier release of new C++ version, starting in 2005.
 
.. list-table::
   :widths: 10 10 30
   :header-rows: 1
  
   * - Version
     - Date
     - Download
   * - 0.1.2.6
     - 05.03.2009
     - `simulavr-0.1.2.6.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavr-0.1.2.6.tar.gz>`__
       and `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/simulavr-0.1.2.6.tar.gz.sig>`__
   * - 0.1.2.7
     - 03.07.2011
     - `simulavr-0.1.2.7.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavr-0.1.2.7.tar.gz>`__
       and `gpg signature <http://download.savannah.nongnu.org/releases/simulavr/simulavr-0.1.2.7.tar.gz.sig>`__
   * - 0.8.006
     - 30.07.2005
     - `simulavrxx-0.8.006.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavrxx-0.8.006.tar.gz>`__,
       gpg signature not available

More versions are available on download area on project homepage!

