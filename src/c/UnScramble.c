/*
   UnScramble.c
   UnScramble

   Created by zarf on 2017-12-15.
   The source code is freely distributable under the terms of an MIT license.

   This program is a lexicon data structure known as a "Directed Acyclic Word Graph",
   more commonly used as a quick way to UnScramble Letters.

   A lot of effort was made to not only create a DAWG from a list of words, but to
   understand how to reverse the process and recreate the list from the DAWG itself.

   To set up the DAWG, all that is required is a sorted list of words.  If the
   maximum word length exceeds 50 characters, please change INPUT_LIMIT. That is all.

   The unsigned integer encoding that we are using for this "DAWG" allows for up to
   33554432 nodes.  That is more than most languages have, including English.

*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef CLANG_ANALYZER_NORETURN
#if __has_feature(attribute_analyzer_noreturn)
#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#else
#define CLANG_ANALYZER_NORETURN
#endif
#endif

// C requires a boolean variable type so use C's typedef concept to create one.
typedef enum { FALSE = 0, TRUE = 1 } Bool;

// Semi Constant (set once)
unsigned int MAX_WORD_LENGTH = 1;
unsigned int NUMBER_OF_LINES_IN_DICTIONARY = 0;

typedef enum
{
   DEBUG0 = 0,     // NONE
   DEBUG1 = 1,     // BASIC
   DEBUG2 = 2,     // DESCRIPTIVE
   DEBUG3 = 3      // EXTREME
} DebugLevelType;



DebugLevelType debugLevel = DEBUG1; // Default
DebugLevelType lineDebugLevel;




// By using two globals, you reudce the number of analyze errors during debug.
#define DEBUGLOG(level, fmt)  do {lineDebugLevel = level; build_log fmt;} while(0)

void build_log(const char *fmt, ...)
{
   if (lineDebugLevel > debugLevel)
      return;
   va_list args;
   va_start(args, fmt);
   vfprintf(stdout, fmt, args);
   va_end(args);
}

// For DEBUG function to be compiled out.
// #define DEBUGLOG(...) /**/


#define MIN_WORD_LENGTH 2

// Assume the maximum word length when reading a dictionary word
#define INPUT_LIMIT 50

#define LETTER_BIT_SHIFT 25
#define LETTER_BIT_MASK 0x3E000000
#define   CHILD_INDEX_BIT_MASK 0x1FFFFFF
#define END_OF_WORD_BIT_MASK 0x80000000
#define END_OF_LIST_BIT_MASK 0x40000000
#define BINARY_STRING_LENGTH 38


// When reading strings from a file, the new-line character is appended, and this macro will remove it before processing.
#define CUT_OFF_NEW_LINE(string) (string[strlen(string) - 1] = '\0')



// This array streamlines the conversion of digit strings into integers.  Unsigned integers do not exceed the low billions, so ten numbers will suffice.

unsigned int PowersOfTwo[32] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648};


// This Function converts any lower case letters in the string "RawWord", into capitals, so that the whole string is made of capital letters.
void MakeMeAllCapital(char *RawWord)
{
   for (int X = 0; X < strlen(RawWord); X++ )
   {
      if (islower(RawWord[X]))
         RawWord[X] = toupper(RawWord[X]);
   }
}

// Returns the unsigned long rerpresented by "numberString" string, and if not a number greater then zero, returns 0.
//#define INT_MAX 2147483647
#import <limits.h>
unsigned int stringToUnsignedInt(char *numberString)
{
   if (strspn(numberString, "0123456789") == strlen(numberString))
   {
      unsigned long value = strtoul(numberString, NULL, 10);

      if (value > INT_MAX)
         return 0;

      //  We only care about the precision of an unsigned int; Checked above.
      return (unsigned int) value;
   }

   return 0;
}


////////////////////////////////////////////////////////////////////////////////
// "Tnode" functionality.  These will be the nodes contained in a "Trie" designed
// to convert into a "Dawg".

#pragma mark - tnode_struct

typedef struct tnode_struct
{
   struct tnode_struct* Next;
   struct tnode_struct* Child;
   struct tnode_struct* Parent;
   // When populating the array, you must know the indices of child nodes.  Hence
   // we Store "ArrayIndex" in every node so that we can mine the information from
   // the reduced "Trie".
   int ArrayIndex;
   Bool IsaDirectChild;
   char Letter;
   size_t MaxChildDepth;
   int Level;
   int NumberOfChildren;
   Bool Dangling;
   char EndOfWordFlag;

   // The UniqueID is used for nothing but debugging.
   // It is easier to tell what node your working with if
   // each node has a different identifier.
   int UniqueID;

} Tnode, *TnodePtr;

// Extracting "Tnode" member data will consume a lot of time when finding redundant
// nodes, so define these functions as macros.
// The only negative effect is that the programmer must be diligent to use these
// constructs on the right data.  Otherwise, this decision makes the program faster.
#define TNODE_ARRAY_INDEX(thistnode) (thistnode->ArrayIndex)
#define TNODE_ISADIRECT_CHILD(thistnode) (thistnode->IsaDirectChild)
#define TNODE_NEXT(thistnode) (thistnode->Next)
#define TNODE_CHILD(thistnode) (thistnode->Child)
#define TNODE_PARENTAL_UNIT(thistnode) (thistnode->Parent)
#define TNODE_LETTER(thistnode) (thistnode->Letter)
#define TNODE_MAX_CHILD_DEPTH(thistnode) (thistnode->MaxChildDepth)
#define TNODE_NUMBER_OF_CHILDREN(thistnode) (thistnode->NumberOfChildren)
#define TNODE_END_OF_WORD_FLAG(thistnode) (thistnode->EndOfWordFlag)
#define TNODE_LEVEL(thistnode) (thistnode->Level)
#define TNODE_DANGLING(thistnode) (thistnode->Dangling)

// The UniqueID is used for nothing but debugging. It is easier to tell what node
// your working with if each node has a different identifier.

#define TNODE_UNIQUEID(thistnode) (thistnode->UniqueID)

void printTnode (TnodePtr node)
{
   if (node == NULL)
      return;

   DEBUGLOG(DEBUG2,("%c-%d ", TNODE_LETTER(node), TNODE_UNIQUEID(node)));

   if (TNODE_DANGLING(node) == TRUE)
      DEBUGLOG(DEBUG2,("Dangling"));

   if ( TNODE_CHILD(node) != NULL )
      DEBUGLOG(DEBUG2,("[%d]",TNODE_UNIQUEID(TNODE_CHILD(node))));

   if ( TNODE_NEXT(node) != NULL )
      DEBUGLOG(DEBUG2,("{%d}",TNODE_UNIQUEID(TNODE_NEXT(node))));

   if (TNODE_END_OF_WORD_FLAG(node) == TRUE)
      DEBUGLOG(DEBUG2,("*"));

   if ( TNODE_CHILD(node) != NULL )
   {
      DEBUGLOG(DEBUG2,("^"));
      printTnode(TNODE_CHILD(node)) ;
   }
   if ( TNODE_NEXT(node) != NULL )
   {
      DEBUGLOG(DEBUG2,("\n   Next[%d]->%c", TNODE_UNIQUEID(TNODE_NEXT(node)),TNODE_LETTER(TNODE_NEXT(node))));

   }
}
void printTnode_recursive (TnodePtr root)
{
   if (root == NULL)
      return;

   printTnode(root);
   if ( TNODE_NEXT(root) != NULL )
   {
      printTnode_recursive(TNODE_NEXT(root));
   }

}

void printHolderOfAllTNodes(TnodePtr** holder)
{
   for (unsigned int X = 0; X < MAX_WORD_LENGTH; X++ )
   {
      DEBUGLOG(DEBUG2,("Holder[%d]=",X));
      TnodePtr current = *holder[X];
      if (current == NULL)
      {
         DEBUGLOG(DEBUG2,("NULL\n"));
      }else
      {
         printTnode_recursive(current);
      }

      DEBUGLOG(DEBUG2,("\n\n"));

   }
}

void printDictionary(char *dictionary[], int *WordsOfLength)
{
   for (unsigned int X = 0; X <= MAX_WORD_LENGTH; X++ )
   {
      DEBUGLOG(DEBUG2,("Dictionary [%4d]-> %4d words = {", X, WordsOfLength[X]));
      for ( int i = 1; i <=WordsOfLength[X]; i++ )
         DEBUGLOG(DEBUG2,("'%s'", dictionary[X]+(X+1)*(i-1)));

      DEBUGLOG(DEBUG2,("}\n"));
   }
}
void _do_graphviz_from_dawg(TnodePtr node, FILE * out)
{
   if (TNODE_DANGLING(node))
   {
      return;
   }
   node->Dangling = 1;

   if (TNODE_UNIQUEID(node) == 0)
   {
      fprintf(out, "\tn%d [label=\"root\", shape=Mdiamond];\n", TNODE_UNIQUEID(node));
   } else
   {
      fprintf(out, "\tn%d [label=\"%d: %c%s\"];\n", TNODE_UNIQUEID(node), TNODE_UNIQUEID(node), TNODE_LETTER(node), TNODE_END_OF_WORD_FLAG(node) ? " *" : " ^");
   }

   while (node != NULL)
   {
      TnodePtr child = TNODE_CHILD(node);
      fprintf(out, "\tn%d -> n%d;\n", TNODE_UNIQUEID(node), TNODE_UNIQUEID(child));
      _do_graphviz_from_dawg(child, out);
      node = TNODE_NEXT(node);
   }
}




TnodePtr TnodeInit(char Chap, TnodePtr Next, Bool EndOfWordFlag, char Level, size_t StarterDepth, TnodePtr Parent, Bool IsaChild)
{
   TnodePtr Result = (Tnode*)malloc(sizeof(Tnode));
   Result->Letter = Chap;
   Result->ArrayIndex = 0;
   Result->NumberOfChildren = 0;
   Result->MaxChildDepth = StarterDepth;
   Result->Next = Next;
   Result->Child = NULL;
   Result->Parent = Parent;
   Result->Dangling = FALSE;
   Result->EndOfWordFlag = EndOfWordFlag;
   Result->Level = Level;
   Result->IsaDirectChild = IsaChild;

   // The UniqueID is used for nothing but debugging.
   // It is easier to tell what node your working with if
   // each node has a different identifier.
   static int UniqueID=0;
   Result->UniqueID=UniqueID++;

   if (Result->UniqueID >= INT_MAX)
   {
      fprintf(stderr, "Crap to many nodes, really?\n");
      exit( -1 );
   }


   return Result;
}

// Define all of the member-setting functions as macros for faster performance
// in "Trie" creation.
#define TNODE_SET_ARRAY_INDEX(thistnode, index) (thistnode->ArrayIndex = index)
#define TNODE_SET_CHILD(thistnode, child) (thistnode->Child = child)
#define TNODE_SET_NEXT(thistnode, next) (thistnode->Next = next)
#define TNODE_SET_PARENT(thistnode, parent) (thistnode->Parent = parent)
#define TNODE_INCREMENT_NUMBEROFCHILDREN(thistnode) (thistnode->NumberOfChildren += 1)
#define TNODE_SET_MAX_CHILD_DEPTH(thistnode, newdepth) (thistnode->MaxChildDepth = newdepth)
#define TNODE_SET_ISADIRECT_CHILD(thistnode, status) (thistnode->IsaDirectChild = status)
#define TNODE_SET_END_OF_WORD_FLAG(thistnode) (thistnode->EndOfWordFlag = TRUE)
#define TNODE_DANGLE_ME(thistnode) (thistnode->Dangling = TRUE)

