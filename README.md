# UnScramble - Direct Acrylic Word Graph & UnScramble JavaScript Web Page.



About the Tool Set
---------------------
DAWG-UnScramble is a complete tool set with a speedy 'C' word list pre-processor and an HTML/JavaScript front end.  The main emphasis is on the pre-processor, it is not only capable of creating a DAWG, but also reversing the DAWG back into the original word list.

The HTML/Javascript front end has only been tested on Safari and a working copy can be found at <a href="http://BeNiceGames.com/Tools/UnScramble/Unscramble.html">Unscramble (No Ads) </a>



How to use the DAWG Pre-Processor
---------------------------------
* gcc UnScramble.c -o UnScramble
* ./UnScramble -h
* ./Unscramble -i Lexicon.txt -o DAWG_SOWPODS.English.dat

Installation
------------
The files should be placed on your Web server in the following configuration.
<BR>
javascript/Bin_FileReader.js<BR>
javascript/jquery-1.12.1.min.js<BR>
javascript/UnScramble.js<BR>
DAWG_SOWPODS.English.dat<BR>
UnScramble.html<BR>
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



