<<<<<<< HEAD
# macOS Build Instructions and Notes
=======
Mac OS X neutroncoind build instructions
====================================
>>>>>>> 0.8

The commands in this guide should be executed in a Terminal application.
The built-in one is located in
```
/Applications/Utilities/Terminal.app
```

## Preparation
Install the macOS command line tools:

```shell
xcode-select --install
```

When the popup appears, click `Install`.

Then install [Homebrew](https://brew.sh).

## Dependencies
```shell
brew install automake libtool boost miniupnpc pkg-config python qt libevent qrencode fmt
```

If you run into issues, check [Homebrew's troubleshooting page](https://docs.brew.sh/Troubleshooting).
See [dependencies.md](dependencies.md) for a complete overview.

If you want to build the disk image with `make deploy` (.dmg / optional), you need RSVG:
```shell
brew install librsvg
```

<<<<<<< HEAD
The wallet support requires one or both of the dependencies ([*SQLite*](#sqlite) and [*Berkeley DB*](#berkeley-db)) in the sections below.
To build Bitcoin Core without wallet, see [*Disable-wallet mode*](#disable-wallet-mode).
=======
See `doc/readme-qt.rst` for instructions on building Neutroncoin-Qt, the
graphical user interface.
>>>>>>> 0.8

#### SQLite

Usually, macOS installation already has a suitable SQLite installation.
Also, the Homebrew package could be installed:

```shell
brew install sqlite
```

In that case the Homebrew package will prevail.

#### Berkeley DB

It is recommended to use Berkeley DB 4.8. If you have to build it yourself,
you can use [this](/contrib/install_db4.sh) script to install it
like so:

```shell
./contrib/install_db4.sh .
```

from the root of the repository.

Also, the Homebrew package could be installed:

```shell
brew install berkeley-db4
```

## Build Litecoin Core

<<<<<<< HEAD
1. Clone the Litecoin Core source code:
    ```shell
    git clone https://github.com/litecoin-project/litecoin
    cd litecoin
    ```
=======
### Building `neutroncoind`
>>>>>>> 0.8

2.  Build Litecoin Core:

<<<<<<< HEAD
    Configure and build the headless Litecoin Core binaries as well as the GUI (if Qt is found).

    You can disable the GUI build by passing `--without-gui` to configure.
    ```shell
    ./autogen.sh
    ./configure
    make
    ```
=======
        git clone git@github.com:neutroncoin-project/neutroncoin.git neutroncoin
        cd neutroncoin

2.  Build neutroncoind:
>>>>>>> 0.8

3.  It is recommended to build and run the unit tests:
    ```shell
    make check
    ```

4.  You can also create a  `.dmg` that contains the `.app` bundle (optional):
    ```shell
    make deploy
    ```

## Disable-wallet mode
When the intention is to run only a P2P node without a wallet, Litecoin Core may be
compiled in disable-wallet mode with:
```shell
./configure --disable-wallet
```

In this case there is no dependency on [*Berkeley DB*](#berkeley-db) and [*SQLite*](#sqlite).

Mining is also possible in disable-wallet mode using the `getblocktemplate` RPC call.

## Running
Litecoin Core is now available at `./src/litecoind`

Before running, you may create an empty configuration file:
```shell
mkdir -p "/Users/${USER}/Library/Application Support/Litecoin"

touch "/Users/${USER}/Library/Application Support/Litecoin/litecoin.conf"

chmod 600 "/Users/${USER}/Library/Application Support/Litecoin/litecoin.conf"
```

The first time you run litecoind, it will start downloading the blockchain. This process could
take many hours, or even days on slower than average systems.

You can monitor the download process by looking at the debug.log file:
```shell
tail -f $HOME/Library/Application\ Support/Litecoin/debug.log
```

## Other commands:
```shell
./src/litecoind -daemon      # Starts the litecoin daemon.
./src/litecoin-cli --help    # Outputs a list of command-line options.
./src/litecoin-cli help      # Outputs a list of RPC commands when the daemon is running.
```

<<<<<<< HEAD
## Notes
* Tested on OS X 10.14 Mojave through macOS 11 Big Sur on 64-bit Intel
processors only.
* Building with downloaded Qt binaries is not officially supported. See the notes in [#7714](https://github.com/bitcoin/bitcoin/issues/7714).
=======
### Building `neutroncoind`

1. Clone the github tree to get the source code and go into the directory.

        git clone https://github.com/neutroncoin-project/neutroncoin.git
        cd neutroncoin

2.  Modify source in order to pick up the `openssl` library.

    Edit `makefile.osx` to account for library location differences. There's a
    diff in `contrib/homebrew/makefile.osx.patch` that shows what you need to
    change, or you can just patch by doing

        patch -p1 < contrib/homebrew/makefile.osx.patch

3.  Build neutroncoind:

        cd src
        make -f makefile.osx

4.  It is a good idea to build and run the unit tests, too:

        make -f makefile.osx test

Creating a release build
------------------------

A neutroncoind binary is not included in the Neutroncoin-Qt.app bundle. You can ignore
this section if you are building `neutroncoind` for your own use.

If you are building `litecond` for others, your build machine should be set up
as follows for maximum compatibility:

All dependencies should be compiled with these flags:

    -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk

For MacPorts, that means editing your macports.conf and setting
`macosx_deployment_target` and `build_arch`:

    macosx_deployment_target=10.5
    build_arch=i386

... and then uninstalling and re-installing, or simply rebuilding, all ports.

As of December 2012, the `boost` port does not obey `macosx_deployment_target`.
Download `http://gavinandresen-bitcoin.s3.amazonaws.com/boost_macports_fix.zip`
for a fix. Some ports also seem to obey either `build_arch` or
`macosx_deployment_target`, but not both at the same time. For example, building
on an OS X 10.6 64-bit machine fails. Official release builds of Neutroncoin-Qt are
compiled on an OS X 10.6 32-bit machine to workaround that problem.

Once dependencies are compiled, creating `Neutroncoin-Qt.app` is easy:

    make -f Makefile.osx RELEASE=1

Running
-------

It's now available at `./neutroncoind`, provided that you are still in the `src`
directory. We have to first create the RPC configuration file, though.

Run `./neutroncoind` to get the filename where it should be put, or just try these
commands:

    echo -e "rpcuser=neutroncoinrpc\nrpcpassword=$(xxd -l 16 -p /dev/urandom)" > "/Users/${USER}/Library/Application Support/Neutroncoin/neutroncoin.conf"
    chmod 600 "/Users/${USER}/Library/Application Support/Neutroncoin/neutroncoin.conf"

When next you run it, it will start downloading the blockchain, but it won't
output anything while it's doing this. This process may take several hours.

Other commands:

    ./neutroncoind --help  # for a list of command-line options.
    ./neutroncoind -daemon # to start the neutroncoin daemon.
    ./neutroncoind help    # When the daemon is running, to get a list of RPC commands
>>>>>>> 0.8
