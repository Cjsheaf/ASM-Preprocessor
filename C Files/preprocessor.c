/**
 * Finds all MIPS macros and functions in multiple .asm source files, and prepends them
 * to a single primary .asm file ready for compilation and linking with the MARS emulator.
 */
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/**
 * Define all ants:
 */
#define true 1
#define false 0

#define defaultOutputFileName "a.asm"

typedef struct {
	char* fileName; //Name of the file to be substituted at position
} IncludeStatement;

/**
 * Prototype all functions:
 */
void interpretConsoleFlags(int argc, char* argv[], char** primaryFileName, char** outputFileName);
void printDebug(char* debugText, ...); /**Wraps printf(), calling it only if global variable debugFlag is true**/
char* readTextFile(char* fileName); /**Allocates space for and returns the contents of fileName**/
void writeTextFile(const char* fileName, char* text);
void splitString(char* input, char* delimiter, char** leftSubString, char** rightSubString); /**With "input" as the original string, copies all characters left and right of "delimiter" to "leftSubString" and "rightSubString", respectively.**/
char* replaceIncludeStatements(char* primaryText); /**Returns a copy of primaryText with all "@Include" statements replaced with the body of the file they correspond to**/
char* findInclude(char* text); /**Utility-function of replaceIncludeStatements(); Searches fileText for the first instance of "@Include <*>" with * being a file name, and returns a copy of it. Otherwise returns NULL**/
char* getIncludedFileText(char* includeStatement); /**Utility-function of replaceIncludeStatements(); Matches the included file name with the appropriate otherFileName and returns the otherFileName's text**/
int getLineNumber(char* text, char* position); /**Returns the line number of the given position in the String named text**/

/**
 * Declare any (absolutely necessary) global variables:
 */
int debugFlag = false; 

int main(int argc, char* argv[]) {
	char* primaryFileName = NULL; //The file to have all macros and functions prepended to. Is not modified.
	char* primaryText = NULL;
	char* outputFileName = defaultOutputFileName; //The resulting file after prepending
	char* outputText;
	
	//Interpret optional command-line flags, modifying the relevant variables as appropriate:
	interpretConsoleFlags(argc, argv, &primaryFileName, &outputFileName);
	if(primaryFileName == NULL) {
		printf("ERROR: No input file name specified! Use '-i filename' to specify an input file name.\n");
		exit(1);
	}
	printDebug("The primary file is named: \"%s\"\n", primaryFileName);
	printDebug("The output file will be named: \"%s\"\n", outputFileName);
	
	printDebug("\n >Reading Files...\n");
	primaryText = readTextFile(primaryFileName);
	printDebug("All Files Read Successfully.\n");
	
	//Search primaryText for "@include <*>". If found, replace it with the contents of the file name indicated in "<*>":
	outputText = replaceIncludeStatements(primaryText);
	
	writeTextFile(outputFileName, outputText);
	
	printDebug("\n >Freeing Memory...\n");
	free(primaryText);
	free(outputText);
	printDebug("Memory freed successfully.\n");
	
	return 0;
}

void interpretConsoleFlags(int argc, char* argv[], char** primaryFileName, char** outputFileName) {
	if(argc > 1) { //If the user entered any of the optional flags
		for(int i = 1; i < argc; i++) {
			if(strcmp(argv[i], "-i") == 0) { //Specifies the input (primary) file name which will be prepended to
				*primaryFileName = argv[i + 1];
			} else if(strcmp(argv[i], "-o") == 0) { //Specifies the output file name
				*outputFileName = argv[i + 1];
			} else if(strcmp(argv[i], "-d") == 0) { //If the user enabled the verbose debug messages
				debugFlag = true;
			} else if(*primaryFileName == NULL) { //If the first argument is not part of a flag, assume it's the input file name
				*primaryFileName = argv[i];
			}
		}
	}
}

void printDebug(char* debugText, ...) { /**Wraps printf(), calling it only if global variable debugFlag is true**/
	if(debugFlag == true) {
		va_list args; //Set up our variable argument's data structure
		va_start(args, debugText); //The variable argument "starts" right after our last finite argument, which is *debugText
		
		char* character = "%";
		
		char* subString = NULL;
		char* position = debugText; //This variable will point to characters in the middle of debugText, serving as the start of any remaining unprocessed text
		int subStrLength = 0;
		
		while(position[0] != '\0') {
			//Get the length of the string preceding the character '%' if there is one, else the total length remaining:
			subStrLength = strcspn(position, character);
			
			//Copy the contents of the sub-string we just found into a buffer, which will be printed later
			subString = malloc(sizeof(char) * subStrLength + 1);
			strncpy(subString, position, subStrLength);
			subString[subStrLength] = '\0';
			
			//Point position to the first character after the sub-string we just found:
			position += subStrLength;
			
			printf(subString);
			
			//If a print directive is present, determine its type, print the appropriate variable argument, and consume the directive:
			if(position[0] == '%') { 
				if(position[1] == 'c') {
					printf("%c", va_arg(args, unsigned int)); //For %c
					position += 2;
				} else if(position[1] == 's') {
					printf("%s", va_arg(args, unsigned int)); //For %s
					position += 2;
				} else if(position[1] == 'd') {
					printf("%d", va_arg(args, signed int*)); //For %d
					position += 2;
				} else if(position[1] == 'p') {
					printf("%p", va_arg(args, void*)); //For %p
					position += 2;
				} else {//This accounts for an edge case where a string contains a % with no valid following type character
					position++; //Skip the lone %
				}
			}
		}
		
		va_end(args); //Clean up the variable argument's data structure
	}
}

