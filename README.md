# UnScramble - Direct Acrylic Word Graph & UnScramble JavaScript Web Page.



About the Tool Set
---------------------
DAWG-UnScramble is a complete tool set with a speedy 'C' word list pre-processor and an HTML/JavaScript front end.  The main emphasis is on the pre-processor, it is not only capable of creating a DAWG, but also reversing the DAWG back into the original word list.

The HTML/Javascript front end has only been tested on Safari and a working copy can be found at <a href="http://www.benicegames.com/Tools/UnScramble/UnScramble.html">Unscramble (No Ads) </a>

Tag V1 Bug versus V2+
----------------------
Since version 2, UnScramble uses npm & webpack to install dependancies required for handling modern web browser ACL permisions and/or Origin requests when trying to read in its binary dictionary file. UnScramble version 1 failed to load the binary dictiinary file and when searching for words would do nothhing. The "Must Contain" selection button would just show "Blast".

Development
-----------
 UnScramble uses npm & webpack to install dependancies. Install dependancies with:

* npm install --save-dev

How to use the DAWG Pre-Processor
---------------------------------
* mkdir -p bin
* gcc src/c/UnScramble.c -o bin/UnScramble
* ./bin/UnScramble -h
* ./bin/Unscramble -i sr/datafiles/Lexicon.txt -o /tmp/DAWG_SOWPODS.English.dat

for more descriptive logs use option -d <0-3>
* ./bin/Unscramble -d 1 -i sr/datafiles/Lexicon.txt -o /tmp/DAWG_SOWPODS.English.dat

or

* npm run cbuild

Modern Web Browsers as of Jan 2023
----------------------------------
UnScramble.html used to read in the dictionary directly via
XMLHttpRequest of ./datafiles/DAWG_SOWPODS_English.dat. See files under tag v1.0<BR>
With modern browsers this is an ACL and/or Origin security risk. To get around this the resultant DAWG is bin64 encoded and then decoded back into its binary form. The final resultant file is included in UnScramble.html with:

  <SCRIPT  SRC="datafiles/DAWG_SOWPODS_English_base64.js"></SCRIPT>

How to use the DAWG Data to base64 Pre-Processor
-------------------------------------------------
As mentioned modern web browsers cannot read local binary files. It first must be
converted from binary to base64 encoding with:.

* node tools/scripts/DAWGDatToBin64JSVar.js -h
* node tools/scripts/DAWGDatToBin64JSVar.js -v base64Dict -i /tmp/DAWG_SOWPODS_English.dat -o /tmp/DAWG_SOWPODS_English_base64.js

or

* npm run build:dat64_E

Local Testing
-------------
UnScramble uses webpack-dev-server. To try UnScramble locally run:

* npm run devServer

Real Web Server Installation
-----------------------------
The resultant files should be placed on your Web server in the following configuration.
<BR>
UnScramble.html<BR>
js/UnScramble.js<BR>
datafiles/DAWG_SOWPODS_English_base64.js<BR>
images/Letter-30x20.png<BR>
images/background.png<BR>
<BR>

## Screenshots


<h3 align="center">
  <img src="https://github.com/ztalbot2000/DAWG-UnScramble/raw/master/screenshots/UnScramble.jpg">
</h3>

Inspiration
-----------
- I sharpenned my teeth here, but I needed more.
* [scrabble-html-ui]https://github.com/danielweck/scrabble-html-ui<BR>


License
-------

See [LICENSE](LICENSE)



