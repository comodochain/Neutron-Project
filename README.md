<<<<<<< HEAD
Litecoin Core integration/staging tree
=====================================

[![Build Status](https://travis-ci.org/litecoin-project/litecoin.svg?branch=master)](https://travis-ci.org/litecoin-project/litecoin)

https://litecoin.org

What is Litecoin?
----------------

Litecoin is an experimental digital currency that enables instant payments to
anyone, anywhere in the world. Litecoin uses peer-to-peer technology to operate
with no central authority: managing transactions and issuing money are carried
out collectively by the network. Litecoin Core is the name of open source
software which enables the use of this currency.

For more information, as well as an immediately useable, binary version of
the Litecoin Core software, see [https://litecoin.org](https://litecoin.org).
=======
Neutroncoin integration/staging tree
================================

http://www.neutroncoin.org

Copyright (c) 2009-2014 Bitcoin Developers
Copyright (c) 2011-2014 Neutroncoin Developers

What is Neutroncoin?
----------------

Neutroncoin is a lite version of Bitcoin using scrypt as a proof-of-work algorithm.
 - 2.5 minute block targets
 - subsidy halves in 840k blocks (~4 years)
 - ~84 million total coins

The rest is the same as Bitcoin.
 - 50 coins per block
 - 2016 blocks to retarget difficulty

For more information, as well as an immediately useable, binary version of
the Neutroncoin client sofware, see http://www.neutroncoin.org.
>>>>>>> 0.8

License
-------

<<<<<<< HEAD
Litecoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.
=======
Neutroncoin is released under the terms of the MIT license. See `COPYING` for more
information or see http://opensource.org/licenses/MIT.
>>>>>>> 0.8

Development Process
-------------------

The `master` branch is regularly built (see `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be
completely stable. [Tags](https://github.com/litecoin-project/litecoin/tags) are created
regularly from release branches to indicate new official, stable release versions of Litecoin Core.

<<<<<<< HEAD
The https://github.com/litecoin-project/gui repository is used exclusively for the
development of the GUI. Its master branch is identical in all monotree
repositories. Release branches and tags do not exist, so please do not fork
that repository unless it is for development reasons.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).
=======
If it is a simple/trivial/non-controversial change, then one of the Neutroncoin
development team members simply pulls it.

If it is a *more complicated or potentially controversial* change, then the patch
submitter will be asked to start a discussion with the devs and community.
>>>>>>> 0.8

The developer [mailing list](https://groups.google.com/forum/#!forum/litecoin-dev)
should be used to discuss complicated or controversial changes before working
on a patch set.

<<<<<<< HEAD
Developer IRC can be found on Freenode at #litecoin-dev.
=======
The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/neutroncoin-project/neutroncoin/tags) are created
regularly to indicate new official, stable release versions of Neutroncoin.
>>>>>>> 0.8

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

<<<<<<< HEAD
The Travis CI system makes sure that every pull request is built for Windows, Linux, and macOS, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

We only accept translation fixes that are submitted through [Bitcoin Core's Transifex page](https://www.transifex.com/projects/p/bitcoin/).
Translations are converted to Litecoin periodically.

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
=======
    cd src; make -f makefile.unix test

Unit tests for the GUI code are in `src/qt/test/`. To compile and run them:

    qmake BITCOIN_QT_TEST=1 -o Makefile.test bitcoin-qt.pro
    make -f Makefile.test
    ./neutroncoin-qt_test

>>>>>>> 0.8