char* readTextFile(char* fileName) { /**Allocates space for and returns the contents of fileName**/
	FILE* textFile = NULL;
	int fileLength = 0; //In bytes
	char* textBuffer = NULL;
	
	textFile = fopen(fileName, "rb");
	if (textFile == NULL) {
		printf("\nERROR: File \"%s\" not found!\n", fileName);
		exit(1);
	}
	
	fseek(textFile, 0, SEEK_END); //Set the file position indicator to the end of the file
	fileLength = ftell(textFile); //get the value of the file position indicator, which now represents the length of the file
	
	textBuffer = malloc(fileLength * sizeof(char) + 1); //Allocate enough buffer space for the text file, plus a terminating zero
	if (textBuffer == NULL) {
		fclose(textFile);
		exit(1);
	}
	
	fseek(textFile, 0, SEEK_SET); //Set the file position indicator back to the beginning of the file
	fread(textBuffer, sizeof(char), fileLength, textFile);
	textBuffer[fileLength] = '\0';
	
	fclose(textFile);
	printDebug("File \"%s\" read successfully.\n", fileName);
	
	return textBuffer;
}

void writeTextFile(const char* fileName, char* text) {
	FILE* outputFile = NULL;
	int i;
	
	outputFile = fopen(fileName, "wb"); //Open the file for [w]riting in [b]inary mode (creating it if it doesn't yet exist)
	if(outputFile == NULL) { //If for some reason opening the file fails, exit immediately with a failure
		printf("\nERROR: File \"%s\" not found!\n", fileName); //<DEBUG>
		exit(1);
	} else {
		printDebug("File opened for writing successfully.\n"); //<DEBUG>
	}
	
	for(i = 0; text[i] != '\0'; i++) {
		fputc(text[i], outputFile);
	}
	
	if (ftell(outputFile) != strlen(text)) { //If not all of the text was written to the file, an error has occurred
		printf("\nERROR: Not all characters were written to file!");
		fclose(outputFile);
		exit(1);
	} else {
		printDebug("Output written to file successfully.\n");
	}
	
	fclose(outputFile);
}

void splitString(char* input, char* delimiter, char** leftSubString, char** rightSubString) { /**With "input" as the original string, copies all characters left and right of "delimiter" to "leftSubString" and "rightSubString", respectively.**/
	char* position;
	int leftSubStrLength;
	int rightSubStrLength;
	
	position = strstr(input, delimiter);
	if (position == NULL) {
		printf("ERROR: delimiter \"%s\" was not found while trying to split input string!\n", delimiter);
		exit(1);
	}
	
	leftSubStrLength = position - input; //Difference of the addresses gives us the length, if any
	printDebug("Left substring length = %d\n", leftSubStrLength);
	rightSubStrLength = strlen(input) - strlen(delimiter) - leftSubStrLength;
	printDebug("Right substring length = %d\n", rightSubStrLength);
	
	//Copy the sub-strings to the appropriate argument pointers:
	free(*leftSubString);
	if(leftSubStrLength > 0) {
		*leftSubString = malloc(sizeof(char) * leftSubStrLength + 1);
		strncpy(*leftSubString, input, leftSubStrLength);
		(*leftSubString)[leftSubStrLength] = '\0';
	} else {
		*leftSubString = NULL;
	}
	
	free(*rightSubString);
	if(rightSubStrLength > 0) {
		*rightSubString = malloc(sizeof(char) * rightSubStrLength + 1);
		strncpy(*rightSubString, input + leftSubStrLength + strlen(delimiter), rightSubStrLength);
		(*rightSubString)[rightSubStrLength] = '\0';
	} else {
		*rightSubString = NULL;
	}
}

