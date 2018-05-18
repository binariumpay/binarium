Binarium staging tree 0.12.3
===============================

`master:` [![Build Status](https://travis-ci.org/dashpay/dash.svg?branch=master)](https://travis-ci.org/dashpay/dash) `develop:` [![Build Status](https://travis-ci.org/dashpay/dash.svg?branch=develop)](https://travis-ci.org/dashpay/dash/branches)

http://binarium.money


What is Binarium?
----------------

Binarium cryptocurrency is the first one, which is protected from ASICs. It does so by configurable hashing algorithm (with use of different hashing and encryption functions), which configures itself once per week or blocks generation difficulty change. This makes it costly to implement and own ASICs for each sub-function and reconfigure them in accordance with current algorithm state. And it uses data amplification with Salsa20 fast stream cipher. This also makes hashing function dependent on RAM sizes and random accesses speed. Which, in turn, makes it even more costly to build ASICs for and reduces GPU efficiency in mining, because their VRAM is better suited for piplined transfers of large data volumes, instead of speed of random accesses, and because their cache is shared between cores and part of it is read-only. Also, each thread, calculating hashes for Binarium blocks should have its own copy of memory area for data amplification, because this process and its intermediate data depends on information of block in consideration. With the main anti-ASIC factor is the ability to change hashing functions after Binarium widespread adoption, while keeping its current consensus.

Binarium cryptocurrency is the first one, which supports hashes functions changes and other major network-wide changes without breaking a consensus. It makes so by implementing updates in 4 steps: introduction of unactivated updates into clients; “soft” clients enforcing each other to update the software; “hard” clients enforcing each other for updates; and actual activation of changes in clients, when network reports satisfying level of changes adoption in it. In this model clients send upgraded peers statistics information to main Binarium RPC server, this allows to monitor network state and activate changes, when enough amount of users upgraded to new software versions. This functionality will be implemented after launch of cryptocurrency, when we’ll gather enough feedback on current algorithm and overall Binarium block-chain functioning.

It is based on Dash cryptocurrency and supports master nodes, InstantSend and PrivateSend functions from it.

We plan to make its integration with online games in form of:

- Ingames currency, which player can buy and sell for real money.
- Created by game developers in-game and near-game quests, which players can complete and receive reward in Binariums.
- Player to player trade agreements, from which game developers and Binarium developers receive commission.
- Games, DLCs, ingame items, ingame money and other valuables exchange.
- Ability to organize tournaments by game developers and gamers themselves with prizes in Binarium, games, DLCs, ingame items and other valuables.

Later on we plan to add smart-contracts, electronic documents management and other functions, which we’ll keep for now in secret. You can see yourself all technical details of Binarium functions implementation in Binarium source code.

It is an experimental new digital currency that enables anonymous, instant
payments to anyone, anywhere in the world. Binarium uses peer-to-peer technology
to operate with no central authority: managing transactions and issuing money
are carried out collectively by the network. Binarium Core is the name of the open
source software which enables the use of this currency.

For more information, as well as an immediately useable, binary version of
the Binarium Core software, see http://binarium.money .


License
-------

Binarium Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is meant to be stable. Development is normally done in separate branches.
[Tags](https://github.com/binariumpay/binarium/tags) are created to indicate new official,
stable release versions of Binarium Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows
and Linux, OS X, and that unit and sanity tests are automatically run.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Binarium Core's Transifex page](https://www.transifex.com/projects/p/binarium/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.

Translators should also follow the [forum](http://binarium.money/forum/topic/binarium-worldwide-collaboration.88/).