// This function Dangles a node, but also recursively dangles every node under
// it as well, that way nodes that are not direct children do hit the chopping block.
// The function returns the total number of nodes dangled as a result.
int TnodeDangle(TnodePtr ThisTnode)
{
   int Result = 0;

   if ( TNODE_DANGLING(ThisTnode) == TRUE )
      return 0;

   if ( TNODE_NEXT(ThisTnode) != NULL )
      Result += TnodeDangle(TNODE_NEXT(ThisTnode));

   if ( TNODE_CHILD(ThisTnode) != NULL )
      Result += TnodeDangle(TNODE_CHILD(ThisTnode));

   TNODE_DANGLE_ME(ThisTnode);
   Result += 1;

   return Result;
}

// This function returns the pointer to the Tnode in a parallel list of nodes
// with the letter "ThisLetter", and returns NULL if the Tnode does not exist.
// In the "NULL" case, an insertion will be required.
TnodePtr TnodeFindParaNode(TnodePtr ThisTnode, char ThisLetter)
{
   if ( ThisTnode == NULL )
      return NULL;

   TnodePtr Result = ThisTnode;
   if ( TNODE_LETTER(Result) == ThisLetter )
      return Result;

   while ( TNODE_LETTER(Result) < ThisLetter )
   {
      Result = TNODE_NEXT(Result);
      if ( Result == NULL )
         return NULL;
   }

   if ( TNODE_LETTER(Result) == ThisLetter )
   {
      return Result;
   } else
   {
      return NULL;
   }
}

// This function inserts a new Tnode before a larger letter node or at the end
// of a Para-List If the list does not exist then it is put at the beginnung.
// The new node has "ThisLetter" in it.  "AboveTnode" is the node 1 level above
// where the new node will be placed.
// This function should never be passed a "NULL" pointer.  "ThisLetter" should
// never exist in the "Child" Para-List.
TnodePtr TnodeInsertParaNode(TnodePtr AboveTnode, char ThisLetter, char WordEnder, size_t StartDepth)
{
   TNODE_INCREMENT_NUMBEROFCHILDREN(AboveTnode);

   TnodePtr ReturnedTnode;

   // Case 1:  Para-List does not exist yet, so start it.
   if ( TNODE_CHILD(AboveTnode) == NULL )
   {
      TnodePtr child = TnodeInit(ThisLetter, NULL, WordEnder, TNODE_LEVEL(AboveTnode) + 1, StartDepth, AboveTnode, TRUE);

      TNODE_SET_CHILD(AboveTnode, child);
      ReturnedTnode = child;
   // Case 2: ThisLetter should be the first in the Para-List that already exists.
   } else if ( TNODE_LETTER(TNODE_CHILD(AboveTnode)) > ThisLetter )
   {
      TnodePtr Holder = TNODE_CHILD(AboveTnode);
      // The "Holder" node will no longer be a direct child, so set it as such.
        TnodePtr child = TnodeInit(ThisLetter, Holder, WordEnder, TNODE_LEVEL(AboveTnode) + 1, StartDepth, AboveTnode, TRUE );
        TNODE_SET_ISADIRECT_CHILD(Holder, FALSE);
      TNODE_SET_CHILD(AboveTnode, child);
      // "Holder"'s "Parent" is now the new node, so make the necessary change.
      TNODE_SET_PARENT(Holder, TNODE_CHILD(AboveTnode));
        ReturnedTnode = child;
   }
   // Case 3: The ParaList exists and ThisLetter is not first in the list.  This is the default case condition:  "if ( TNODE_LETTER(TNODE_CHILD(AboveTnode)) < ThisLetter )".
   else
   {
      TnodePtr Currently = TNODE_CHILD(AboveTnode);
      while ( TNODE_NEXT(Currently) != NULL )
      {
         if ( TNODE_LETTER(TNODE_NEXT(Currently)) > ThisLetter )
            break;
         Currently = TNODE_NEXT(Currently);
      }
      TnodePtr Holder = TNODE_NEXT(Currently);
      TnodePtr next = TnodeInit(ThisLetter, Holder, WordEnder, TNODE_LEVEL(AboveTnode) + 1, StartDepth, Currently, FALSE);

      TNODE_SET_NEXT(Currently, next);
      if ( Holder != NULL )
         TNODE_SET_PARENT(Holder, TNODE_NEXT(Currently));

        ReturnedTnode = next;
   }

    return ReturnedTnode;
}