char* replaceIncludeStatements(char* primaryText) { /**Returns a copy of primaryText with all "@Include" statements replaced with the body of the file they correspond to**/
	int combinedFileSize = 0;
	char* includeStatement = NULL;
	char* leftBuffer = NULL; //Text left of the "@Include" statement
	char* rightBuffer = NULL; //Text right of the "@Include" statement
	char* includedTextBuffer = NULL; //Text retrieved from the file named in the "@Include" statement
	char* combinedText = primaryText;
	
	printDebug(" \n>Finding include statements in the provided files...\n");
	while((includeStatement = findInclude(combinedText)) != NULL) { //While there are "@Include" statements remaining:
		/**Split the file around the include statement, and load the text that will be inserted in its place:**/
		printDebug("Found the include statement: \"%s\"\n", includeStatement);
		splitString(combinedText, includeStatement, &leftBuffer, &rightBuffer); //Split the text into two pieces on either side of the next "@Include <*>" statement
		includedTextBuffer = getIncludedFileText(includeStatement);
		
		/**The size of our recombined file is the sum of its pieces. NOTE: If any of the pieces do not exist, their pointer will be NULL**/
		combinedFileSize = 1; //Enough room for a null-terminator, in addition to the following lengths
		if(leftBuffer != NULL) {
			combinedFileSize += strlen(leftBuffer);
		}
		
		if(rightBuffer != NULL) {
			combinedFileSize += strlen(rightBuffer);
		}
		
		if(includedTextBuffer != NULL) {
			combinedFileSize += strlen(includedTextBuffer);
		} else {
			printf("WARNING: Could not resolve \"%s\" to a filename!\n", includeStatement);
		}
		printDebug("\nCombined file size is: %d\n", combinedFileSize);
		
		/**We have the necessary content in the buffers, reallocate space for the new (potentially larger) file**/
		if(combinedText != primaryText) { //Don't accidentally free the string that was passed as an argument, we don't want any side-effects
			free(combinedText);
		}
		combinedText = calloc(combinedFileSize, sizeof(char));
		
		/**Recombine the text around the contents of the included file:**/
		printDebug("\n >Combining text...\n");
		if(leftBuffer != NULL) {
			strcat(combinedText, leftBuffer);
		}
		if(includedTextBuffer != NULL) {
			strcat(combinedText, includedTextBuffer);
		}
		if(rightBuffer != NULL) {
			strcat(combinedText, rightBuffer);
		}
		printDebug("Text Combined.\n");
		printDebug("\n\"%s\"\n\n", combinedText);
		
		free(includeStatement);
		leftBuffer = NULL;
		rightBuffer = NULL;
	}
	
	return combinedText;
}

char* findInclude(char* text) { /**Utility-function of replaceIncludeStatements(); Searches fileText for the first instance of "@Include <*>" with * being a file name, and returns a copy of it. Otherwise returns NULL**/
	//Find the first "@Include" string, if one is present:
	char* position = strstr(text, "@Include");
	if(position == NULL) {
		return NULL;
	}
	
	//Find the next occurrence of '>':
	char* closingChar = strchr(position, '>');
	if(closingChar == NULL) {
		printf("ERROR: @Include statement on line %d has no indicated file name!\n", getLineNumber(text, position));
		exit(1);
	}
	//Copy all the contents between position and this occurrence:
	int includeLength = closingChar - position + 1; //Length of the string from "@Include" to the first '>'
	char* includeStatement = malloc(includeLength + 1);
	strncpy(includeStatement, position, includeLength);
	includeStatement[includeLength] = '\0';
	
	//Verify that there are only two tokens in the copy, and that the second one is valid:
	char* tempBuffer = malloc(strlen(includeStatement) + 1);
	char* token = NULL;
	strcpy(tempBuffer, includeStatement); //Copy the include statement to a temporary buffer, since strtok is destructive
	printDebug("tempBuffer contains: \"%s\"\n", tempBuffer);
	
	token = strtok(tempBuffer, " \n\t");
	printDebug("First Token: %s\n", token);
	if(strcmp(token, "@Include") != 0) {
		printf("ERROR: Include statement on line %d was unable to be parsed!\n", getLineNumber(text, position));
		exit(1);
	}
	token = strtok(NULL, "");
	printDebug("Second Token: %s\n", token);
	if(token[0] != '<' || token[strlen(token) - 1] != '>') {
		printf("ERROR: @Include statement on line %d was unable to be parsed!\n", getLineNumber(text, position));
		exit(1);
	}
	
	free(tempBuffer);
	return includeStatement;
}

char* getIncludedFileText(char* includeStatement) { /**Utility-function of replaceIncludeStatements(); Matches the included file name with the appropriate otherFileName and returns the otherFileName's text**/
	/**Parse the file name from the includeStatement (it's assumed to be syntactically valid at this point)**/
	printDebug("\n >Parsing fileName from include statement: \"%s\"", includeStatement);
	char* startPos = strchr(includeStatement, '<') + 1;
	char* endPos = strchr(includeStatement, '>');
	
	int fileNameSize = endPos - startPos;
	printDebug("Length of the file name is %d\n", fileNameSize);
	char* fileName = malloc(sizeof(char) * fileNameSize + 1);
	strncpy(fileName, startPos, fileNameSize);
	fileName[fileNameSize] = '\0';
	printDebug("Resulting fileName is: \"%s\"\n", fileName);

	return readTextFile(fileName);
	
	printDebug("WARNING: Found no matching file name in the provided input!");
	return NULL;
}

int getLineNumber(char* text, char* position) { /**Returns the line number of the given position in the String named text**/
	if(text == NULL || position == NULL || text > position) {
		return -1; //Signify an error
	}
	
	int lineNumber = 1;
	
	while(text < position) {
		if(*text == '\n') {
			lineNumber++;
		} else if(*text == '\0') {
			return -1; //Signify an error
		}
	}
	
	return lineNumber;
}
