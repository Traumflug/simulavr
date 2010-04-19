Download
========

Project homepage is available at http://savannah.nongnu.org/projects/simulavr.
There you'll find also a link to download area. If you want to download other
versions, please use the link to download area!

.. note::

  In the moment we have only some old versions available for download.
  
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
example, you've downloaded ``simulavr-0.1.2.6.tar.gz`` together with
``simulavr-0.1.2.6.tar.gz.sig``)::
  
  > gpg --verify simulavr-0.1.2.6.tar.gz.sig
  
If there is no message, that file is invalid, you can use your downloaded file.
(of course, you can use it also without verifying signature, but on your own
risk!)

Linux
-----

Precompiled versions for linux are not available at the moment.

Windows
-------

Precompiled versions for Windows platform are not available at the moment.

Documentation
-------------

Downloadable documentation and documentation packages aren't available at the
moment.

Source tarball
--------------

More versions are available on download area on project homepage!

.. list-table::
   :widths: 10 10 20 20
   :header-rows: 1
  
   * - Version
     - Date
     - Download
     - Signature
   * - 0.1.2.6
     - 05.03.2005
     - `simulavr-0.1.2.6.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavr-0.1.2.6.tar.gz>`__
     - `simulavr-0.1.2.6.tar.gz.sig <http://download.savannah.nongnu.org/releases/simulavr/simulavr-0.1.2.6.tar.gz.sig>`__
   * - 0.8.006
     - 30.07.2005
     - `simulavrxx-0.8.006.tar.gz <http://download.savannah.nongnu.org/releases/simulavr/simulavrxx-0.8.006.tar.gz>`__
     - `simulavrxx-0.8.006.tar.gz.sig <http://download.savannah.nongnu.org/releases/simulavr/simulavrxx-0.8.006.tar.gz.sig>`__
       *Attention! There is no valid GPG key available for now!* 

