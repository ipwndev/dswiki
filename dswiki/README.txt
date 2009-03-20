 DSwiki (v0.3alpha2)
=====================

DSwiki is an offline viewer for Wiki markup, for example for the
Wikipedia, Wikibooks, Wiktionaries, ...

DSwiki is NOT a web browser!

 Why DSwiki?
=============

DSwiki is optimal for all people with a Nintendo DS that like to
have their information "portable", but cannot access the Internet
directly. So far, no Program was very good in displaying the
Wikipedia on the DS. NewDictS can display an uncompressed dump, but
this dump is outdated, and the Wikipedia has grown dramatically
since then, so compression was absolutely necessary. In addition,
most homebrew programs can't handle UTF-8.

 Features
==========

-- Get the complete(!) Wikipedia on a Nintendo DS. The English language
   edition fits on a 4GB storage device, the German edition is just 1.2GB.
-- Quite good handling of internal links. Alternative display text
   and appended letters are supported to create the same look as
   in the online version. Subpage links are supported.
-- Very good Unicode/UTF8-Support (as much - or as little - as the
   underlying font covers)
-- Fast and easy searchable index
-- Bookmarks

 Usage
=======

Just take the tutorial at the first start, or start it any time later
from the menu.

 Installation
==============

1. Copy DSwiki.nds somewhere on your flash card, DLDI-patch it if
   necessary.

2. Create dumps of your favorite wiki or download already prepared ones.
   Visit <http://code.google.com/p/dswiki/wiki/Dumps> for a list.

3. Copy all files with the extensions

     ifo
     idx
     ao1
     ao2
     db?    [ ? = one letter, typically "a" or "b", "z" is used in older versions ]

   into /dswiki. This directory must exist in the root directory of your flash card.

 Planned Features
==================

-- Improved Search (advanced input)
-- "Find as you type" inside an article
-- Switching between fonts on the fly
-- Customization

 DSwiki is based on
====================

-- NewDictS       <http://tvgame360.com.tw/viewthread.php?tid=21913&extra=&page=1>
                    Thanks to sesa for graphics-/unicode- and font-routines

-- Wiki2Touch     <http://code.google.com/p/wiki2touch/>
                    Thanks to Tom Haukap for the file format and index routines

-- libbzip2       <http://bzip.org/>

-- UCS fonts      <http://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html>
                    Thanks to Markus Kuhn

 + some other fonts

 + Titlegraphic   <http://www.stefanie-gronau.de>
                    Thanks to my beloved wife


 Have Fun!
===========

OlliPolli
