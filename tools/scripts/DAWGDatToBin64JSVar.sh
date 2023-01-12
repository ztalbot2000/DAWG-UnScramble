#!/bin/bash

#  Author: John Talbot
#
#  Script to convert binary file into javascript file with a variable
#  containing the binary data converted to base64
#
#  Same as DAWGDatToBin64JSVar.js but as a shell.
#  This file is not used.

# Exit immediately if a command exits with a non-zero status
# set -e

# Exit immediately for unbound variables.
# set -u

# a global return code value for those who return one
rc='0'

# Fun colour & cursor stuff.

TNRM=$(tput sgr0)
TRED=$(tput setaf 1)
TYEL=$(tput setaf 3)

function showHelp()
{
cat <<'HELP_EOF'
   This shell script is a front end to crosstool-ng to help build a cross compiler on your Mac.  It downloads all the necessary files to build the cross compiler.  It only assumes you have Xcode command line tools installed.

   Options:
     -i <Input File>     - Binary input file name
     -o <Outpur File>    - Resultant Base64 javascript file
     -v <Variable name>  - The variable in thenoutput file holding
                           the base64 date

   i.e binaryFileToData64JSVar.sh -i inputBinaryFile -o base64Var.js -v myVar
     where:
       inputBinaryFile is a file containg binary data
          - could be an image file or in this case DOWSOWPODS_English.dat
       base64Var.js is the resultant file
       myVar is the variable name within the resultant file. The contents
          of the resultant file would look something like:
             myVar="AA01BcAA.....";


HELP_EOF
}

DEFAULT_inputFile="/tmp/DAWG_SOWPODS_English.dat"
DEFAULT_outputFile="dist/datafiles/DAWG_SOWPODS_English_base64.js"
DEFAULT_variableName="base64Dict"

inputFile=DEFAULT_inputFile
outputFile=DEFAULT_outputFile
variableName="${DEFAULT_variableName}"

# Define this once and you save yourself some trouble
OPTSTRING='h?i:o:v:'

while getopts "${OPTSTRING}" opt; do
   case ${opt} in
      h)
          showHelp
          exit 0
          ;;
      i)
         inputFile="${OPTARG}"
      ;;
      o)
         outputFile="${OPTARG}"
         ;;
      v)
         variableName="${OPTARG}"
         ;;
      \?)
         # Done automatically by getopts
         # echo "${TRED}ERROR:${TNRM} Unknown option ${opt}"
         exit 1
         ;;
      :)
         # Done automatically by getopts
         #echo "${TRED}Option ${TNRM}-${opt} requires an argument."
         exit 1
         ;;
   esac
done

if [ ! -f "${inputFile}" ]; then
   echo "${TRED}ERROR:${TNRM} Input file: ${inputFile} does not exist"
   exit 1
fi

if [ -f "${outputFile}" ]; then
   echo "${TYEL}WARNING:${TNRM} File already exists: ${outputFile}"
   echo "         and will be overwritten in 3 seconds"

   # Give a couple of seconds for the user to react
   sleep 3
fi

# what we now want to do is this:
# (echo "var image64 = \"\\"; \
#  base64 $< | sed -e 's/$$/\\/'; \
#/ echo '";') > $@

 (echo "var ${variableName} = \"\\"; \
  base64 "${inputFile}" | sed -e 's/$$/\\/'; \
  echo '";') > "${outputFile}"

# Set our global return code of the process
rc=$?

if [ "${rc}" != '0' ]; then
       echo "${TRED}ERROR : [${rc}] ${TNRM} convert failed"
       exit "${rc}"
fi
exit "${rc}"

