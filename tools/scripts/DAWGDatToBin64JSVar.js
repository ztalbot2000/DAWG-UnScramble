#!node

//  Author: John Talbot

//  Script to convert binary file into javascript file with a variable
//  containing the binary data converted to base64

//  Same as DAWGDatToBin64JSVar.sh but as a javascript file.
//  As UnScramble.js does the opposite conversion, This javascript tool seemed
//  a better thing to do.

const fs = require('fs');
let DEFAULT_inputFile = "/tmp/DAWG_SOWPODS_English.dat";
let DEFAULT_outputFile = "dist/datafiles/DAWG_SOWPODS_English_base64.js";
let DEFAULT_variableName = "base64Dict";

let inputFile = DEFAULT_inputFile;
let outputFile = DEFAULT_outputFile;
let variableName = DEFAULT_variableName;

// Fun colour & cursor stuff.
const TRED = "\x1B[31m";
const TNRM = "\x1B[0m";
const TYEL = "\x1B[43m";

function showHelp()
{
   console.log( "This shell script is a front end to crosstool-ng to help build a cross compiler on your Mac.  It downloads all the necessary files to build the cross compiler.  It only assumes you have Xcode command line tools installed.\n" +
"\n" +
"   Options:\n" +
"    -i <Input File>     - Binary input file name\n" +
"    -o <Outpur File>    - Resultant Base64 javascript file\n" +
"    -v <Variable name>  - The variable in thenoutput file holding\n" +
"                          the base64 date\n" +
"\n" +
"  i.e binaryFileToData64JSVar.sh -i inputBinaryFile -o base64Var.js -v myVar\n" +
"    where:\n" +
"      inputBinaryFile is a file containg binary data\n" +
"         - could be an image file or in this case DOWSOWPODS_English.dat\n" +
"      base64Var.js is the resultant file\n" +
"      myVar is the variable name within the resultant file. The contents\n" +
"         of the resultant file would look something like:\n" +
"            myVar=\"AA01BcAA.....\";\n"  );

}

process.argv.forEach(function (val, index, array)
{
   if ( val[0] != "-" )
      return;

   switch (val)
   {
      case "-h":
         showHelp();
         process.exit(1);
         break;
      case "-i":
         if ( index >= array.length -1 )
         {
            console.log( TRED + "ERROR:" + TNRM + " No input file name given" );
            process.exit(1);
         }
         inputFile = array[ index + 1 ];
         break;
      case "-o":
         if ( index >= array.length -1 )
         {
            console.log( TRED + "ERROR:" + TNRM + " No output file name given" );
            process.exit(1);
         }
         outputFile = array[ index + 1 ];
         break;
      case "-v":
         if ( index >= array.length -1 )
         {
            console.log( TRED + "ERROR:" + TNRM + " No variable name given" );
            process.exit(1);
         }
         variableName = val;
         break;
      default:
         console.log("Error" + TNRM +": unknown option: " + val );
         process.exit(1);
   }
});

if (! fs.existsSync(inputFile))
{
   console.log( TRED + "ERROR:" + TNRM + " Input file: " + inputFile + " does not exist");
   process.exit(1);
}

if (fs.existsSync(outputFile))
{
   console.log( TYEL + "WARNING:" + TNRM + "File already exists: " + outputFile );
   console.log( "         and will be overwritten in 3 seconds" );
   // Cheep and dirty sleep
   function delay(ms) {
      const date = Date.now();
      let currentDate = null;
 
      do {
          currentDate = Date.now();
      } while (currentDate - date < ms);
   }
 
   delay(3000);
}

function createBase64( inputFile, outputFile, variableName )
{
   console.log( "Reading: " + inputFile );

   let buff = fs.readFileSync( inputFile );
   let base64Data = buff.toString( 'base64' );

   console.log( "Creating: " + outputFile );
   try{
      fs.writeFileSync(outputFile, "var " + variableName + "=\"");
      fs.appendFileSync(outputFile, base64Data );
      fs.appendFileSync(outputFile, "\";\n");
      // const rawData = 'Hello World';
      // const data = Buffer.from(rawData);
      // fs.writeFileSync('index.txt', data, { flag: 'ax' });
   } catch (e) {
      console.log(e); // will log any error
      process.exit(1);
   }
}

createBase64( inputFile, outputFile, variableName );

// what we now want to do is this:
// (echo "var image64 = \"\\"; \
//  base64 $< | sed -e 's/$$/\\/'; \
/// echo '";') > $@

process.exit(0);

