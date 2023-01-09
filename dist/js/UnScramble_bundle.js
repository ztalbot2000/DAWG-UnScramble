/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "./src/js/BinFileReader.js":
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

__webpack_require__.r(__webpack_exports__);
/* harmony export */ __webpack_require__.d(__webpack_exports__, {
/* harmony export */   "BinFileReader": () => (/* binding */ BinFileReader)
/* harmony export */ });
/**
 * BinFileReader.js
 * You can find more about this function at
 * http://nagoon97.com/reading-binary-files-using-ajax/
 *
 * Copyright (c) 2008 Andy G.P. Na <nagoon97@naver.com>
 * The source code is freely distributable under the terms of an MIT-style license. 
 */

function BinFileReader(fileURL) {
  var _exception = {};
  _exception.FileLoadFailed = 1;
  _exception.EOFReached = 2;
  var filePointer = 0;
  var fileSize = -1;
  var fileContents;
  this.getFileSize = function () {
    return fileSize;
  };
  this.getFilePointer = function () {
    return filePointer;
  };
  this.movePointerTo = function (iTo) {
    if (iTo < 0) filePointer = 0;else if (iTo > this.getFileSize()) throwException(_exception.EOFReached, "Failed to load file. Check the logs. EOF reached. iTo: " + iTo + " fileSize: " + this.getFileSize());else filePointer = iTo;
    return filePointer;
  };
  this.movePointer = function (iDirection) {
    this.movePointerTo(filePointer + iDirection);
    return filePointer;
  };
  this.readNumber = function (iNumBytes, iFrom) {
    iNumBytes = iNumBytes || 1;
    iFrom = iFrom || filePointer;
    this.movePointerTo(iFrom + iNumBytes);
    let result = 0;
    for (let i = iFrom + iNumBytes; i > iFrom; i--) {
      result = result * 256 + this.readByteAt(i - 1);
    }
    return result;
  };
  this.readString = function (iNumChars, iFrom) {
    iNumChars = iNumChars || 1;
    iFrom = iFrom || filePointer;
    this.movePointerTo(iFrom);
    let result = "";
    let tmpTo = iFrom + iNumChars;
    for (let i = iFrom; i < tmpTo; i++) {
      result += String.fromCharCode(this.readNumber(1));
    }
    return result;
  };
  this.readUnicodeString = function (iNumChars, iFrom) {
    iNumChars = iNumChars || 1;
    iFrom = iFrom || filePointer;
    this.movePointerTo(iFrom);
    let result = "";
    let tmpTo = iFrom + iNumChars * 2;
    for (let i = iFrom; i < tmpTo; i += 2) {
      result += String.fromCharCode(this.readNumber(2));
    }
    return result;
  };
  function throwException(errorCode, reason) {
    switch (errorCode) {
      case _exception.FileLoadFailed:
        alert(reason);
        throw new Error('Error: Failed to load');
      case _exception.EOFReached:
        alert(reason);
        throw new Error("Error: EOF reached");
    }
  }
  function BinFileReaderImpl_IE(fileURL) {
    var vbArr = BinFileReaderImpl_IE_VBAjaxLoader(fileURL);
    fileContents = vbArr.toArray();
    fileSize = fileContents.length - 1;
    if (fileSize < 0) throwException(_exception.FileLoadFailed, "Failed to load: " + fileURL + " Check the logs. file size: " + fileSize);
    this.readByteAt = function (i) {
      return fileContents[i];
    };
  }
  function BinFileReaderImpl(fileURL) {
    var req = new XMLHttpRequest();

    // asynchronous = false
    req.open('GET', fileURL, false);

    //XHR binary charset opt by Marcus Granado 2006 [http://mgran.blogspot.com] 
    req.overrideMimeType('text/plain; charset=x-user-defined');
    req.send(null);
    if (req.status != 200) {
      throwException(_exception.FileLoadFailed, "Failed to load: " + fileURL + " Check the logs. Status: " + req.status);
    }
    fileContents = req.responseText;
    fileSize = fileContents.length;
    this.readByteAt = function (i) {
      return fileContents.charCodeAt(i) & 0xff;
    };
  }
  if (/msie/i.test(navigator.userAgent) && !/opera/i.test(navigator.userAgent)) BinFileReaderImpl_IE.apply(this, [fileURL]);else BinFileReaderImpl.apply(this, [fileURL]);
}
document.write('<script type="text/vbscript">\n\
Function BinFileReaderImpl_IE_VBAjaxLoader( fileName )\n\
   Dim xhr\n\
   Set xhr = CreateObject("Microsoft.XMLHTTP")\n\
\n\
   xhr.Open "GET", fileName, False\n\
\n\
   xhr.setRequestHeader "Accept-Charset", "x-user-defined"\n\
   xhr.send\n\
\n\
   Dim byteArray()\n\
\n\
   if xhr.Status = 200 Then\n\
      Dim byteString\n\
      Dim i\n\
\n\
      byteString=xhr.responseBody\n\
\n\
      ReDim byteArray(LenB(byteString))\n\
\n\
      For i = 1 To LenB(byteString)\n\
         byteArray(i-1) = AscB(MidB(byteString, i, 1))\n\
      Next\n\
   End If\n\
\n\
   BinFileReaderImpl_IE_VBAjaxLoader=byteArray\n\
End Function\n\
</script>');