// This function returns "TRUE" if "FirstNode" and "SecondNode" are found to be
// the parent nodes of equal tree branches.  This includes identical nodes in
// the current list.
// The "MaxChildDepth" of the two nodes can not be assumed equal due to the
// recursive nature of this function, so we must check for equivalence.
Bool TnodeAreWeTheSame(TnodePtr FirstNode, TnodePtr SecondNode)
{
   if ( FirstNode == SecondNode )
      return TRUE;
   if ( FirstNode == NULL || SecondNode == NULL )
      return FALSE;
   if ( TNODE_LETTER(FirstNode) != TNODE_LETTER(SecondNode) )
      return FALSE;
   if ( TNODE_MAX_CHILD_DEPTH(FirstNode) != TNODE_MAX_CHILD_DEPTH(SecondNode) )
      return FALSE;
   if ( TNODE_NUMBER_OF_CHILDREN(FirstNode) != TNODE_NUMBER_OF_CHILDREN(SecondNode) )
      return FALSE;
   if ( TNODE_END_OF_WORD_FLAG(FirstNode) != TNODE_END_OF_WORD_FLAG(SecondNode) )
      return FALSE;
   if ( TnodeAreWeTheSame(TNODE_CHILD(FirstNode), TNODE_CHILD(SecondNode)) == FALSE )
      return FALSE;
   if ( TnodeAreWeTheSame(TNODE_NEXT(FirstNode), TNODE_NEXT(SecondNode)) == FALSE )
      return FALSE;
   else
      return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// "Trie" functionality.

#pragma mark - trie_struct

typedef struct trie_struct
{
   int NumberOfTotalWords;
   int NumberOfTotalNodes;
   TnodePtr First;
} Trie, *TriePtr;

// Set up the "First" node in the Trie.
TriePtr TrieInit (void)
{
   TriePtr Result;
   Result = (TriePtr)malloc(sizeof(Trie));

   Result->NumberOfTotalWords = 0;
   Result->NumberOfTotalNodes = 0;
   // The "First" node in the "Trie" will be a dummy node.
   Result->First = TnodeInit('?', NULL, FALSE, 0, 0, NULL, FALSE);
   return Result;
}

void printTrie(TriePtr trie)
{
   DEBUGLOG(DEBUG2,("NumberOfTotalWords=%d\n",trie->NumberOfTotalWords));
   DEBUGLOG(DEBUG2,("NumberOfTotalNodes=%d\n",trie->NumberOfTotalNodes));

   printTnode_recursive(trie->First);
}



TnodePtr TrieRootTnode(TriePtr ThisTrie)
{
   if ( ThisTrie->NumberOfTotalWords > 0 )
   {
      return TNODE_CHILD(ThisTrie->First);
   } else
   {
      fprintf(stderr, "Oops\n");
      exit (1);
   }
}

// This function simply makes "TrieAddWord" look a lot smaller.  It returns the number of new nodes that it just inserted.
int TnodeAddWord(TnodePtr ParentNode, const char *Word)
{
   int Result = 0;
   //int X, Y;
   size_t WordLength = strlen(Word);
   DEBUGLOG(DEBUG2,("TnodeAddWord Adding word '%s'\n",Word));
   TnodePtr HangPoint = NULL;
   TnodePtr Current = ParentNode;
   // Try to follow the path of "Word" until it doesn't exist, and then hang a new chain of nodes to include it in the trie.
   for ( unsigned int X = 0; X < WordLength; X++ )
   {
      char c = Word[X];
      DEBUGLOG(DEBUG2,("%*c\n", (int)(WordLength * X),c));
      HangPoint = TnodeFindParaNode(TNODE_CHILD(Current), c);



      // This would be essentially the first time
      if ( HangPoint == NULL)
      {
         //  AboveTnode,  ThisLetter,  WordEnder,StartDepth
         Bool WordEndFlag  = ((X == (WordLength - 1))? TRUE: FALSE);
         unsigned long depth = WordLength - X - 1;
         DEBUGLOG(DEBUG2,("Major Tnode =%d\n",TNODE_UNIQUEID(Current)));
         TnodeInsertParaNode(Current, c, WordEndFlag, depth);
         Result += 1;

         Current = TnodeFindParaNode(TNODE_CHILD(Current), c);
         for ( unsigned int  Y = X + 1; Y < WordLength; Y++ )
         {
            c = Word[Y];
            Bool WordEndFlag  = ((Y == (WordLength - 1))? TRUE: FALSE);
            unsigned long depth = WordLength - Y - 1;
            DEBUGLOG(DEBUG2,("Major Child Tnode =%d\n",TNODE_UNIQUEID(Current)));
            TnodeInsertParaNode(Current, c, WordEndFlag, depth);
            Result += 1;

            Current = TNODE_CHILD(Current);

         }
         break;
      }
      else if ( TNODE_MAX_CHILD_DEPTH(HangPoint) < (WordLength - X - 1) )
      {
         TNODE_SET_MAX_CHILD_DEPTH(HangPoint, (WordLength - X - 1));
      }
      Current = HangPoint;
      // The path for the word that we are trying to insert already exists, so
      // just make sure that the end flag is flying on the last node.  This
      // should never happen if words are added in alphabetical order, but
      // this is not a requirement of the program.
      if ( X == WordLength - 1 )
         TNODE_SET_END_OF_WORD_FLAG(Current);
   }
   return Result;
}

// This function adds to "ThisTrie"'s counter variables and adds "NewWord", using "TnodeAddWord", where the real addition algorithm exists.
void TrieAddWord(TriePtr ThisTrie, char *NewWord)
{
   ThisTrie->NumberOfTotalWords += 1;
   ThisTrie->NumberOfTotalNodes += TnodeAddWord( ThisTrie->First, NewWord);
}

// This is a standard depth-first preorder tree traversal, whereby the objective
// is to count nodes of various "MaxChildDepth"s.
// The counting results are placed into the "Tabulator" array.  This will be used to streamline the "Trie"-to-"Dawg" conversion process.
void TnodeGraphTabulateRecurse(TnodePtr ThisTnode, int Level, int* Tabulator)
{
   if ( Level == 0 )
   {
      TnodeGraphTabulateRecurse(TNODE_CHILD(ThisTnode), Level + 1, Tabulator);

   } else if ( TNODE_DANGLING(ThisTnode) == FALSE )
   {
      Tabulator[TNODE_MAX_CHILD_DEPTH(ThisTnode)] += 1;

      // Go Down if possible.
      if ( TNODE_CHILD(ThisTnode) != NULL )
         TnodeGraphTabulateRecurse(TNODE_CHILD(ThisTnode), Level + 1, Tabulator );

      // Go Right through the Para-List if possible.
      if ( TNODE_NEXT(ThisTnode) != NULL )
         TnodeGraphTabulateRecurse(TNODE_NEXT(ThisTnode), Level, Tabulator );
   }
}

// Count the "Tnode"'s in "ThisTrie" for each "MaxChildDepth".  "Count" needs
// to be an integer array of size "MAX_WORD_LENGTH".
int* TrieGraphTabulate(TriePtr ThisTrie)
{
   // Create the array and init to all zeros first
   int *Numbers = calloc(MAX_WORD_LENGTH+1, sizeof(int));

   if ( ThisTrie->NumberOfTotalWords > 0)
   {
      TnodeGraphTabulateRecurse(ThisTrie->First, 0, Numbers);
   }
   return Numbers;
}

// This function can never be called after a trie has been mowed because this
// will free pointers twice resulting in a core dump!
void FreeTnodeRecurse(TnodePtr ThisTnode)
{
   if ( TNODE_CHILD(ThisTnode) != NULL )
      FreeTnodeRecurse(TNODE_CHILD(ThisTnode));
   if ( TNODE_NEXT(ThisTnode) != NULL )
      FreeTnodeRecurse(TNODE_NEXT(ThisTnode));
   free(ThisTnode);
}

// An important function, it returns the first node in the list "MaxChildDepthWist",'// that is identical to "ThisTnode".
// If the function returns a value equal to "ThisTnode", then it is the first'// of its kind in the "Trie"
TnodePtr TnodeMexicanEquivalent( TnodePtr ThisTnode, TnodePtr ** MaxChildDepthWist )
{
   size_t Tall = TNODE_MAX_CHILD_DEPTH(ThisTnode);
   unsigned int X = 0;
   while ( TnodeAreWeTheSame(ThisTnode, MaxChildDepthWist[Tall][X]) == FALSE )
   {
      X += 1;
   }
   return MaxChildDepthWist[Tall][X];
}

// Recursively replaces all redundant nodes in a trie with their first equivalent.
void TnodeLawnMowerRecurse(TnodePtr ThisTnode, TnodePtr ** HeightWits)
{
   // printf("\nTnodeLawnMowerRecurse\n");

   if ( TNODE_LEVEL(ThisTnode) == 0 )
   {
      TnodeLawnMowerRecurse(TNODE_CHILD(ThisTnode), HeightWits);
   } else
   {
      // When replacing a "Tnode", we must do so knowing that "ThisTnode" is how we got to it.
      if ( TNODE_NEXT(ThisTnode) == NULL && TNODE_CHILD(ThisTnode) == NULL )
         return;

      if ( TNODE_CHILD(ThisTnode) != NULL )
      {
         // we have found a node that has been tagged for mowing, so let us
         // destroy it, not literally, and replace it with its first equivelant
         // in the "HeightWits" list, which won't be tagged.
         if ( TNODE_DANGLING(TNODE_CHILD(ThisTnode)) == TRUE )
         {
            TNODE_SET_CHILD(ThisTnode, TnodeMexicanEquivalent(TNODE_CHILD(ThisTnode), HeightWits));
         } else
         {
            TnodeLawnMowerRecurse( ThisTnode->Child, HeightWits );
         }
      }
      // Traverse the rest of the "Trie", but a "Tnode" that is not a direct
      // child will never be directly replaced.
      // This will allow the resulting "Dawg" to fit into a contiguous array
      // of node lists.
      if ( TNODE_NEXT(ThisTnode) != NULL )
         TnodeLawnMowerRecurse(TNODE_NEXT(ThisTnode), HeightWits);
   }
}

// Replaces each redundant list in "ThisTrie" with its first equivalent.
void TrieLawnMower(TriePtr ThisTrie, TnodePtr ** HolderOfAllTnodePointers)
{
   TnodeLawnMowerRecurse(ThisTrie->First, HolderOfAllTnodePointers );
}

///////////////////////////////////////////////////////////////////////////////
// A queue is required for breadth first traversal, and the rest is self evident.

#pragma mark - breadthqueuenode_struct

typedef struct breadthqueuenode_struct
{
   TnodePtr Tnode;
   struct breadthqueuenode_struct *Next;
} BreadthQueueNode,* BreadthQueueNodePtr;



// In the spirit of keeping the code fast, use macros for basic functionality.
#define BREADTH_QUEUE_NODE_NEXT(thisbreadthqueuenode) (thisbreadthqueuenode->Next)
#define BREADTH_QUEUE_TNODE(thisbreadthqueuenode) (thisbreadthqueuenode->Tnode)
#define BREADTH_QUEUE_NODE_SET_NEXT(thisbreadthqueuenode, next) (thisbreadthqueuenode->Next = next)
#define BREADTH_QUEUE_NODE_SET_TNODE(thisbreadthqueuenode, tnode) (thisbreadthqueuenode->Tnode = tnode)

BreadthQueueNodePtr BreadthQueueNodeInit(TnodePtr Tnode)
{
   BreadthQueueNodePtr Result = (BreadthQueueNode*)malloc(sizeof(BreadthQueueNode));
   Result->Tnode = Tnode;
   Result->Next = NULL;
   return Result;
}
#pragma mark - breadthqueue_struct

typedef struct breadthqueue_struct
{
   BreadthQueueNodePtr Front;
   BreadthQueueNodePtr Back;
   int Size;
} BreadthQueue, *BreadthQueuePtr;

#define BREADTH_QUEUE_FRONT(thisbreadthqueue) (thisbreadthqueue->Front)
#define BREADTH_QUEUE_BACK(thisbreadthqueue) (thisbreadthqueue->Back)
#define BREADTH_QUEUE_SIZE(thisbreadthqueue) (thisbreadthqueue->Size)
#define BREADTH_QUEUE_SET_FRONT(thisbreadthqueue, front) (thisbreadthqueue->Front = front)
#define BREADTH_QUEUE_SET_BACK(thisbreadthqueue, back) (thisbreadthqueue->Back = back)
#define BREADTH_QUEUE_SET_SIZE(thisbreadthqueue, size) (thisbreadthqueue->Size = size)
#define BREADTH_QUEUE_DECREMENT_SIZE(thisbreadthqueue) (thisbreadthqueue->Size--)
#define BREADTH_QUEUE_INCREMENT_SIZE(thisbreadthqueue) (thisbreadthqueue->Size++)

// By defining CLANG_ANALYZER_NORETURN, the analyzer wont complain about
// allocatig memory, without dealocating it in the same function
void printBreadthQueue(BreadthQueuePtr breadthQueue) CLANG_ANALYZER_NORETURN
{

   DEBUGLOG(DEBUG2,("*** FRONT QUEUE\n"));

   BreadthQueueNodePtr front  = BREADTH_QUEUE_FRONT(breadthQueue);

   while ( front != NULL )
   {
      BreadthQueueNodePtr next = BREADTH_QUEUE_NODE_NEXT(front);
      printTnode_recursive(BREADTH_QUEUE_TNODE(front));

      front = next;
   }

   DEBUGLOG(DEBUG2,("*** BACK QUEUE\n"));
   BreadthQueueNodePtr back  = BREADTH_QUEUE_BACK(breadthQueue);

   while ( back != NULL )
   {
      BreadthQueueNodePtr next = BREADTH_QUEUE_NODE_NEXT(back);
      printTnode_recursive(BREADTH_QUEUE_TNODE(back));

      back = next;
   }
}

// By defining CLANG_ANALYZER_NORETURN, the analyzer wont complain about
// allocatig memory, without dealocating it in the same function
BreadthQueuePtr BreadthQueueInit( void ) CLANG_ANALYZER_NORETURN
{
   BreadthQueuePtr Result = (BreadthQueue*)malloc(sizeof(BreadthQueue));
   Result->Front = NULL;
   Result->Back = NULL;
   Result->Size = 0;

   return Result;
}

void BreadthQueuePush(BreadthQueuePtr ThisBreadthQueue, TnodePtr NewTnode)
{

   BreadthQueueNodePtr Noob = BreadthQueueNodeInit(NewTnode);

   if ( BREADTH_QUEUE_BACK(ThisBreadthQueue) != NULL )
   {
      BREADTH_QUEUE_NODE_SET_NEXT(ThisBreadthQueue->Back, Noob);
   } else
   {
      BREADTH_QUEUE_SET_FRONT(ThisBreadthQueue, Noob);
   }

   BREADTH_QUEUE_SET_BACK(ThisBreadthQueue, Noob);

   BREADTH_QUEUE_INCREMENT_SIZE(ThisBreadthQueue);
}

TnodePtr BreadthQueuePop(BreadthQueuePtr ThisBreadthQueue)
{
   TnodePtr Result;
   if ( BREADTH_QUEUE_SIZE(ThisBreadthQueue) == 0 )
      return NULL;

   if ( BREADTH_QUEUE_SIZE(ThisBreadthQueue) == 1 )
   {
      BREADTH_QUEUE_SET_BACK(ThisBreadthQueue, NULL);
      BREADTH_QUEUE_SET_SIZE(ThisBreadthQueue, 0);
      Result = BREADTH_QUEUE_TNODE(BREADTH_QUEUE_FRONT(ThisBreadthQueue));
      free(BREADTH_QUEUE_FRONT(ThisBreadthQueue));
      BREADTH_QUEUE_SET_FRONT(ThisBreadthQueue, NULL);
      return Result;
   }
   Result = BREADTH_QUEUE_TNODE(BREADTH_QUEUE_FRONT(ThisBreadthQueue));
   BreadthQueueNodePtr Holder = BREADTH_QUEUE_FRONT(ThisBreadthQueue);
   BREADTH_QUEUE_SET_FRONT(ThisBreadthQueue, BREADTH_QUEUE_NODE_NEXT(BREADTH_QUEUE_FRONT(ThisBreadthQueue)));
   free(Holder);
   BREADTH_QUEUE_DECREMENT_SIZE(ThisBreadthQueue);

   return Result;
}
// By defining CLANG_ANALYZER_NORETURN, the analyzer wont complain about
// allocatig memory, without dealocating it in the same function
void BreadthQueuePopulateReductionArray(TnodePtr Root, TnodePtr **Holder) CLANG_ANALYZER_NORETURN
{
   DEBUGLOG(DEBUG2,("Inside external BreadthQueuePopulateReductionArray function.\n"));

   TnodePtr Current = Root;

   int Taker[MAX_WORD_LENGTH + 1];
   memset(&Taker, 0, sizeof(Taker));

   BreadthQueuePtr Populator = BreadthQueueInit();
   BreadthQueuePtr ThisBreadthQueue=Populator;

   // Push the first row onto the queue.
   while ( Current != NULL )
   {
      DEBUGLOG(DEBUG2,("Push Populator for Reduction array\n"));
      BreadthQueuePush(Populator, Current);
      Current = TNODE_NEXT(Current);
   }


   // Initiate the pop-followed-by-push-all-children loop.
   while ( (ThisBreadthQueue->Size) != 0 )
   {
      Current = BreadthQueuePop(ThisBreadthQueue);
      size_t CurrentMaxChildDepth = TNODE_MAX_CHILD_DEPTH(Current);
      Holder[CurrentMaxChildDepth][Taker[CurrentMaxChildDepth]] = Current;
      Taker[CurrentMaxChildDepth] += 1;
      Current = TNODE_CHILD(Current);
      while ( Current != NULL )
      {
         BreadthQueuePush(ThisBreadthQueue, Current);
         Current = TNODE_NEXT(Current);
      }
   }
   DEBUGLOG(DEBUG1,("Completed Populating The Reduction Array.\n"));
}

// This function will label all of the remaining nodes in the Trie-Turned-Dawg
// so that they will fit contiguously into an unsigned integer array.
// The return value is the final index number handed out to the "Tnode"'s, and
// hence one less than the size of the array that they need to fit into.
int BreadthQueueUseToIndex(TnodePtr root)
{

   // The first index to be given out will be "1" because "0" denotes "NULL".
   int IndexNow = 0;
   BreadthQueuePtr OrderMatters = BreadthQueueInit();

   // Push the first Para-List onto the queue.
   BreadthQueuePtr ThisBreadthQueue = OrderMatters;
   TnodePtr Root = root;

   TnodePtr Current = Root;

    DEBUGLOG(DEBUG2,("Root '%c' %d\n",TNODE_LETTER(Current),TNODE_UNIQUEID(Root)));
    while ( Current != NULL )
    {
       DEBUGLOG(DEBUG2,("Push First '%c' %d\n",TNODE_LETTER(Current),TNODE_UNIQUEID(Current)));
       BreadthQueuePush(ThisBreadthQueue, Current);
       Current = TNODE_NEXT(Current);
    }


   // Pop each element off of the queue and only push its children if has not
   // been dangled yet.
   // Assign an index to the node, if one has not been given to it yet. Nodes
   // will be placed on the queue many times.
   while ( (ThisBreadthQueue->Size) != 0 )
   {
      TnodePtr CurrentTnode = BreadthQueuePop(ThisBreadthQueue);
      DEBUGLOG(DEBUG2,("pop %d\n",TNODE_UNIQUEID(CurrentTnode)));
      // If the "Current" node already has an index, don't give it a new one.
      if ( TNODE_ARRAY_INDEX(CurrentTnode) == 0 )
      {
         IndexNow += 1;
         DEBUGLOG(DEBUG2,("Increment index for Unique Tnode '%c'%d\n",TNODE_LETTER(CurrentTnode),TNODE_UNIQUEID(CurrentTnode)));
         TNODE_SET_ARRAY_INDEX(CurrentTnode, IndexNow);
         CurrentTnode = TNODE_CHILD(CurrentTnode);
         while ( CurrentTnode != NULL )
         {
            DEBUGLOG(DEBUG2,("Push onto BreadthQueue'%c'%d'\n",TNODE_LETTER(CurrentTnode),TNODE_UNIQUEID(CurrentTnode)));
            BreadthQueuePush(ThisBreadthQueue, CurrentTnode);
            CurrentTnode = TNODE_NEXT(CurrentTnode);
         }
      }
   }
   DEBUGLOG(DEBUG1,("Completed Assigning Indices To Living Nodes.\n"));
   return IndexNow;
}

///////////////////////////////////////////////////////////////////////////////
// This section contains code related to the format of unsigned integers that
// represent "Dawg" nodes.

// The "BinaryNode" string must be at least 32 + 5 + 1 bytes in length.  Space
// for the bits, the seperation pipes, and the end of string char.
void ConvertUnsignedIntNodeToBinaryString(unsigned int TheNode, char* BinaryNode)
{

   BinaryNode[0] = '|';
   // Bit 31, the leftmost bit, represents the "EndOfWordFlag".
   BinaryNode[1] = (TheNode & PowersOfTwo[31])?'1':'0';
   BinaryNode[2] = '|';
   // Bit 30 represents the "EndOfListFlag".
   BinaryNode[3] = (TheNode & PowersOfTwo[30])?'1':'0';
   BinaryNode[4] = '|';
   // Bit 29 to bit 25 are the "Letter" bits.
   BinaryNode[5] = (TheNode & PowersOfTwo[29])?'1':'0';
   BinaryNode[6] = (TheNode & PowersOfTwo[28])?'1':'0';
   BinaryNode[7] = (TheNode & PowersOfTwo[27])?'1':'0';
   BinaryNode[8] = (TheNode & PowersOfTwo[26])?'1':'0';
   BinaryNode[9] = (TheNode & PowersOfTwo[25])?'1':'0';
   BinaryNode[10] = '|';
   // Bit 24 to bit 0, are space of the first child index.
   //int Bit = 24;
   for (int X = 11, Bit = 24; X <= 35; X++, Bit-- )
   {
      BinaryNode[X] = (TheNode & PowersOfTwo[Bit])? '1': '0';
      //Bit -= 1;
   }
   BinaryNode[36] = '|';
   BinaryNode[37] = '\0';
}



///////////////////////////////////////////////////////////////////////////////

// This function is the core of the "Dawg" creation procedure.  Pay close
// attention to the order of the steps involved.
#pragma mark - dawg_struct

typedef struct dawg_struct
{
   unsigned int node_count;
   int *node_number_count_for_wordlength;
   TnodePtr **root;
} dawg, *dawgPtr;

#pragma mark - ENTRY freeTheDawg

void freeTheDawg(dawgPtr dawg)
{
   if (dawg)
   {
      if (dawg->root)
      {
         // Do Garbage cleanup and free the whole Trie, which is no longer needed.  Free all nodes from the holding array.
         for (int X = 0; X <= MAX_WORD_LENGTH; X++ )
         {
            if (dawg->root[X])
            {
               for (unsigned int W = 0; W < dawg->node_number_count_for_wordlength[X]; W++ )
               {
                  if (dawg->root[X][W])
                  {
                     free(dawg->root[X][W]);
                  }
               }
            }
            if (dawg->root[X])
              free(dawg->root[X]);
         }
         free(dawg->root);
      }

      if (dawg->node_number_count_for_wordlength)
         free(dawg->node_number_count_for_wordlength);

      free(dawg);
   }
}


#pragma mark - ENTRY Dawg from Dictionary in RAM

dawgPtr DawgFromRAMDictionary(char *Dictionary[], int *WordsOfLength)
{
   DEBUGLOG(DEBUG1,("\n ****** DICTIONARY ******\n"));
   printDictionary(Dictionary, WordsOfLength);
   DEBUGLOG(DEBUG1,("\n\n"));

   DEBUGLOG(DEBUG1,("\nStep 1 - Create the intermediate Trie and begin filling it with the provided lexicon.\n"));
   // Create a Temp Trie structure and then feed in the given lexicon.
   TriePtr TemporaryTrie = TrieInit();
   for ( int X = MIN_WORD_LENGTH; X <= MAX_WORD_LENGTH; X++ )
   {
      for (unsigned int Y = 0; Y < WordsOfLength[X]; Y++ )
      {
         TrieAddWord(TemporaryTrie, &(Dictionary[X][(X + 1)*Y]));
      }
   }

   DEBUGLOG(DEBUG1,("\n ****** ORIGINAL TRIE TREE ******\n"));
   printTrie(TemporaryTrie);
   DEBUGLOG(DEBUG1,("\n\n"));


   DEBUGLOG(DEBUG1,("\nStep 2 - Finished adding words to the temporary Trie, so set up the space to sort the Tnodes by Word Lengths.\n"));

   // Create the associated pointer array with all the nodes inside it.
   int *InitialNodeNumberCountForWordLength = (int*)calloc((MAX_WORD_LENGTH+1),sizeof(int) );


   DEBUGLOG(DEBUG1,("\nStep 3 - Count the total number of nodes in the raw Trie by MaxChildDepth.\n"));

   int *NumberCountForWordLength = TrieGraphTabulate(TemporaryTrie);




   DEBUGLOG(DEBUG1,("\nStep 4 - Initial counting of Trie nodes complete, so display the results.\n\n"));

   int TotalNodeSum = 0;
   for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
   {
      // Save the initial number of nodes
      InitialNodeNumberCountForWordLength[X] = NumberCountForWordLength[X];

      DEBUGLOG(DEBUG1,("Initial Count For Words of Size |%2d| is |%6d| nodes\n", X, InitialNodeNumberCountForWordLength[X]));

      TotalNodeSum += InitialNodeNumberCountForWordLength[X];
   }

   DEBUGLOG(DEBUG1,("\nThe Total Number Of Nodes In The Trie = |%d| \n", TotalNodeSum));


   DEBUGLOG(DEBUG1,("\nStep 5 - Allocate a 2 dimensional array of Tnode pointers to minimize the trie into a Dawg.\n"));


   TnodePtr **HolderOfAllTnodePointers = (TnodePtr **)malloc((MAX_WORD_LENGTH +1)* sizeof(TnodePtr*));



   for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
   {
      if (InitialNodeNumberCountForWordLength[X] > 0)
      {

         HolderOfAllTnodePointers[X] = (TnodePtr *)malloc(InitialNodeNumberCountForWordLength[X]*sizeof(TnodePtr));

      } else
      {
          HolderOfAllTnodePointers[X] = NULL;
      }
   }



   // When populating the "HolderOfAllTnodePointers", we are going to do so in
   // a breadth first manner to ensure that the next "Tnode" in a list located
   // at the next array index.
   DEBUGLOG(DEBUG1,("\nStep 6 - Init the 2 dimensional array of Trie node pointers.\n"));


   BreadthQueuePopulateReductionArray(TrieRootTnode(TemporaryTrie), HolderOfAllTnodePointers );

   //printf("\n ****** ORIGINAL HolderOfAllTnodePointers ******\n");
   //printHolderOfAllTNodes(HolderOfAllTnodePointers);
   //printf("\n\n");


   DEBUGLOG(DEBUG1,("\nStep 7 - Population complete.\n"));
   // Flag all of the reduntant nodes.
   // Flagging requires the node comparison function that will take a very long
   // time for a big dictionary.
   // This is especially true when comparing the nodes with small
   // "MaxChildDepth"'s because there are so many of them.
   // It is faster to start with nodes of the largest "MaxChildDepth" to
   // recursively reduce as many lower nodes as possible.

   DEBUGLOG(DEBUG1,("\nStep 8 - Begin to tag redundant nodes as dangled - Use recursion because only direct children are considered for elimination to keep the remaining lists in tact.\n"));
   DEBUGLOG(DEBUG1,("This may take a while ...\n"));

   // Start at the largest "WORD LENGTH" and work down from there for recursive
   // reduction to take place early on to reduce the work load for the shallow nodes.

   int DangleCount[MAX_WORD_LENGTH];
   int TotalDangled = 0;
   for ( int Y = MAX_WORD_LENGTH-1; Y >= 0 ; Y--)
   {
      int NumberDangled = 0;
      // Move through the holder array from the beginning, looking for any nodes that have not been dangled, these will be the surviving nodes.
      for ( unsigned int W = 0; W < (InitialNodeNumberCountForWordLength[Y]-1); W++ )
      {
         // The Node is already Dangling.  Note that this node need not be the
         // first in a child list, it could have been dangled recursively.
         // In order to eliminate the need for the "Next" index, the nodes at
         // the root of elimination must be the first in a list, in other words,
         // a "DirectChild".
         // The node that we replace the "DirectChild" node with can be located
         // at any position.
         TnodePtr first = HolderOfAllTnodePointers[Y][W];
         if ( first == NULL )
            continue;

         if ( TNODE_DANGLING(first) == TRUE )
            continue;

         // Traverse the rest of the array looking for equivalent nodes that are
         // both not dangled and are tagged as direct children.
         // When we have found an identical list structure further on in the
         // array, dangle it, and all the nodes coming after, and below it.
         for ( unsigned int X = W + 1; X < InitialNodeNumberCountForWordLength[Y]; X++ )
         {
            TnodePtr other = HolderOfAllTnodePointers[Y][X];

            if ( TNODE_DANGLING(other) == FALSE && TNODE_ISADIRECT_CHILD(other) == TRUE )
            {
               if ( TnodeAreWeTheSame(first, other) == TRUE )
               {
                  NumberDangled += TnodeDangle(other);
               }
            }
         }
      }
      DEBUGLOG(DEBUG1,("Dangled |%5d| Nodes In all, through recursion, for words of length |%2d|\n", NumberDangled, Y));
      DangleCount[Y] = NumberDangled;
      TotalDangled += NumberDangled;
   }

   DEBUGLOG(DEBUG1,("\nTotal Number Of Dangled Nodes |%d|\n", TotalDangled));
   int NumberOfLivingNodesAfterDangling = TotalNodeSum - TotalDangled;
   DEBUGLOG(DEBUG1,("\nTotal Number Of Living Nodes |%d|\n", NumberOfLivingNodesAfterDangling));

   DEBUGLOG(DEBUG1,("\nStep 9 - Count the number of living nodes in the trie before replacement so that we check our numbers.\n"));
   // By running the graph tabulation function on a different array, and before
   // we replace the nodes, we can determine if our numbers are correctish.
   int *NodeNumberCountForWordLengthCheck =TrieGraphTabulate(TemporaryTrie);

   for (unsigned int X = 0; X < MAX_WORD_LENGTH; X++ )
   {
      DEBUGLOG(DEBUG1,("Count for living nodes of MaxChildDepth |%2d| is |%5d|. It used to be |%6d| and so the number dangled is |%6d| \n", X, NodeNumberCountForWordLengthCheck[X], InitialNodeNumberCountForWordLength[X], InitialNodeNumberCountForWordLength[X] - NumberCountForWordLength[X]));
   }
   free(NumberCountForWordLength);

   unsigned int TotalDangledCheck = 0;
   for ( int X = 0; X < MAX_WORD_LENGTH; X++ )
   {
      TotalDangledCheck += (InitialNodeNumberCountForWordLength[X] - NodeNumberCountForWordLengthCheck[X]);
   }
   free(NodeNumberCountForWordLengthCheck);

   if ( TotalDangled == TotalDangledCheck )
   {
      DEBUGLOG(DEBUG1,("The total number of nodes dangled %d adds up.\n", TotalDangled));
   } else
   {
      DEBUGLOG(DEBUG1,("Something went terribly wrong, Total Dangled %d not equal to check %d so fix it.\n", TotalDangled, TotalDangledCheck));
      exit(-1);
   }


   DEBUGLOG(DEBUG1,("\n ****** HolderOfAllTnodePointers AFTER DANGLING ******\n"));
   printHolderOfAllTNodes(HolderOfAllTnodePointers);
   DEBUGLOG(DEBUG1,("\n\n"));



   DEBUGLOG(DEBUG1,("\nStep 10 - Dangling is complete, so replace all dangled nodes with their first mexican equivelant in the Trie to make a compressed Dawg.\n"));
   // Node replacement has to take place before indices are set up so nothing points to redundant nodes. - This step is absolutely critical.  Mow The Lawn so to speak!  Then assign indicies.
   TrieLawnMower( TemporaryTrie, HolderOfAllTnodePointers );

   DEBUGLOG(DEBUG1,("\n ****** Mowed TREE ******\n"));
   printTrie(TemporaryTrie);
   DEBUGLOG(DEBUG1,("\n\n"));

   DEBUGLOG(DEBUG1,("\n- Mowing of the lawn is now complete, so start to assign array indices to all living nodes.\n"));
   DEBUGLOG(DEBUG1,("Step 12 - The use of a breadth first queue during this step ensures that lists of contiguous nodes in the array will eliminate the need for a Next pointer.\n\n"));
   TnodePtr Root;

   Root = TNODE_CHILD(TemporaryTrie->First);
   if ( Root == NULL )
   {
      fprintf(stderr, "Something is screwy if there is no head of the trie\n");
      exit(-1);
   }


   // Try to find out if the nodes we are setting are unique before we set them.
   int IndexCount = BreadthQueueUseToIndex( Root );

   DEBUGLOG(DEBUG1,("Finished indexing.\n"));
   DEBUGLOG(DEBUG1,("NumberOfLivingNodes from after the dangling process|%d|\n", NumberOfLivingNodesAfterDangling));
   DEBUGLOG(DEBUG2,("IndexCount from the index-handing-out breadth first traversal |%d|\n", IndexCount));

   if ( NumberOfLivingNodesAfterDangling == IndexCount )
   {
      DEBUGLOG(DEBUG1,("The numbers add up properly once again.\n"));
   } else
   {
      fprintf(stderr, "The Numbers got Scrooged, so you still have some problems to iron out.\n");
      exit(-1);
   }

   dawgPtr dawg = (dawgPtr) calloc(1, sizeof(struct dawg_struct));
   dawg->node_count = NumberOfLivingNodesAfterDangling;
   dawg->node_number_count_for_wordlength = InitialNodeNumberCountForWordLength;
   dawg->root = HolderOfAllTnodePointers;

   free(TemporaryTrie);

   return dawg;


}

#pragma mark - ENTRY writeDawgToBinaryFile

void writeDawgToBinaryFile(dawgPtr dawg, FILE * out, Bool graphInstead)
{

   // Allocate the space needed to store the "Dawg" inside of an array.
   unsigned int *children_as_ints = (unsigned int*)calloc((dawg->node_count + 1), sizeof(unsigned int));



   DEBUGLOG(DEBUG1,("\nStep 13 - Populate the unsigned integer array with a bitwise encoding.\n"));
   // Traverse the entire 2D pointer array and search for undangled nodes.  When an undangled node is found, encode it, and place it at position "ArrayIndex".
   unsigned int IndexFollower = 0;
   for ( int X = (MAX_WORD_LENGTH - 1); X >= 0; X-- )
   {
      for (unsigned int W = 0;
           W < dawg->node_number_count_for_wordlength[X];
           W++)
      {
         TnodePtr node = dawg->root[X][W];
         if (!node)
            continue;

         if ( TNODE_DANGLING(node) == FALSE )
         {
            unsigned int UnsignedEncodingValue = TNODE_LETTER(node) - 'A';
            UnsignedEncodingValue <<= LETTER_BIT_SHIFT;
            UnsignedEncodingValue += (TNODE_END_OF_WORD_FLAG(node) == FALSE)? 0: END_OF_WORD_BIT_MASK;
            UnsignedEncodingValue += (TNODE_NEXT(node) == NULL)? END_OF_LIST_BIT_MASK: 0;
            UnsignedEncodingValue += (TNODE_CHILD(node) == NULL)? 0: TNODE_ARRAY_INDEX(TNODE_CHILD(node));
            int index = TNODE_ARRAY_INDEX(node);
            if ( index > IndexFollower )
            {
                IndexFollower = index;
            }
            children_as_ints[index] = UnsignedEncodingValue;
         }
      }
   }
   DEBUGLOG(DEBUG1,("IndexFollower, which is the largest index assigned in the array = |%d|\n", IndexFollower));
   DEBUGLOG(DEBUG1,("NumberOfLivingNodesAfterDangling|%d|, assert that these two are equal because they must be.\n", dawg->node_count));
   if ( IndexFollower == dawg->node_count )
   {
      DEBUGLOG(DEBUG1,("The numbers add up again, excellent!\n"));
   } else
   {
      fprintf(stderr,"Don't jump!  You are very close to getting this program working.\n");
      free(children_as_ints);
      freeTheDawg(dawg);
      exit(-1);
   }


   DEBUGLOG(DEBUG1,("\nCreation of traditional Dawg is complete,\n"));



   if ( graphInstead )
   {
      DEBUGLOG(DEBUG1,("\nStep 14 - Store it into a graphical file for verification\n"));

      fprintf(out, "Total number of Dawg nodes = |%d|, including \"0 = NULL\".\n\n", dawg->node_count);

      char TheNodeInBinary[BINARY_STRING_LENGTH];
      for (unsigned int X = 1; X < dawg->node_count; X++ )
      {
         ConvertUnsignedIntNodeToBinaryString(children_as_ints[X], TheNodeInBinary);
         fprintf(out, "Node|%6d|-Letter|%c|-EOW|%3s|-EOL|%3s|-Child|%6d| - Binary%s\n",
                 X,
                 ((children_as_ints[X]&LETTER_BIT_MASK)>>LETTER_BIT_SHIFT) + 'A',
                 (children_as_ints[X]&END_OF_WORD_BIT_MASK)? "yes": "no",
                 (children_as_ints[X]&END_OF_LIST_BIT_MASK)? "yes": "no",
                 children_as_ints[X]&CHILD_INDEX_BIT_MASK,
                 TheNodeInBinary);
      }

      DEBUGLOG(DEBUG1,("Completed graphing of dawg.\n"));

      fclose(out);
   } else
   {
      DEBUGLOG(DEBUG1,("\nStep 14 - Store it into a 32-bit binary file for use.\n"));

      // It is critical, especially in a binary file, that the first integer
      // written to the file be the number of nodes stored in the file.
      // Simply write the entire unsigned integer array "Result" into the data file.
      fwrite(&dawg->node_count, sizeof(int), 1, out);
      fwrite(children_as_ints, sizeof(unsigned int), dawg->node_count +1, out);
      fclose(out);

      DEBUGLOG(DEBUG1,("Completed 32 bit traditional data binary output.\n"));
   }

   free(children_as_ints);
}

void usage(char* name)
{
   char *whoami = name;
   (whoami = strrchr(name, '/')) ? ++whoami : (whoami = name);
   printf("%s - Direct Acylic Word Graph\n\n", whoami);
   printf("The Ultimate experiment in creating/recreating a DAWG.\n");
   printf("Besides reading the code:\n");
   printf(" -c <file> compile a sorted list of words into a DAWG structure.\n");
   printf(" -o <file> output dawg, graph, dictionary to file. If no output given, then stdout is the default.\n");
   printf(" -e <file> extract dawg or dawg in memory:\n");
  printf(" -g <file> graph dawg in memory.\n");
   printf(" -d <level> set debug level. Default is 1. 0 is none, 2 is more descriptive, 3 is verbose\n");
   printf("\n");
   printf(" -h This help, but you know that.\n");
   printf("You can string commands together, so this is actuall possible:\n");
   printf(" %s -c Lexicon.txt -o output.dat -g -o graph1.txt -d2 -e -o output.txt\n", whoami);
   printf("or\n");
   printf(" %s -d 0 -c Lexicon.txt -o output.dat -d1 -o output.txt -e Lexicon.txt -g -o graph2.txt\n", whoami);
   printf("\n");


}
Bool getMAX_WORD_LENGTH_AND_COUNT(FILE *dict)
{
   // MAX_WORD_LENGTH is a semi hash define as it is set only once.
   // A Trie must know the MAX WORD LENGTH before it starts so that memory is
   // not constantly being resized and take forever for large dictionaries to
   // create a DAWG.

   // This will get the largest word size, compare line count in file with
   // actual line count. Finally it positions the FILE handler at the first
   // dictionary word to be read.

   NUMBER_OF_LINES_IN_DICTIONARY = 0;

   unsigned int lineCountInFile = 0;

   char thisLine[INPUT_LIMIT];
   fgets(thisLine, INPUT_LIMIT, dict);

   // Correction is needed to get rid of the new line character that "fgets()" appends to the string.
   CUT_OFF_NEW_LINE(thisLine);
   DEBUGLOG(DEBUG1,("ThisLine is |%s|\n", thisLine));


   lineCountInFile = stringToUnsignedInt(thisLine);
   DEBUGLOG(DEBUG1,("FirstLineIsSize is |%u|\n", lineCountInFile));

   if ( lineCountInFile == 0 )
   {
      rewind(dict);
      fprintf(stderr, "No line count at beginning of file.\n");
      fprintf(stderr, "Will use counted value.\n");
   }


   while (fgets(thisLine, INPUT_LIMIT, dict) != NULL)
   {
      CUT_OFF_NEW_LINE(thisLine);

      size_t length = strlen( thisLine );
      if ( length == 0 )
      {
         fprintf(stderr,"Lexicon.txt no word on line %u, so bye\n\n", NUMBER_OF_LINES_IN_DICTIONARY);
         exit(-1);
      }
      if ( length > MAX_WORD_LENGTH )
      {
         MAX_WORD_LENGTH = (int) length;

         DEBUGLOG(DEBUG2,("Changing MAX_WORD_LENGTH:%zu for line:%u word '%s'\n", length, NUMBER_OF_LINES_IN_DICTIONARY, thisLine));
      }
      NUMBER_OF_LINES_IN_DICTIONARY += 1;

   }
   DEBUGLOG(DEBUG1,("Lexicon.txt read %u lines completed\n\n", NUMBER_OF_LINES_IN_DICTIONARY));

   if (lineCountInFile != 0 &&
       lineCountInFile != NUMBER_OF_LINES_IN_DICTIONARY)
   {
      fprintf(stderr, "Warning, First Line in dictionary file does not match counted number of lines in file %u != %u. Using counted value of %u.\n", NUMBER_OF_LINES_IN_DICTIONARY, lineCountInFile, NUMBER_OF_LINES_IN_DICTIONARY);

      // Even though it does not match, it is a number, so position the file
      // handler past it, so the dictionary can be read.
      rewind(dict);
      fgets(thisLine, INPUT_LIMIT, dict);

   } else if ( lineCountInFile != 0 )
   {
       DEBUGLOG(DEBUG1,("Line count (%u) matches\n", lineCountInFile));

       // It is a number, so position the file handler past it, so the dictionary can be read.
       rewind(dict);
       fgets(thisLine, INPUT_LIMIT, dict);
   } else
   {

       // It is not a number, so position the file handler to the beginning of the dictionary.
       rewind(dict);
   }

   return TRUE;
}

#pragma mark - ENTRY dawgFromWordFile

dawgPtr dawgFromWordFile(FILE *dict)
{
   char thisLine[INPUT_LIMIT];


   // This will get thelargest word size, compare line count in file with actual
   // line count. Finally it positions the FILE handler at tge first dictionary
   //word to read.
   getMAX_WORD_LENGTH_AND_COUNT(dict);


   char *AllWordsInEnglish[MAX_WORD_LENGTH + 1];
   for ( unsigned int X = 0; X <= MAX_WORD_LENGTH; X++ )
       AllWordsInEnglish[X] = NULL;


   // Read the memory straight into ram using dynamically allocated space to
   // count the words of each length.
   // All of the words of similar length will be stored sequentially in the
   // same array so that there will be (MAX + 1)  arrays in total.  The Smallest
   // length of a string is assumed to be 2.
   int numberOfWordsOfLength[MAX_WORD_LENGTH + 1];
   memset(&numberOfWordsOfLength, 0, sizeof(numberOfWordsOfLength));


   //for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
   //    numberOfWordsOfLength[X] = 0;

   char **LexiconInRam = (char**)malloc(NUMBER_OF_LINES_IN_DICTIONARY*sizeof(char*));
   memset(LexiconInRam, 0, NUMBER_OF_LINES_IN_DICTIONARY*sizeof(char*));

   int lineCounter = 0;
   size_t length;

   while (fgets(thisLine, INPUT_LIMIT, dict) != NULL)
   {
      CUT_OFF_NEW_LINE(thisLine);
      MakeMeAllCapital(thisLine);
      length = strlen( thisLine );
      if ( length <= 0 )
      {
         fprintf(stderr, "Lexicon.txt no word on line %d, so bye\n\n",lineCounter);
         exit(-1);
      }
      if ( length > MAX_WORD_LENGTH )
      {
         fprintf(stderr, "Lexicon.txt word '%s' is greater than %zu, Liar\n\n",thisLine, length);
         exit(-1);
      }

      numberOfWordsOfLength[length] += 1;


      LexiconInRam[lineCounter] = (char*)malloc((length + 1)*sizeof(char));
      strcpy(LexiconInRam[lineCounter], thisLine);

      if (lineCounter++ > NUMBER_OF_LINES_IN_DICTIONARY)
      {
         fprintf(stderr, "Lexicon.txt More lines in file than expected %d, Liar\n\n", lineCounter);
         exit(-1);
      }
   }
   DEBUGLOG(DEBUG3,("Lexicon.txt read %d lines completed\n\n", lineCounter));

   if (lineCounter != NUMBER_OF_LINES_IN_DICTIONARY)
   {
      fprintf(stderr, "Lexicon.txt Unexpected number of lines %d, Liar\n\n", lineCounter);
      exit(-1);
   }


   for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
      DEBUGLOG(DEBUG2,("There are |%5d| words of length |%2d|\n", numberOfWordsOfLength[X], X));

   // Allocate enough space to hold all of the words in strings so that we can add them to the trie by length.
   for ( int X = MIN_WORD_LENGTH; X <= MAX_WORD_LENGTH; X++ )
   {
      if ( numberOfWordsOfLength[X] > 0 )
      {
         AllWordsInEnglish[X] = (char*)malloc((X + 1)*numberOfWordsOfLength[X]*sizeof(char));
      } else
      {
         AllWordsInEnglish[X] = NULL;
      }
   }
   DEBUGLOG(DEBUG1,("\nInitial malloc() complete\n"));

   int WordLengthTracker[MAX_WORD_LENGTH + 1];
   memset(&WordLengthTracker, 0, sizeof(WordLengthTracker));

   //for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
   //    WordLengthTracker[X] = 0;

   // Copy all of the strings into "AllWordsInEnglish".
   for ( int X = 0; X < NUMBER_OF_LINES_IN_DICTIONARY; X++ )
   {
      size_t wordLength = strlen(LexiconInRam[X]);
      if (wordLength > 0)
      {
         // As convoluted as this command may appear, it simply copies a string from its temporary ram location to the array of length equivelant strings for adding to the intermediate "Trie".
         if ( wordLength <= MAX_WORD_LENGTH )
         {
             strcpy(&((AllWordsInEnglish[wordLength])[(WordLengthTracker[wordLength]*(wordLength + 1))]), LexiconInRam[X]);
         }
         WordLengthTracker[wordLength] += 1;
      }

   }
   DEBUGLOG(DEBUG2,("The words have now been sorted by length\n"));

   // Make sure that the counting has resulted in all of the strings being placed correctly.
   for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
   {
      if ( numberOfWordsOfLength[X] != WordLengthTracker[X] )
      {
         fprintf(stderr, "The number of words %d vs %d for %d is not adding up properly, so fix it.\n",numberOfWordsOfLength[X], WordLengthTracker[X], X);
         exit(-1);
      }
   }


   // Free the initial ram read space
   for ( unsigned int X = 0; X < NUMBER_OF_LINES_IN_DICTIONARY; X++ )
      free(LexiconInRam[X]);
   free(LexiconInRam);

   DEBUGLOG(DEBUG1,("The Dawg init function will now be run, so be patient, it will take some time to complete.\n"));
   dawgPtr dawg = DawgFromRAMDictionary(AllWordsInEnglish, numberOfWordsOfLength);

   // Free up the array that holds the uncompressed English language.
   for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
      if ( AllWordsInEnglish[X] != NULL )
         free(AllWordsInEnglish[X]);

   return dawg;

}

#pragma mark - ENTRY HolderOfAllTnodesFromBinaryFile

TnodePtr *HolderOfAllTnodesFromBinaryFile(FILE *in, unsigned int *node_count)
{

   // The first line is the node_counts
   if (fread(node_count, sizeof(node_count),1, in) != 1)
   {
      fprintf(stderr ,"Could not read node_count on first line\n");
      exit(-1);
   }
   DEBUGLOG(DEBUG2,("\nNode count read is %d.\n", *node_count));

   DEBUGLOG(DEBUG1,("\nStep 2 - Create the intermeriary dawg structures.\n"));


   // Allocate a one dimensional array for all TnodePtrs
   TnodePtr *HolderOfAllTnodePointers = (TnodePtr *)malloc(*node_count * sizeof(TnodePtr));
   memset(HolderOfAllTnodePointers, 0, *node_count * sizeof(TnodePtr));

   // Allocate each TnodePtr to be filled in.
   for (int X=0; X<=*node_count; X++)
      HolderOfAllTnodePointers[X] = TnodeInit(32, NULL, FALSE, 0, 0, NULL, FALSE);

   // The Globals will be recomputed from the dawg.
   MAX_WORD_LENGTH = 0;
   NUMBER_OF_LINES_IN_DICTIONARY = 0;



   DEBUGLOG(DEBUG1,("\nStep 3 - Reading Tnode Holder from binary file.\n"));


   // Fill in each Tnode from the file.
   //addBinaryNodesToDawg_recursive(HolderOfAllTnodePointers, &NUMBER_OF_LINES_IN_DICTIONARY, in, node_count);
   int nodeCounter=1;
   unsigned int encodedValue;
   TnodePtr parent = NULL;

   while (fread(&encodedValue, sizeof(encodedValue),1, in) != 0)
   {
      TnodePtr node = HolderOfAllTnodePointers[nodeCounter];

      unsigned char Letter = ((encodedValue & LETTER_BIT_MASK)>>LETTER_BIT_SHIFT) + 'A';
      node->Letter = Letter;

       Bool eow = (encodedValue & END_OF_WORD_BIT_MASK)? TRUE: FALSE;
       node -> EndOfWordFlag = eow;

       unsigned int child_offset = encodedValue & CHILD_INDEX_BIT_MASK;
       TnodePtr child = NULL;
       if ( child_offset > *node_count )
       {
          // The offset cannot exceed the limit
          fprintf(stderr, "Error. Child offset %d exceeds limit of %d\n",child_offset, *node_count);
          exit (-1);
       } else if ( child_offset == 0 )
       {
          // If there is no child offset, There is no child.
          child = NULL;

       } else
       {
          child = HolderOfAllTnodePointers[child_offset];
          DEBUGLOG(DEBUG3,("ofset %d %c\n", child_offset, TNODE_LETTER(node)));
       }
       node->Child = child;


       node-> Parent = parent;
       parent = node; // for next time
       node-> UniqueID = (int) nodeCounter;
       node-> ArrayIndex = (int) nodeCounter;

       Bool nextFlag = (encodedValue & END_OF_LIST_BIT_MASK) ? FALSE: TRUE;

       // In the case where the encoded value is null, then there is no next Flag. This is true for the root node.
       if (encodedValue == 0)
          nextFlag = FALSE;

       TnodePtr next = NULL;

       if (nextFlag == TRUE)
          next = HolderOfAllTnodePointers[nodeCounter + 1];

       node->Next = next;

       nodeCounter++;
   }

   // We would have incremented one to many
   nodeCounter --;

   DEBUGLOG(DEBUG1,("\nStep 4 - Checking node count for equivalancy.\n"));

   // For the binary file, it is important that the number of nodes read in equal the count it said it was.
   if ( nodeCounter != *node_count )
   {
      fprintf(stderr, "Number of lines read in (%u) not equal to node_count (%d) in file", nodeCounter, *node_count);
      exit (-1);
   } else
   {
      DEBUGLOG(DEBUG2,("Number of lines matches %d\n", *node_count));
   }

   return HolderOfAllTnodePointers;

}

char ** dictionaryFromHolderOfAllTnodes_recursive(TnodePtr *HolderOfAllTnodes, TnodePtr node, char *acc, int depth, char **Dictionary, unsigned int node_count, FILE *out)
{

   if (node->EndOfWordFlag)
   {
      acc[depth] = TNODE_LETTER( node);
      acc[depth+1] = '\0';
      NUMBER_OF_LINES_IN_DICTIONARY ++;
      size_t length = strlen(acc);
      DEBUGLOG(DEBUG2, ("word %s\n",acc));

      if (out == stdout)
      {
         fprintf(out, "%s\n", acc);
      } else
      {
         if (Dictionary == NULL)
         {
            Dictionary = (char**)malloc(sizeof(*Dictionary) * NUMBER_OF_LINES_IN_DICTIONARY);
         } else
         {
            Dictionary = (char **)realloc(Dictionary, sizeof(*Dictionary) * NUMBER_OF_LINES_IN_DICTIONARY);
         }
         Dictionary[NUMBER_OF_LINES_IN_DICTIONARY-1] = calloc((length +1), sizeof(char));
         strcpy(Dictionary[NUMBER_OF_LINES_IN_DICTIONARY-1], acc);
      }



      if (length > MAX_WORD_LENGTH)
      {
         MAX_WORD_LENGTH = (int) length;
         DEBUGLOG(DEBUG2,("Changing MAX_WORD_LENGTH:%zu for word: '%s'\n", length, acc));
      }
   }

   TnodePtr child = TNODE_CHILD( node );
   if (child)
   {
      acc[depth] = TNODE_LETTER( node );

      DEBUGLOG(DEBUG3,("node c %d count %d\n", node->ArrayIndex, node_count));

      if (node->ArrayIndex == 0 || node->ArrayIndex > node_count)
      {
         printf ("OOPS\n");
      } else
      {
         Dictionary = dictionaryFromHolderOfAllTnodes_recursive(HolderOfAllTnodes, child, acc, depth +1, Dictionary, node_count, out);
      }

   }
   TnodePtr next = TNODE_NEXT( node );
   if (next)
   {
      DEBUGLOG(DEBUG3,("node n %d %d\n", node->ArrayIndex, node_count));
      if (node->ArrayIndex == 0 || node->ArrayIndex > node_count)
      {
         printf ("OOPS\n");
      } else
      {
         DEBUGLOG(DEBUG3,("node# %d letter %c\n", TNODE_UNIQUEID(next), TNODE_LETTER(next)));

         Dictionary =  dictionaryFromHolderOfAllTnodes_recursive(HolderOfAllTnodes, next, acc, depth, Dictionary, node_count, out);
      }

   }

   return Dictionary;
}

#pragma mark - ENTRY dictionaryFromHolderOfAllTnodes

char **dictionaryFromHolderOfAllTnodes(TnodePtr *HolderOfAllTnodes, unsigned int node_count, FILE *out)
{

   // As unravelling the dawg progresses, the words are built in the accumulator.
   char acc[INPUT_LIMIT +1];
   memset(&acc, 0, sizeof(acc));

   // This will be recompited
   NUMBER_OF_LINES_IN_DICTIONARY = 0;
   MAX_WORD_LENGTH = 0;


   TnodePtr node = HolderOfAllTnodes[1];

   DEBUGLOG(DEBUG1,("\nStep 5 - Converting HolderOfAllTnodes to Dictionary.\n"));
   char **Dictionary = NULL;

   Dictionary = dictionaryFromHolderOfAllTnodes_recursive(HolderOfAllTnodes, node, acc, 0, Dictionary, node_count, out);

   return Dictionary;
}


#pragma mark - ENTRY for Dictionary From Binary File

char ** dictionaryFromBinaryFile(FILE *in, unsigned int  *node_count, int **numberOfWordsOfLength, FILE *out)
{

   // First convert the Binary file to a list of Tnodes
   TnodePtr *HolderOfAllTnodes = HolderOfAllTnodesFromBinaryFile(in, node_count);

   // Get the word list from the list of all Tnodes.
   // Hey, Why not convert the TNodes to a dawg you say?.  Well you can't because
   // this HolderOfAllTnodes is a one dimensional array while the dawg has a two
   // dimensional array. Even if you extract the node_number_count_for_wordlengths
   // to try and construct the two dimensional array, you still cannot because
   // the order of the Tnodes do not represent the original order and size.
   // So by converting it to a word list and then to a dawg, we know that works
   // as it is supposed to.
   char **dictionary = dictionaryFromHolderOfAllTnodes(HolderOfAllTnodes, *node_count, out);

   if (out == stdout)
   {
      // noop
   } else
   {
      DEBUGLOG(DEBUG1,("\nStep 6 - Creating WordsOfLength from Dictionary.\n"));

      // Create the associated pointer array with all the nodes inside it.
      *numberOfWordsOfLength = (int*)calloc( MAX_WORD_LENGTH +1, sizeof(int) );

      for (unsigned int X = 0; X < NUMBER_OF_LINES_IN_DICTIONARY; X++)
      {
          size_t length = strlen(dictionary[X]);
          (*numberOfWordsOfLength)[length] = (*numberOfWordsOfLength)[length] + 1;
      }

       int wordCountCheck = 0;
       for (unsigned int X = 0; X <= MAX_WORD_LENGTH;X++)
       {
          wordCountCheck +=  (*numberOfWordsOfLength)[X];
       }

       DEBUGLOG(DEBUG2,("\nThe Total Number Of words in the list = |%d|\n", wordCountCheck));

       DEBUGLOG(DEBUG1,("\nStep 7 - Checking word count for equivalancy.\n"));


       if (wordCountCheck == NUMBER_OF_LINES_IN_DICTIONARY)
       {
          DEBUGLOG(DEBUG2,("\nThe Total Number of words in the list matches.\n"));
       } else
       {
          fprintf(stderr, "Number of words %d != number of lines in dictionary %u.", wordCountCheck, NUMBER_OF_LINES_IN_DICTIONARY);
          exit (-1);
       }
   }
   return dictionary;

}


dawgPtr dawgFromBinaryFile(FILE * in)
{
   unsigned int node_count =0;
   int *numberOfWordsOfLength = NULL;

   char **Dictionary = dictionaryFromBinaryFile(in, &node_count, &numberOfWordsOfLength,stdout);

   return DawgFromRAMDictionary(Dictionary, numberOfWordsOfLength);
}

#pragma mark - ENTRY Dictionary to Word File

void dictionaryToWordFile(char** dictionary, FILE *out)
{
   for (unsigned int X = 0; X < NUMBER_OF_LINES_IN_DICTIONARY; X++)
   {
      fprintf(out, "%s\n",dictionary[X]);
   }
}

dawgPtr old_dawgFromBinaryFile(FILE * in)
{

   //dawg * dawg = calloc(1, sizeof(struct dawg *));
   dawgPtr dawg = (dawgPtr) malloc(1*sizeof(struct dawg_struct));
   dawg->node_count =0;
   dawg->node_number_count_for_wordlength = NULL;


   DEBUGLOG(DEBUG1,("\nStep 1 - Reading node count of binary file.\n"));

   // The first line is the node_counts
   if (fread(&dawg->node_count, sizeof(dawg->node_count),1, in) != 1)
   {
      fprintf(stderr ,"Could not read node-count on first line\n");
      exit(-1);
   }
   DEBUGLOG(DEBUG2,("\nNode count read is %d.\n", dawg->node_count));

   DEBUGLOG(DEBUG1,("\nStep 2 - Create the dawg structures.\n"));


   // Allocate a one dimensional array for all TnodePtrs
   TnodePtr *HolderOfAllTnodePointers = (TnodePtr *)malloc(dawg->node_count * sizeof(TnodePtr));
   memset(HolderOfAllTnodePointers, 0, dawg->node_count * sizeof(TnodePtr));

   // Allocate each TnodePtr to be filled in.
   for (unsigned int X = 0; X <= dawg->node_count; X++)
      HolderOfAllTnodePointers[X] = TnodeInit(32, NULL, FALSE, 0, 0, NULL, FALSE);

   // The Globals will be recomputed from the dawg.
   MAX_WORD_LENGTH = 0;
   NUMBER_OF_LINES_IN_DICTIONARY = 0;

   //dawg->root = (TnodePtr **)HolderOfAllTnodePointers;



   // There is no possible way to know what it will be, but it cannot exceed INPUT_LIMIT
   // Create the associated pointer array with all the nodes inside it.
   dawg->node_number_count_for_wordlength =  (int*)malloc((INPUT_LIMIT)*sizeof(int) );

   // initial all the counters for word lengths
   for ( int X = 0; X < MAX_WORD_LENGTH; X++ )
      dawg->node_number_count_for_wordlength[X] = 0;

   DEBUGLOG(DEBUG1,("\nStep 3 - Reading Dawg from binary file.\n"));


    // Fill in each Tnode from the file.
   //addBinaryNodesToDawg_recursive(HolderOfAllTnodePointers, &NUMBER_OF_LINES_IN_DICTIONARY, in, node_count);
   int nodeCounter=0;
   unsigned int encodedValue;
   TnodePtr parent = NULL;

   while (fread(&encodedValue, sizeof(encodedValue),1, in) != 0)
   {
      TnodePtr node = HolderOfAllTnodePointers[nodeCounter];

      unsigned char Letter = ((encodedValue & LETTER_BIT_MASK)>>LETTER_BIT_SHIFT) + 'A';
      node->Letter = Letter;

      Bool eow = (encodedValue & END_OF_WORD_BIT_MASK)? TRUE: FALSE;
      node -> EndOfWordFlag = eow;

      unsigned int child_offset = encodedValue & CHILD_INDEX_BIT_MASK;
      TnodePtr child = NULL;
      if ( child_offset > dawg->node_count )
      {
         // The offset cannot exceed the limit
         fprintf(stderr, "Error. Child offset %d exceeds limit of %d\n",child_offset, dawg->node_count);
         exit (-1);
      } else if ( child_offset == 0 )
      {
         // If there is no child offset, There is no child.
         child = NULL;

      } else
      {
          child = HolderOfAllTnodePointers[child_offset];
          DEBUGLOG(DEBUG3,("ofset %d %c\n", child_offset, TNODE_LETTER(node)));
      }
      node->Child = child;


      node-> Parent = parent;
      parent = node; // for next time
      node-> UniqueID = (int) nodeCounter;
      node-> ArrayIndex = (int) nodeCounter;

      Bool nextFlag = (encodedValue & END_OF_LIST_BIT_MASK) ? FALSE: TRUE;

      // In the case where the encoded value is null, then there is no next Flag. This is true for the root node.
      if (encodedValue == 0)
         nextFlag = FALSE;

      TnodePtr next = NULL;

      if (nextFlag == TRUE)
         next = HolderOfAllTnodePointers[nodeCounter + 1];

      node->Next = next;

      nodeCounter++;
      //dawg->node_count = dawg->node_count +1;
   }

   // We would have incremented one to many
   //dawg->node_count = dawg->node_count -1;
   nodeCounter --;

   DEBUGLOG(DEBUG1,("\nStep 4 - Checking node count for equivalancy.\n"));

   // For the binary file, it is important that the number of nodes read in
   // equal the count it said it was.
   if ( nodeCounter != dawg->node_count )
   {
      fprintf(stderr, "Number of lines read in (%u) not equal to node_count (%d) in file", nodeCounter, dawg->node_count);
      exit (-1);
   } else
   {
      DEBUGLOG(DEBUG2,("Number of lines matches %d\n", dawg->node_count));
   }

   dawg->root = (TnodePtr **) HolderOfAllTnodePointers;

   return dawg;
}


void dawgToWordFile_recursive(dawgPtr dawg, TnodePtr node, FILE * out, char * acc, int depth)
{
   if (node->EndOfWordFlag)
   {
      acc[depth] = TNODE_LETTER( node);
      acc[depth+1] = '\0';
      fprintf(out, "%s\n", acc);

      // To complete the dawg, increase the counter for this words word length.
      size_t length = strlen(acc);
      dawg->node_number_count_for_wordlength[length] = dawg->node_number_count_for_wordlength[length] +1;

      if (length > MAX_WORD_LENGTH)
      {
         MAX_WORD_LENGTH = (int) length;
         DEBUGLOG(DEBUG2,("Changing MAX_WORD_LENGTH:%zu for word: '%s'\n", length, acc));
      }

      NUMBER_OF_LINES_IN_DICTIONARY ++;


   }

   TnodePtr child = TNODE_CHILD( node);
   if (child)
   {
      acc[depth] = TNODE_LETTER( node);

      dawgToWordFile_recursive(dawg, child, out, acc, depth+1);
   }
   TnodePtr next = TNODE_NEXT( node );
   if (next)
   {
      DEBUGLOG(DEBUG3,("node# %d letter %c\n", TNODE_UNIQUEID(next), TNODE_LETTER(next)));

      dawgToWordFile_recursive(dawg, next, out, acc, depth);

   }
}



void dawgToWordFile(dawgPtr dawg, FILE * out)
{
   // As unravelling the dawg progresses, the words are built in the accumulator.
   char acc[INPUT_LIMIT +1];
   memset(&acc, 0, sizeof(acc));

    // This will be recomputed
   NUMBER_OF_LINES_IN_DICTIONARY = 0;
   MAX_WORD_LENGTH = 0;

   DEBUGLOG(DEBUG1,("\nStep 5 - Writing placeholder for real word count.\n"));
   fprintf(out,"%09u\n", NUMBER_OF_LINES_IN_DICTIONARY);

   TnodePtr node = (TnodePtr) dawg->root[1];

   DEBUGLOG(DEBUG1,("\nStep 6 - Converting word list to Dawg.\n"));

   dawgToWordFile_recursive(dawg, node, out, acc, 0);

   unsigned int wordCountCheck = 0;
   for (unsigned int X=0; X<=MAX_WORD_LENGTH;X++)
   {
      wordCountCheck += dawg->node_number_count_for_wordlength[X];
   }

   DEBUGLOG(DEBUG2,("\nThe Total Number Of words in the list = |%d|\n", wordCountCheck));

   DEBUGLOG(DEBUG1,("\nStep 7 - Checking word count for equivalancy.\n"));


   if (wordCountCheck == NUMBER_OF_LINES_IN_DICTIONARY)
   {
      DEBUGLOG(DEBUG2,("\nThe Total Number of words in the list matches.\n"));
   } else
   {
      fprintf(stderr, "Number of words %d != number of lines in dictionary %u.", wordCountCheck, NUMBER_OF_LINES_IN_DICTIONARY);
      exit (-1);
   }

   DEBUGLOG(DEBUG1,("\nStep 8 - Writing real word count at top of file.\n"));
   if (out != stdout)
   {
      rewind(out);
   }
   fprintf(out,"%09u", NUMBER_OF_LINES_IN_DICTIONARY);

   DEBUGLOG(DEBUG1,("\nStep 9 - ReArranging DawgPtr based on wordLengths.\n"));

   TnodePtr * HolderOfAllTnodePointers = (TnodePtr *) dawg->root ;


   dawg->root = (TnodePtr **)malloc(MAX_WORD_LENGTH * sizeof(TnodePtr*));



   for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
   {
      dawg->root[X] = (TnodePtr *)malloc((dawg->node_number_count_for_wordlength[X] +1)*sizeof(TnodePtr));
   }

   int index =0;
   for (int X = (MAX_WORD_LENGTH - 1); X >= 0; X-- )
   {
      int wordLength = dawg->node_number_count_for_wordlength[X];

      for (int W = 0; W < wordLength; W++)
      {
         unsigned int c;
         c = wordLength * W +X; // 118818
         //c = wordLength * X + W; // 109577

         if (c > dawg->node_count)
         {
            dawg->root[X][W] = NULL;
            continue;
         }

         dawg->root[X][W] = HolderOfAllTnodePointers[index];
         index++;
      }
   }
   /*
   for ( int X = 0; X <= MAX_WORD_LENGTH; X++ )
   {

      int wordLength = dawg->node_number_count_for_wordlength[X];

      // Move through the holder array from the beginning.
      for ( int Y = (wordLength -1); Y >= 0; Y-- )
      {
         unsigned int c = wordLength * X +Y;

         if (c > dawg->node_count)
         {
            dawg->root[X][Y] = NULL;
            continue;
         }

         dawg->root[X][Y] = HolderOfAllTnodePointers[wordLength * X +Y];
      }
   }
   */

   DEBUGLOG(DEBUG1,("\nCompleted Extracting Dawg from Binary File.\n"));

}


#pragma mark - The "Main()" function just runs the show



int main(int argc, char* argv[])
{

   // This appears because debug level not set yet and default is 1.
   DEBUGLOG(DEBUG1,("The main Debug Level 1 function lives\n"));
   DEBUGLOG(DEBUG2,("The main Debug Level 2 function lives\n"));
   DEBUGLOG(DEBUG3,("The main Debug Level 3 function lives\n"));

   // Input files.
   char *inputFileNames[argc +1];
   char *outputFileNames[argc+1];
   int commands[argc+1];
   char *outputFileName = NULL;

   // Stupid check for static analyzer
   if (argc <=1 )
   {
      usage("UnScramble");
   } else
   {
      // Marker of what was last requested to be done.
      int argCounter;

      for (argCounter =0; argCounter <= argc; argCounter++)
      {
         inputFileNames[argCounter] =  NULL;
         outputFileNames[argCounter] = NULL;
         commands[argCounter] = 0;
      }

      argCounter = 0;
      int last = -1;
      int opt;
      while ((opt=getopt(argc, argv, ":hc:d:e:g:X:o:")) != -1)
      {
         switch (opt)
         {
            case 'h': // help
            {
               usage(argv[0]);
               exit(0);
            }
            case 'c':
            case 'e':
            case 'g':
            case 'X':
            {
               if ( opt == 'c' && optarg == NULL )
               {
                  fprintf(stderr, "No file given to compile\n");
                  exit ( -1 );
               }

               if ( (opt == 'e' || opt == 'g' ) &&
                    optarg != NULL &&
                    optarg[0] == '-')
               {
                   // No option specified, which is okay
               } else
               {
                      inputFileNames[argCounter] = optarg;
               }


               if (outputFileName != NULL)
               {
                  outputFileNames[argCounter] = outputFileName;
                  outputFileName = NULL;
                  last = -1;
               } else
               {
                  last = argCounter;
               }

               commands[argCounter] = opt;

               argCounter ++;

               break;
            }
            case 'd': // debug level
            {
               if (optarg == NULL)
               {
                  fprintf(stderr, "No debug level given.\n");
                  exit(-1);
               }

               commands[argCounter] = opt;
               outputFileNames[argCounter] = optarg;
               argCounter ++;

               break;
            }

            case 'o':
            {
               if (optarg == NULL)
               {
                  fprintf(stderr, "No output file given.\n");
                  exit(-1);
               }

               if (last < 0)
               {
                  outputFileName = optarg;
               } else if (last < argc)
               {
                  outputFileNames[last] = optarg;
                  outputFileName = NULL;
                  last = -1;
               } else
               {
                  fprintf(stderr, "Cannot assign '%s' to any command\n", optarg);
               }

               break;
            }
            default:
            {
               fprintf(stderr, "Unknown option %c\n", opt);
               exit(-1);
            }
         }
      }
      DEBUGLOG(DEBUG1,("Creating DAWG. This may take a while ...\n"));

      dawgPtr dawg = NULL;
      for ( int argIndex =0; argIndex <= argCounter; argIndex++ )
      {
         switch ( commands[argIndex] )
         {
            case 'c': // COMPILE
            {
               if ( inputFileNames[argIndex] == NULL )
               {
                  fprintf(stderr, "Oops, no input file name stored\n");
                  exit( -1 );
               }

               FILE *in = fopen(inputFileNames[argIndex], "r");

               if ( in == NULL )
               {
                  fprintf(stderr, "Could not open '%s' for reading\n", inputFileNames[argIndex]);
                  exit( -1 );
               }

               FILE *out = stdout;
               if ( outputFileNames[argIndex] != NULL )
               {
                  out = fopen(outputFileNames[argIndex], "wb");

                  if ( out == NULL )
                  {
                     fprintf(stderr, "Could not open '%s' for writing\n", outputFileNames[argCounter]);
                     exit( -1 );
                  }
               }
               freeTheDawg(dawg);
               dawg = dawgFromWordFile(in);
               fclose(in);
               writeDawgToBinaryFile(dawg, out, FALSE);

               if ( out != stdout )
               {
                  fclose (out);
               }
               break;
            }
            case 'e': // EXTRACT
            {
               FILE *out = stdout;
               if (outputFileNames[argIndex] !=NULL)
               {
                  out = fopen(outputFileNames[argIndex], "w");

                  if ( out == NULL )
                  {
                     fprintf(stderr, "Could not open '%s' for writing\n", outputFileNames[argIndex]);
                     exit( -1 );
                  }
               }

               if ( inputFileNames[argIndex] == NULL)
               {
                  fprintf(stderr, "Oops, no input file extract\n");
                  exit( -1 );
               }

               FILE *in = NULL;
               in = fopen(inputFileNames[argIndex], "r");
               if (in == NULL)
               {
                  fprintf(stderr, "Could not open '%s' for reading\n", inputFileNames[argCounter]);
                  exit( -1 );
               }
               unsigned int node_count= 0;
               int *numberOfWordsOfLength = NULL;

               char **dictionary = dictionaryFromBinaryFile(in, &node_count, &numberOfWordsOfLength, out);

               //fclose(in);



               if ( out != stdout )
               {
                  dictionaryToWordFile(dictionary, out);
                  fclose (out);
               }
               break;
            }
            case 'X': // Xperimental
            {
               // Why is this experimental.  it was proof that extracting a dawg from a binary compressed dawg is not possible.
               FILE *out = stdout;
               if (outputFileNames[argIndex] !=NULL)
               {
                  out = fopen(outputFileNames[argIndex], "w");

                  if ( out == NULL )
                  {
                     fprintf(stderr, "Could not open '%s' for writing\n", outputFileNames[argIndex]);
                     exit( -1 );
                  }
               }

               if ( inputFileNames[argIndex] == NULL && dawg == NULL )
               {
                  fprintf(stderr, "Oops, no input file or dawg to extract\n");
                  exit( -1 );
               }

               FILE *in = NULL;
               if ( inputFileNames[argIndex] != NULL )
               {
                  in = fopen(inputFileNames[argIndex], "r");

                  if (in == NULL)
                  {
                     fprintf(stderr, "Could not open '%s' for reading\n", inputFileNames[argCounter]);
                     exit( -1 );
                  }
                  freeTheDawg(dawg);
                  dawg = dawgFromBinaryFile(in);

                  fclose(in);
               }

               if ( dawg == NULL )
               {
                  fprintf(stderr, "No Dawg to extract\n");
                  exit( -1 );
               }


               dawgToWordFile(dawg, out);

               if ( out != stdout )
               {
                  fclose (out);
               }
               break;
            }
            case 'g': // GRAPH
            {
               FILE *out = stdout;
               if ( outputFileNames[argIndex] != NULL )
               {
                  out = fopen(outputFileNames[argIndex], "w");

                  if ( out == NULL )
                  {
                     fprintf(stderr, "Could not open '%s' for writing\n", outputFileNames[argIndex]);
                     exit( -1 );
                  }
               }

               if ( inputFileNames[argIndex] == NULL && dawg == NULL )
               {
                  fprintf(stderr, "Oops, no input file or dawg to graph\n");
                  exit( -1 );
               }

               FILE *in = NULL;
               if ( inputFileNames[argIndex] != NULL )
               {
                  in = fopen(inputFileNames[argIndex], "r");

                  if ( in == NULL )
                  {
                     fprintf(stderr, "Could not open '%s' for reading\n", inputFileNames[argIndex]);
                     exit( -1 );
                  }
                  freeTheDawg(dawg);
                  dawg = dawgFromWordFile(in);
                  fclose(in);
               }

               if (dawg == NULL)
               {
                  fprintf(stderr, "No Dawg to graph\n");
                  exit( -1 );
               }

               // The TRUE is actually to do a graph.
               writeDawgToBinaryFile(dawg, out, TRUE);

               if ( out != stdout )
               {
                  fclose (out);
               }
               break;
            }
            case 'd': // DEBUG LEVEL
            {
               if ( outputFileNames[argIndex] == NULL )
               {
                  fprintf(stderr, "No debug level stored for index %d\n", argIndex);
                  exit( -1 );
               }

               debugLevel = stringToUnsignedInt(outputFileNames[argIndex]);

               break;
            }
            default:
               // It is totally possible that there is no command at this argIndex
               break;
         }
      }

      freeTheDawg(dawg);
   }
   exit(0);
}
