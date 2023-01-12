/**
 * Base64ToBinReader.js
 *
 * Copyright (c) 2023 John Talbot <ztalbot2000@gmail.com>
 * The source code is freely distributable under the terms of an MIT license.
 *
 *    Advanced browsers no longer let you read a local file. This is against
 *    security policies like ACL and not from Origin.
 *    To get around this we just pass in the blob of binary data.
 *
 *    Now the problem is how to get the blob of binary data.
 *    So we use the newly created tool: tools/scripts/DAWGDatToData64JSVar.js
 *    which reads the binary DAWG, transposes it to base64 and then
 *    creates a javascript file with:  var base64Dict="with the base64 data";
 *    This javascript file gets included like:
 *    <SCRIPT TYPE="text/javascript" SRC="datafiles/DAWG_SOWPODS_English_base64.js"></SCRIPT>
 *    Then base64Dict gets passed to this function
 */

var Buffer = require('buffer/').Buffer;

export function Base64ToBinReader( binData64 )
{
   var filePointer = 0;
   var fileSize = -1;
   var fileContents;

   this.getFileSize = function()
   {
      return fileSize;
   }

   this.movePointerTo = function( iTo )
   {
      if ( iTo < 0 )
         filePointer = 0;
      else if ( iTo > this.getFileSize( ) )
         throw new Error("Errpr: EOF reached. iTo: " + iTo + " fileSize: " + this.getFileSize( ) );
      else
         filePointer = iTo;

      return filePointer;
   };

   this.readNumber = function( iNumBytes, iFrom )
   {
      iNumBytes = iNumBytes || 1;
      iFrom = iFrom || filePointer;

      this.movePointerTo( iFrom + iNumBytes );

      let result = 0;
      for( let i=iFrom + iNumBytes; i>iFrom; i-- )
      {
         result = result * 256 + this.readByteAt( i-1 );
      }

      return result;
   };

   this.readString = function( iNumChars, iFrom )
   {
      iNumChars = iNumChars || 1;
      iFrom = iFrom || filePointer;

      this.movePointerTo( iFrom );

      let result = "";
      let tmpTo = iFrom + iNumChars;
      for( let i=iFrom; i<tmpTo; i++ )
      {
         result += String.fromCharCode( this.readNumber( 1 ) );
      }

      return result;
   };

   this.readUnicodeString = function( iNumChars, iFrom )
   {
      iNumChars = iNumChars || 1;
      iFrom = iFrom || filePointer;

      this.movePointerTo(iFrom);

      let result = "";
      let tmpTo = iFrom + iNumChars*2;
      for( let i=iFrom; i<tmpTo; i+=2 )
      {
         result += String.fromCharCode( this.readNumber( 2 ) );
      }

      return result;
   };

   function Base64ToBinReaderImpl( binData64 )
   {
      fileContents = binData64;

      fileSize = fileContents.length;

      this.readByteAt = function( i )
      {
         return fileContents.charCodeAt(i) & 0xff;
      }
   }

   // Convert the base64 encoded dictionary to a binary array
   let binData = Buffer.from(base64Dict, 'base64').toString("binary");

   Base64ToBinReaderImpl.apply(this, [ binData ]);
}