/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		// Execute the module function
/******/ 		__webpack_modules__[moduleId](module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/************************************************************************/
/******/ 	/* webpack/runtime/define property getters */
/******/ 	(() => {
/******/ 		// define getter functions for harmony exports
/******/ 		__webpack_require__.d = (exports, definition) => {
/******/ 			for(var key in definition) {
/******/ 				if(__webpack_require__.o(definition, key) && !__webpack_require__.o(exports, key)) {
/******/ 					Object.defineProperty(exports, key, { enumerable: true, get: definition[key] });
/******/ 				}
/******/ 			}
/******/ 		};
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/hasOwnProperty shorthand */
/******/ 	(() => {
/******/ 		__webpack_require__.o = (obj, prop) => (Object.prototype.hasOwnProperty.call(obj, prop))
/******/ 	})();
/******/ 	
/******/ 	/* webpack/runtime/make namespace object */
/******/ 	(() => {
/******/ 		// define __esModule on exports
/******/ 		__webpack_require__.r = (exports) => {
/******/ 			if(typeof Symbol !== 'undefined' && Symbol.toStringTag) {
/******/ 				Object.defineProperty(exports, Symbol.toStringTag, { value: 'Module' });
/******/ 			}
/******/ 			Object.defineProperty(exports, '__esModule', { value: true });
/******/ 		};
/******/ 	})();
/******/ 	
/************************************************************************/
var __webpack_exports__ = {};
// This entry need to be wrapped in an IIFE because it need to be isolated against other modules in the chunk.
(() => {
__webpack_require__.r(__webpack_exports__);
/* harmony import */ var _BinFileReader__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__("./src/js/BinFileReader.js");

(function () {
  var _OBJECT_ROOT_ = window;

  // BEGIN Namespace util
  function EnsureNamespace(nsString) {
    let nsStrings = nsString.split(".");
    let root = _OBJECT_ROOT_;
    for (let i = 0; i < nsStrings.length; i++) {
      let nsName = nsStrings[i];
      let val = root[nsName];
      if (typeof val == "undefined" || !val) {
        root[nsName] = new Object(); // {} ?
      }

      root = root[nsName];
    }
  }
  EnsureNamespace("UnScramble.Core");

  // UnScramble.Core.Dictionary
  if (typeof _OBJECT_ROOT_.UnScramble.Core.Dictionary == "undefined" || !_OBJECT_ROOT_.UnScramble.Core["Dictionary"]) _OBJECT_ROOT_.UnScramble.Core.Dictionary = function () {
    //    with (UnScramble.Core)
    //    {
    var _Dictionary = function (lang) {
      if (lang == "French") {
        this.IndexedAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        this.MaxWordLength = 28;
        this.DAWGReader = new _BinFileReader__WEBPACK_IMPORTED_MODULE_0__.BinFileReader("datafiles/DAWG_ODS5_French.dat");
        this.DAWGReader.movePointerTo(0);
      } else {
        this.IndexedAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        this.MaxWordLength = 28;
        this.DAWGReader = new _BinFileReader__WEBPACK_IMPORTED_MODULE_0__.BinFileReader("datafiles/DAWG_SOWPODS_English.dat");
        this.DAWGReader.movePointerTo(0);
      }
    };
    _Dictionary.prototype.LETTER_BIT_SHIFT = 25;
    _Dictionary.prototype.LETTER_BIT_MASK = 1040187392;
    _Dictionary.prototype.CHILD_INDEX_BIT_MASK = 33554431;
    _Dictionary.prototype.END_OF_WORD_BIT_MASK = 2147483648;
    _Dictionary.prototype.END_OF_LIST_BIT_MASK = 1073741824;
    _Dictionary.prototype.DAWGReader = 0;
    _Dictionary.prototype.IndexedAlphabet = "";
    _Dictionary.prototype.MaxWordLength = 0;
    var DAWG_OFFSET = 4; //4 bytes initial data at top of file, stores the number of nodes in the DAWG

    _Dictionary.prototype.DAWG_Letter = function (dataIndex) {
      let byteOffset = 4 * dataIndex + DAWG_OFFSET;
      this.DAWGReader.movePointerTo(byteOffset);
      let alphabetPos = (this.DAWGReader.readNumber(4) & this.LETTER_BIT_MASK) >> this.LETTER_BIT_SHIFT;
      let letter = this.IndexedAlphabet.substring(alphabetPos, alphabetPos + 1);
      return letter;
    };
    _Dictionary.prototype.DAWG_IsEndOfWord = function (dataIndex) {
      let byteOffset = 4 * dataIndex + DAWG_OFFSET;
      this.DAWGReader.movePointerTo(byteOffset);
      let val = this.DAWGReader.readNumber(4) & this.END_OF_WORD_BIT_MASK;
      return val == 0 ? false : true;
    };
    _Dictionary.prototype.DAWG_NextIndex = function (dataIndex) {
      let byteOffset = 4 * dataIndex + DAWG_OFFSET;
      this.DAWGReader.movePointerTo(byteOffset);
      let val = this.DAWGReader.readNumber(4) & this.END_OF_LIST_BIT_MASK;
      return val == 0 ? dataIndex + 1 : 0;
    };
    _Dictionary.prototype.DAWG_ChildIndex = function (dataIndex) {
      let byteOffset = 4 * dataIndex + DAWG_OFFSET;
      this.DAWGReader.movePointerTo(byteOffset);
      let val = this.DAWGReader.readNumber(4) & this.CHILD_INDEX_BIT_MASK;
      return val;
    };
    _Dictionary.prototype.CheckDictionaryRecursive = function (theWord, string_index, dawg_index)
    //dawg_index is NOT zero-based (start with 1)
    {
      let ch = theWord[string_index];
      let letter = this.DAWG_Letter(dawg_index);
      if (letter == ch) {
        if (string_index == theWord.length - 1) {
          if (this.DAWG_IsEndOfWord(dawg_index)) return true;else return false;
        } else {
          let childIndex = this.DAWG_ChildIndex(dawg_index);
          if (childIndex > 0) return this.CheckDictionaryRecursive(theWord, string_index + 1, childIndex);else return false;
        }
      } else {
        let nextIndex = this.DAWG_NextIndex(dawg_index);
        if (nextIndex > 0) return this.CheckDictionaryRecursive(theWord, string_index, nextIndex);else return false;
      }
    };
    _Dictionary.prototype.CheckWord = function (theWord) {
      theWord = theWord.toUpperCase();
      return this.CheckDictionaryRecursive(theWord, 0, 1); //dawg_index is NOT zero-based (start with 1)
    };

    let currentString = [];
    let sortedChars = [];
    _Dictionary.prototype.FindAnagramsRecursive = function (dawg_index, string_index)
    //dawg_index is NOT zero-based (start with 1)
    {
      let retArray = [];
      let previousChar = 0;
      let tempIndex = this.DAWG_ChildIndex(dawg_index);
      let string = "";
      currentString[string_index] = this.DAWG_Letter(dawg_index);
      if (this.DAWG_IsEndOfWord(dawg_index)) {
        for (let i = 0; i <= string_index; i++) {
          string += currentString[i];
        }
        retArray.push(string);
      }
      if (sortedChars.length > 0 && tempIndex > 0) {
        for (let i = 0; i < sortedChars.length; i++) {
          string += sortedChars[i];
        }
        for (let i = 0; i < sortedChars.length; i++) {
          var currentChar = sortedChars[i];
          if (currentChar == previousChar) continue;
          do {
            let tempLetter = this.DAWG_Letter(tempIndex);
            if (currentChar == tempLetter) {
              sortedChars.splice(i, 1);
              let retAr = this.FindAnagramsRecursive(tempIndex, string_index + 1);
              retArray = retArray.concat(retAr);
              sortedChars.splice(i, 0, currentChar);
              tempIndex = this.DAWG_NextIndex(tempIndex);
              break;
            } else {
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
    };
    _Dictionary.prototype.FindWildChar = function (theChars) {
      var n = theChars.indexOf("?");
    };
    _Dictionary.prototype.FindAnagrams = function (theChars) {
      theChars = theChars.toUpperCase();

      // RESET currentString
      for (let i = 0; i < currentString.length; i++) {
        //delete callbacks[i];
        currentString.splice(i--, 1);
      }
      for (let i = 0; i < this.MaxWordLength; i++) {
        currentString.push(i + "");
      }

      // RESET sortedChars
      for (let i = 0; i < sortedChars.length; i++) {
        //delete callbacks[i];
        sortedChars.splice(i--, 1);
      }
      for (let i = 0; i < theChars.length; i++) {
        let aChar = theChars[i];
        if (this.IndexedAlphabet.indexOf(aChar) >= 0) {
          sortedChars.push(aChar);
        }
      }
      sortedChars.sort();
      let string = "";
      for (let i = 0; i < sortedChars.length; i++) {
        string += sortedChars[i];
      }
      if (sortedChars.length < 2) {
        return [string];
      }
      let anagrams = [];
      let charBank = [];
      for (let i = 0; i < sortedChars.length; i++) {
        charBank.push(sortedChars[i]);
      }
      var previousChar = 0;
      for (let i = 0; i < charBank.length; i++) {
        let currentChar = charBank[i];
        if (currentChar == previousChar) {
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
    };

    //    } // END - with (UnScramble.Core)

    return _Dictionary;
  }();
  // END UnScramble.Core.Dictionary
})();
// END script-scope

window_onload = function () {

  //with (UnScramble)
  //{
  // jslint empty block statement

  //}
};
})();

/******/ })()
;
//# sourceMappingURL=UnScramble_bundle.js.map