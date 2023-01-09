import { BinFileReader } from "./BinFileReader"
(function()
{

   var _OBJECT_ROOT_ = window;


   // BEGIN Namespace util
   function EnsureNamespace(nsString)
   {

      let nsStrings = nsString.split(".");
      let root = _OBJECT_ROOT_;
      for (let i = 0; i < nsStrings.length; i++)
      {
         let nsName = nsStrings[i];
         let val = root[nsName];
         if (typeof val == "undefined" || !val)
         {
            root[nsName] = new Object(); // {} ?
         }

         root = root[nsName];
      }
   }

   EnsureNamespace("UnScramble.Core");


   // UnScramble.Core.Dictionary
   if (typeof _OBJECT_ROOT_.UnScramble.Core.Dictionary == "undefined" || !_OBJECT_ROOT_.UnScramble.Core["Dictionary"])
_OBJECT_ROOT_.UnScramble.Core.Dictionary = (function()
   {

//    with (UnScramble.Core)
//    {
         var _Dictionary = function(lang)
         {
            if (lang == "French")
            {
               this.IndexedAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";  
               this.MaxWordLength = 28;
               this.DAWGReader = new BinFileReader("datafiles/DAWG_ODS5_French.dat");
               this.DAWGReader.movePointerTo(0);
            } else
            {
               this.IndexedAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";  
               this.MaxWordLength = 28;
               this.DAWGReader = new BinFileReader("datafiles/DAWG_SOWPODS_English.dat");
               this.DAWGReader.movePointerTo(0);
            }
         }


         _Dictionary.prototype.LETTER_BIT_SHIFT = 25;
         _Dictionary.prototype.LETTER_BIT_MASK = 1040187392;
         _Dictionary.prototype.CHILD_INDEX_BIT_MASK = 33554431;
         _Dictionary.prototype.END_OF_WORD_BIT_MASK = 2147483648;
         _Dictionary.prototype.END_OF_LIST_BIT_MASK = 1073741824;

         _Dictionary.prototype.DAWGReader = 0;
         _Dictionary.prototype.IndexedAlphabet = "";
         _Dictionary.prototype.MaxWordLength = 0;

         var DAWG_OFFSET = 4; //4 bytes initial data at top of file, stores the number of nodes in the DAWG

         _Dictionary.prototype.DAWG_Letter = function(dataIndex)
         {
            let byteOffset = 4 * dataIndex + DAWG_OFFSET;
            this.DAWGReader.movePointerTo(byteOffset);

            let alphabetPos = ((this.DAWGReader.readNumber(4) & this.LETTER_BIT_MASK) >> this.LETTER_BIT_SHIFT);
            let letter = this.IndexedAlphabet.substring(alphabetPos, alphabetPos+1);
            return letter;
         }
         _Dictionary.prototype.DAWG_IsEndOfWord = function(dataIndex)
         {
            let byteOffset = 4 * dataIndex + DAWG_OFFSET;
            this.DAWGReader.movePointerTo(byteOffset);

            let val = (this.DAWGReader.readNumber(4) & this.END_OF_WORD_BIT_MASK);
            return val == 0 ? false : true;
         }
_Dictionary.prototype.DAWG_NextIndex = function(dataIndex)
         {
            let byteOffset = 4 * dataIndex + DAWG_OFFSET;
            this.DAWGReader.movePointerTo(byteOffset);

            let val = (this.DAWGReader.readNumber(4) & this.END_OF_LIST_BIT_MASK);
            return val == 0 ? (dataIndex + 1) : 0;
         }
         _Dictionary.prototype.DAWG_ChildIndex = function(dataIndex)
         {
            let byteOffset = 4 * dataIndex + DAWG_OFFSET;
            this.DAWGReader.movePointerTo(byteOffset);

            let val = (this.DAWGReader.readNumber(4) & this.CHILD_INDEX_BIT_MASK);
            return val;
         }

         _Dictionary.prototype.CheckDictionaryRecursive = function(theWord, string_index, dawg_index) //dawg_index is NOT zero-based (start with 1)
         {
            let ch = theWord[string_index];
            let letter = this.DAWG_Letter(dawg_index);
            if (letter == ch)
            {
               if (string_index == (theWord.length - 1))
               {
                  if (this.DAWG_IsEndOfWord(dawg_index))
                     return true;
                  else
                     return false;
               }
               else
               {
                  let childIndex = this.DAWG_ChildIndex(dawg_index);
                  if (childIndex > 0)
                     return this.CheckDictionaryRecursive(theWord, string_index+1, childIndex);
                  else
                     return false;
               }
            }
            else
            {
               let nextIndex = this.DAWG_NextIndex(dawg_index);
               if (nextIndex > 0)
                  return this.CheckDictionaryRecursive(theWord, string_index, nextIndex);
               else
                  return false;
            }
         }

         _Dictionary.prototype.CheckWord = function(theWord)
         {
            theWord = theWord.toUpperCase();
            return this.CheckDictionaryRecursive(theWord, 0, 1); //dawg_index is NOT zero-based (start with 1)
         }

         let currentString = [];
         let sortedChars = [];

         _Dictionary.prototype.FindAnagramsRecursive = function(dawg_index, string_index) //dawg_index is NOT zero-based (start with 1)
         {
            let retArray = [];

            let previousChar = 0;
            let tempIndex = this.DAWG_ChildIndex(dawg_index);
            let string = "";

            currentString[string_index] = this.DAWG_Letter(dawg_index);

            if (this.DAWG_IsEndOfWord(dawg_index))
            {
               for (let i = 0; i <= string_index; i++)
               {
                  string += currentString[i];
               }

               retArray.push(string);
            }

            if ((sortedChars.length > 0) && (tempIndex > 0))
            {
               for (let i = 0; i < sortedChars.length; i++)
               {
                  string += sortedChars[i];
               }

               for (let i = 0; i < sortedChars.length; i++)
               {
                  var currentChar = sortedChars[i];
                  if (currentChar == previousChar) continue;

                  do
                  {
                     let tempLetter = this.DAWG_Letter(tempIndex);
                     if (currentChar == tempLetter)
                     {
                        sortedChars.splice(i, 1);
                        let retAr = this.FindAnagramsRecursive(tempIndex, string_index + 1);
                        retArray = retArray.concat(retAr);
                        sortedChars.splice(i, 0, currentChar);

                        tempIndex = this.DAWG_NextIndex(tempIndex);
                        break;
                     }
                     else
                     {
                        let alphabetPos1 = this.IndexedAlphabet.indexOf(currentChar);
                        let alphabetPos2 = this.IndexedAlphabet.indexOf(tempLetter);

                        if (alphabetPos1 < alphabetPos2) break;
                     }

                     tempIndex = this.DAWG_NextIndex(tempIndex);

                  } while (tempIndex > 0);

                  if (tempIndex <= 0) break;

                  previousChar = currentChar;
               }
            }

            return retArray;
         }

         _Dictionary.prototype.FindWildChar = function(theChars)
         {
             var n = theChars.indexOf("?");
         }


         _Dictionary.prototype.FindAnagrams = function(theChars)
         {
            theChars = theChars.toUpperCase();

            // RESET currentString
            for (let i = 0; i < currentString.length; i++)
            {
               //delete callbacks[i];
               currentString.splice(i--, 1);
            }
            for (let i = 0; i < this.MaxWordLength; i++)
            {
               currentString.push(i + "");
            }

            // RESET sortedChars
            for (let i = 0; i < sortedChars.length; i++)
            {
               //delete callbacks[i];
               sortedChars.splice(i--, 1);
            }
            for (let i = 0; i < theChars.length; i++)
            {
               let aChar = theChars[i];
               if (this.IndexedAlphabet.indexOf(aChar) >= 0)
               {
                  sortedChars.push(aChar);
               }
            }
            sortedChars.sort();

            let string = "";
            for (let i = 0; i < sortedChars.length; i++)
            {
               string += sortedChars[i];
            }

            if (sortedChars.length < 2)
            {
               return [string];
            }

            let anagrams = [];
            let charBank = [];

            for (let i = 0; i < sortedChars.length; i++)
            {
               charBank.push(sortedChars[i]);
            }

            var previousChar = 0;

            for (let i = 0; i < charBank.length; i++)
            {
               let currentChar = charBank[i];

               if (currentChar == previousChar)
               {
                  continue;
               }

               sortedChars.splice(i, 1);

               let dawg_index = this.IndexedAlphabet.indexOf(currentChar) + 1; //dawg_index is NOT zero-based (start with 1)

               let retAr = this.FindAnagramsRecursive(dawg_index, 0, sortedChars.length);
               anagrams = anagrams.concat(retAr);

               sortedChars.splice(i, 0, currentChar);

               previousChar = currentChar;
            }

            return anagrams;
         }

//    } // END - with (UnScramble.Core)

      return _Dictionary;
   })();
   // END UnScramble.Core.Dictionary

})();
// END script-scope



window_onload = function()
{

   //with (UnScramble)
   //{
      // jslint empty block statement

   //}

};

