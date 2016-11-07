setport
========

Installation
-------------

Installation requires g++ 5.

To install setport, change the source code directory and run `make install`.
This will compile the program and test suite, run the test suite, and, if all
tests are successful, create a bash alias to setport. To only compile and run
tests without a full installation, run `make runtest`.

The bash alias will be available the next time you log in to your computer.
To use it immediately (only in your current shell),run `source ~/.bash_aliases`.

Due to limitations of the install process, setport can only be run from
the directory containing its localization files (setport.*.txt).


Translation
------------

setport chooses its output language by checking the locale specified in the
following locale environment variables, in the given order:

- LANGUAGE
- LC_ALL
- LC_MESSAGES
- LANG

Translation requires that all localization files (setport.*.txt) exist in the
target language. For example, to enable Polish translation, you must provide
setport.about\_pl.txt, setport.usage\_pl.txt, and setport.messages\_pl.txt.

License
--------

setport © 2016 Brandon Dyck
Distributed under the WTFPL v2. See LICENSE.txt for more info.
