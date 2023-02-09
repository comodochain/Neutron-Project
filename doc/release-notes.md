<<<<<<< HEAD
0.21.2 Release Notes
====================

Bitcoin Core version 0.21.2 is now available from:

  <https://bitcoincore.org/bin/bitcoin-core-0.21.2/>

This minor release includes various bug fixes and performance
improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/bitcoin/bitcoin/issues>

To receive security and update notifications, please subscribe to:

  <https://bitcoincore.org/en/list/announcements/join/>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes in some cases), then run the
installer (on Windows) or just copy over `/Applications/Bitcoin-Qt` (on Mac)
or `bitcoind`/`bitcoin-qt` (on Linux).

Upgrading directly from a version of Bitcoin Core that has reached its EOL is
possible, but it might take some time if the data directory needs to be migrated. Old
wallet versions of Bitcoin Core are generally supported.

Compatibility
==============

Bitcoin Core is supported and extensively tested on operating systems
using the Linux kernel, macOS 10.12+, and Windows 7 and newer.  Bitcoin
Core should also work on most other Unix-like systems but is not as
frequently tested on them.  It is not recommended to use Bitcoin Core on
unsupported systems.

From Bitcoin Core 0.20.0 onwards, macOS versions earlier than 10.12 are no
longer supported. Additionally, Bitcoin Core does not yet change appearance
when macOS "dark mode" is activated.


0.21.2 change log
=================

### P2P protocol and network code

- #21644 use NetPermissions::HasFlag() in CConnman::Bind() (jonatack)
- #22569 Rate limit the processing of rumoured addresses (sipa)

### Wallet

- #21907 Do not iterate a directory if having an error while accessing it (hebasto)

### RPC

- #19361 Reset scantxoutset progress before inferring descriptors (prusnak)

### Build System

- #21932 depends: update Qt 5.9 source url (kittywhiskers)
- #22017 Update Windows code signing certificate (achow101)
- #22191 Use custom MacOS code signing tool (achow101)
- #22713 Fix build with Boost 1.77.0 (sizeofvoid)

### Tests and QA

- #20182 Build with --enable-werror by default, and document exceptions (hebasto)
- #20535 Fix intermittent feature_taproot issue (MarcoFalke)
- #21663 Fix macOS brew install command (hebasto)
- #22279 add missing ECCVerifyHandle to base_encode_decode (apoelstra)
- #22730 Run fuzzer task for the master branch only (hebasto)

### GUI

- #277 Do not use QClipboard::Selection on Windows and macOS. (hebasto)
- #280 Remove user input from URI error message (prayank23)
- #365 Draw "eye" sign at the beginning of watch-only addresses (hebasto)

### Miscellaneous

- #22002 Fix crash when parsing command line with -noincludeconf=0 (MarcoFalke)
- #22137 util: Properly handle -noincludeconf on command line (take 2) (MarcoFalke)


Credits
=======

Thanks to everyone who directly contributed to this release:

- Andrew Chow
- Andrew Poelstra
- fanquake
- Hennadii Stepanov
- Jon Atack
- Kittywhiskers Van Gogh
- Luke Dashjr
- MarcoFalke
- Pavol Rusnak
- Pieter Wuille
- prayank23
- Rafael Sadowski
- W. J. van der Laan


