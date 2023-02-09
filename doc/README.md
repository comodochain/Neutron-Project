<<<<<<< HEAD
Litecoin Core
=============

Setup
---------------------
Litecoin Core is the original Litecoin client and it builds the backbone of the network. It downloads and, by default, stores the entire history of Litecoin transactions, which requires approximately 22 gigabytes of disk space. Depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more.

To download Litecoin Core, visit [litecoin.org](https://litecoin.org/).

Running
---------------------
The following are some helpful notes on how to run Litecoin Core on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/litecoin-qt` (GUI) or
- `bin/litecoind` (headless)

### Windows
=======
Neutroncoin 0.8.x BETA
====================

Copyright (c) 2009-2014 Bitcoin Developers
Copyright (c) 2011-2014 Neutroncoin Developers

Distributed under the MIT/X11 software license, see the accompanying
file COPYING or http://www.opensource.org/licenses/mit-license.php.
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](http://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.


Intro
---------------------
Neutroncoin is a free open source peer-to-peer electronic cash system that is
completely decentralized, without the need for a central server or trusted
parties.  Users hold the crypto keys to their own money and transact directly
with each other, with the help of a P2P network to check for double-spending.


Setup
---------------------
You need the Qt4 run-time libraries to run Neutroncoin-Qt. On Debian or Ubuntu:
	`sudo apt-get install libqtgui4`

Unpack the files into a directory and run:

- bin/32/neutroncoin-qt (GUI, 32-bit)
- bin/32/neutroncoind (headless, 32-bit)
- bin/64/neutroncoin-qt (GUI, 64-bit)
- bin/64/neutroncoind (headless, 64-bit)

See the documentation at the [Neutroncoin Wiki](http://neutroncoin.info)
for help and more information.
>>>>>>> 0.8

Unpack the files into a directory, and then run litecoin-qt.exe.

### macOS

Drag Litecoin Core to your applications folder, and then run Litecoin Core.

### Need Help?

* See the documentation at the [Litecoin Wiki](https://litecoin.info/) for help and more information.
* Ask for help on [#litecoin](https://webchat.freenode.net/#litecoin) on Freenode. If you don't have an IRC client, use [webchat here](https://webchat.freenode.net/#litecoin).
* Ask for help on the [LitecoinTalk](https://litecointalk.io/) forums, in the [Technical Support board](https://litecointalk.io/c/technical-support).

Building
---------------------
The following are developer notes on how to build Litecoin Core on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [Dependencies](dependencies.md)
- [macOS Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [FreeBSD Build Notes](build-freebsd.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [NetBSD Build Notes](build-netbsd.md)
- [Gitian Building Guide (External Link)](https://github.com/bitcoin-core/docs/blob/master/gitian-building.md)

Development
---------------------
The Litecoin repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Productivity Notes](productivity.md)
- [Release Notes](release-notes.md)
<<<<<<< HEAD
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://doxygen.bitcoincore.org/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [JSON-RPC Interface](JSON-RPC-interface.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)

### Resources
* Discuss on the [LitecoinTalk](https://litecointalk.io/) forums.
* Discuss general Litecoin development on #litecoin-dev on Freenode. If you don't have an IRC client, use [webchat here](https://webchat.freenode.net/#litecoin-dev).

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [bitcoin.conf Configuration File](bitcoin-conf.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [Reduce Memory](reduce-memory.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [ZMQ](zmq.md)
- [PSBT support](psbt.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
=======
- [Multiwallet Qt Development](multiwallet-qt.md)
- [Unit Tests](unit-tests.md)
- [Translation Process](translation_process.md)
>>>>>>> 0.8
