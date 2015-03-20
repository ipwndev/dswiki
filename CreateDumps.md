Creating your own dump for DSwiki is a simple process in these three steps:
  1. Download
  1. Index
  1. Install

# 1. Download #

## Official Wikimedia Dumps ##

See http://code.google.com/p/dswiki/wiki/Dumps for a list of pre-made dumps, if your preferred language/project is not on that list, you have to create a dump by yourself.

Download the compressed XML dump from http://download.wikimedia.org/.

E.g. for the Wikipedia on Esperanto, go to http://download.wikimedia.org/eowiki/ and change into the directory with the highest date string (in my experience, _latest_ does not necessarily mean latest). Download the ...-pages-articles.xml.bz2 version! The other files do not include the necessary data, or have too much data in them.

## Wikia.com Dumps ##

wikia.com is a service that hosts mediawiki powered wikis, covering every possible topic and interest.

The complete download list from wikia.com can be found under this address: http://wikistats.wikia.com/dbdumps/dbdumps.html

~~For projects hosted on http://wikia.com, you can follow [this](http://forum.palib.info/index.php?topic=5616.msg40881#msg40881) guide if you are using windows, or [these](http://forum.palib.info/index.php?topic=5616.msg40501#msg40501) short instructions under Linux (and maybe Mac OS X).~~

The latest version of the indexer allows the direct indexing of any dumpfile, so it's not required to repack the archive first. So the index process is the same as mentioned below.

# 2. Index #

Download the latest indexer from http://code.google.com/p/dswiki/downloads/list and unpack it into the same directory that contains the article file.

Under Windows, run `indexer.exe <filename>` from the command line, under Linux execute `./indexer <filename>`. After the indexing process, you should have at least these five files (additional files will be created if the source files is too big):
  * articles.ifo
  * articles.dba
  * articles.idx
  * articles.ao1
  * articles.ao2

# 3. Install #

For every file, change "articles" into a better name for your dump. Copy all files into the directory /dswiki of your flash card.