As well as to everyone that helped with translations on
[Transifex](https://www.transifex.com/bitcoin/bitcoin/).
=======
0.8.7.5 changes
=============
- openssl-1.0.1k or older versions patched for CVE-2014-8275 broke compatibility with Bitcoin and Neutroncoin.
  This update patches Neutroncoin to maintain compatibility with CVE-2014-8275 patched openssl.
- If you are running v0.8.7.4 as distributed by neutroncoin.org you do not need to upgrade.
  The binaries distributed on neutroncoin.org contain their own copy of openssl so they are unaffected by this issue.

0.8.7.4 changes
=============
- Enforce v2 blocks at height 710000 on mainnet, 400000 on testnet
- Add `-maxorphantx=<n>` and `-maxorphanblocks=<n>` options for control over the maximum orphan transactions and blocks
- Stricter memory limits on CNode
- Upgrade OpenSSL to 1.0.1i (see https://www.openssl.org/news/secadv_20140806.txt - just to be sure, no critical issues

0.8.7.2 changes
=============
- Mac and Windows Official Gitian Builds: upgrade to openssl-1.0.1h for CVE-2014-0224
                   Linux Gitian build uses Lucid 0.9.8k-7ubuntu8.18

0.8.7.1 changes
=============
- Mac and Windows Official Gitian Builds: upgrade to openssl-1.0.1g for CVE-2014-0160
                   Linux was not vulnerable with Lucid openssl-0.9.8k
                   Older versions were only vulnerable with rarely used RPC SSL
- If you build from source, be sure that your openssl is patched for CVE-2014-0160.
- Upgrade openssl, qt, miniupnpc, zlib, libpng, qrencode
- Many bug fixes from Bitcoin 0.8.7rc stable branch
    including transaction malleability mitigation backports from 0.9
- Add testnet checkpoints
- Add new testnet seed

0.8.6.2 changes
=============

- Windows only: Fixes issue where network connectivity can fail.

- Cleanup of SSE2 scrypt detection.

- Minor fixes:
  - s/Bitcoin/Neutroncoin/ in the Coin Control example
  - Fix custom build on MacOS X 10.9
  - Fix QT5 custom build
  - Update Debian build instructions
  - Update Homebrew build 

0.8.6.1 changes
=============

- Coin Control - experts only GUI selection of inputs before you send a transaction

- Disable Wallet - reduces memory requirements, helpful for miner or relay nodes

- 20x reduction in default mintxfee.

- Up to 50% faster PoW validation, faster sync and reindexing.

- Peers older than protocol version 70002 are disconnected.  0.8.3.7 is the oldest compatible client.

- Internal miner added back to Neutroncoin.  setgenerate now works, although it is generally a bad idea as it is significantly slower than external CPU miners.

- New RPC commands: getbestblockhash and verifychain

- Improve fairness of the high priority transaction space per block

- OSX block chain database corruption fixes
  - Update leveldb to 1.13
  - Use fcntl with `F_FULLSYNC` instead of fsync on OSX
  - Use native Darwin memory barriers
  - Replace use of mmap in leveldb for improved reliability (only on OSX)

- Fix nodes forwarding transactions with empty vins and getting banned

- Network code performance and robustness improvements

- Additional debug.log logging for diagnosis of network problems, log timestamps by default

- Fix rare GUI crash on send

0.8.5.1 changes
===============

Workaround negative version numbers serialization bug.

Fix out-of-bounds check (Neutroncoin currently does not use this codepath, but we apply this
patch just to match Bitcoin 0.8.5.)

0.8.4.1 changes
===============

CVE-2013-5700 Bloom: filter crash issue - Neutroncoin 0.8.3.7 disabled bloom by default so was 
unaffected by this issue, but we include their patches anyway just in case folks want to 
enable bloomfilter=1.

CVE-2013-4165: RPC password timing guess vulnerability

CVE-2013-4627: Better fix for the fill-memory-with-orphaned-tx attack

Fix multi-block reorg transaction resurrection.

Fix non-standard disconnected transactions causing mempool orphans.  This bug could cause 
nodes running with the -debug flag to crash, although it was lot less likely on Neutroncoin 
as we disabled IsDust() in 0.8.3.x.

Mac OSX: use 'FD_FULLSYNC' with LevelDB, which will (hopefully!) prevent the database 
corruption issues have experienced on OSX.

Add height parameter to getnetworkhashps.

Fix Norwegian and Swedish translations.

Minor efficiency improvement in block peer request handling.


0.8.3.7 changes
===============

Fix CVE-2013-4627 denial of service, a memory exhaustion attack that could crash low-memory nodes.

Fix a regression that caused excessive writing of the peers.dat file.

Add option for bloom filtering.

Fix Hebrew translation.
>>>>>>> 0.8
