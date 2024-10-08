#define _CRT_SECURE_NO_WARNINGS
/*
Name: Darasimi Ogunbinu-Peters
WSUID: 011812162
*/



#include <stdio.h>            
#include <stdbool.h> //for bool
#include <stdlib.h>    
#include <string.h>

#define MAX_PATH 50
#define MAX_FILENAME 75
#define MAX_LINE 100


typedef struct node {
	char  name[64];  // node's name string
	char  type; // D - directory, F - file
	struct node* child, * sibling, * parent;
} NODE;

NODE* root;
NODE* cwd;

void initialize();
NODE* createNode(const char* name, char type);
void insertNode(NODE** root, const char* name, char type, bool isSibling);
void inorderTraversal(NODE* root);
bool searchNode(NODE* root, const char* pathname);
NODE* findNode(NODE* root, const char* pathname);
bool isBasenameInFullpathname(char* fullpathname, char* basename);
void deleteNode(NODE** root, const char* pathname);




// defining constant macros

// D - directory, F - file
#define DIR 'D'
#define FIL 'F'

// other global variables

void displayCommands();
int findCommand(char* user_command);
char* promptPathname(char* pathname);
char* promptFilename(char* filename);
bool isAbsolutePath(char* pathname);
void tokenizePathname(char* pathname, char tokens[10][64], int* numTokens);
bool searchPath(NODE* root, char* pathname);
void splitPath(const char* pathname, char* dirname, char* basename);
void saveTree(char* filename, NODE* root);
void saveTreeToFile(FILE* file, NODE* node, const char* path);
void loadTree(char* filename, NODE** root);
void loadTreeFromFile(FILE* file, NODE** root);

//command specifics
//--------------------------------------------------------------------------------------------
/*
	<summary>
	mkdir [pathname]
	Make a new directory for a given pathname.
	Show error message(DIR pathname already exists!) if the directory already present in the
	filesystem.
	</summary>
*/
void mkdir_(char pathname[MAX_PATH]);
/*
	<summary>
	rmdir [pathname]
	Remove the directory for a given pathname.Show error messages if:
	− The directory specified in pathname does not exist (DIR pathname does not exist!)
	− The directory is not empty (Cannot remove DIR pathname (not empty)!).Show error message(DIR pathname is not empty!) if the directory is not empty.
	</summary>
*/
void rmdir_(char pathname[MAX_PATH]);
/*
	<summary>
	cd [pathname]
	Change CWD to pathname, or to / if no pathname specified.
	Display an error message (No such file or directory: pathname) for an invalid pathname.
	</summary>
*/
void cd_(char pathname[MAX_PATH]);
/*
	<summary>
	ls [pathname]
	List the directory contents of pathname or CWD (if pathname not specified).
	Display an error message (No such file or directory: pathname) for an invalid pathname.
	</summary>
*/
void ls_(char pathname[MAX_PATH]);
/*
	<summary>
	pwd []
	Show the current working directory (CWD).
	</summary>
*/
void pwd_();
/*
	<summary>
	creat [pathname]
	Create a file for a given pathname. Show error message(FILE pathname already exists!) if the file already present in the filesystem.
	</summary>
*/
void creat_(char pathname[MAX_PATH]);
/*
	<summary>
	rm [pathname]
	Remove the file for a given pathname. Show error message(FILE pathname does not exist!) if the file specified in pathname does not exist.
	</summary>
*/
void rm_(char pathname[MAX_PATH]);
/*
	<summary>
	save [filename]
	Save the current file system tree to a file. The file is specified by filename. Show error message(FILE filename already exists!) if the file already present in the filesystem.
	</summary>
*/
void save_(char filename[MAX_FILENAME]);
/*
	<summary>
	reload [filename]
	Reload a file system tree from a file. The file is specified by filename. Show error message(FILE filename does not exist!) if the file specified in filename does not exist.
	</summary>
*/
void reload_(char filename[MAX_FILENAME]);


//Function to create a new node
NODE* createNode(const char* name, char type) {
	NODE* newNode = (NODE*)malloc(sizeof(NODE));
	if (newNode == NULL) {
		printf("error: memory allocation failed.\n");
		exit(1);
	}
	strncpy(newNode->name, name, 64);
	newNode->name[63] = '\0'; //ensuring null char at the end.
	newNode->type = type;
	newNode->child = NULL;
	newNode->sibling = NULL;
	newNode->parent = NULL;
	return newNode;
}

void initialize() {
	root = (NODE*)malloc(sizeof(NODE));
	strcpy(root->name, "/");
	root->parent = root;
	root->sibling = 0;
	root->child = 0;
	root->type = 'D';
	cwd = root;
	printf("Filesystem initialized!\n");
};


void deleteNode(NODE** root, const char* pathname) {
	NODE* nodeToDelete = findNode(*root, pathname);
	if (nodeToDelete == NULL) {
		printf("Node not found\n");
		return;
	}

	if (nodeToDelete->sibling != NULL) {
		// If the node has a sibling, promote the sibling to the root
		*root = nodeToDelete->sibling;
	}
	else if (nodeToDelete->parent != NULL) {
		// If the node is the only child, set the parent's child pointer to NULL
		if (nodeToDelete->parent->child == nodeToDelete) {
			nodeToDelete->parent->child = NULL;
		}
		else {
			nodeToDelete->parent->sibling = NULL;
		}
	}
	else {
		// Deleting the only node in the tree
		*root = NULL;
	}

	free(nodeToDelete);
	printf("\nNode deleted successfully\n");
}



//Function to insert node into the BST
//work on this.
void insertNode(NODE** root, const char* name, char type, bool isSibling) {
	if (*root == NULL) {
		*root = createNode(name, type);
		printf("new node: %s added, type  %c\n", name, type);
	}
	else {
		if (strcmp(name, (*root)->name) != 0) {
			insertNode(&(*root)->child, name, type, false);
		}
		else if (isSibling) {
			insertNode(&(*root)->sibling, name, type, false);
		}
	}
}





bool searchPath(NODE* root, char* pathname) {
	char tokens[10][64]; // Assuming maximum 10 tokens in a pathname
	int numTokens = 0;
	tokenizePathname(pathname, tokens, &numTokens);

	for (int i = 0; i < numTokens; i++) {
		if (searchNode(root, tokens[i])) {
			return true; // Token found in BST
		}
	}
	return false; // No matching tokens found
}

//Function to perform inorder traversal of the binary search tree (for printing) [work on the printing]
void inorderTraversal(NODE* root) {
	if (root != NULL) {
		inorderTraversal(root->child);
		printf("\ntype:(%c)\tname:%s \n", root->type, root->name);
		inorderTraversal(root->sibling);
	}
}

bool searchNode(NODE* root, const char* pathname) {
	if (root == NULL) {
		return false; // Node not found
	}

	if (strcmp(root->name, pathname) == 0) {
		return true; // Node found
	}

	// Recursively search in the left subtree
	if (searchNode(root->child, pathname)) {
		return true;
	}

	// Recursively search in the right subtree
	if (searchNode(root->sibling, pathname)) {
		return true;
	}

	return false; // Node not found
}




void saveTreeToFile(FILE* file, NODE* node, const char* path) {
	if (node == NULL) {
		return;
	}

	fprintf(file, "%c\t%s\n", node->type, node->name);

	saveTreeToFile(file, node->child, node->name); // Recursively save child nodes
	saveTreeToFile(file, node->sibling, node->name); // Recursively save sibling nodes
}

void saveTree(char* filename, NODE* root) {
	FILE* file = fopen(filename, "w");
	if (file == NULL) {
		printf("Error opening file.\n");
		return;
	}

	saveTreeToFile(file, root, "/");
	fclose(file);
}

NODE* findNode(NODE* root, const char* pathname) {
	if (root == NULL) {
		return NULL; // Node not found
	}

	if (strcmp(root->name, pathname) == 0) {
		return root; // Node found
	}

	// Recursively search in the left subtree
	NODE* foundNode = findNode(root->child, pathname);
	if (foundNode != NULL) {
		return foundNode;
	}

	// Recursively search in the right subtree
	foundNode = findNode(root->sibling, pathname);
	if (foundNode != NULL) {
		return foundNode;
	}

	return NULL; // Node not found
}



void displayCommands() {
	printf("1. mkdir\n");
	printf("2. rmdir\n");
	printf("3. cd\n");
	printf("4. ls\n");
	printf("5. pwd\n");
	printf("6. creat\n");
	printf("7. rm\n");
	printf("8. save\n");
	printf("9. reload\n");
	printf("10. quit\n");
	printf("\n>>.");
};

int findCommand(char* user_command) {
	char* cmd[] = { "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "quit" };
	int i = 0;
	while (cmd[i]) {
		if (strcmp(user_command, cmd[i]) == 0) {
			return i;
			i++;
		}
		return -1;
	}
	return -1;
};

char* promptPathname(char* pathname) {
	printf("enter pathname here:");
	scanf("%s", pathname);
	return pathname;
}

char* promptFilename(char* filename) {
	printf("enter filename here:");
	scanf("%s", filename);
	return filename;
}



/*
* <summary>
* (Partially AI-generated), prompt: "how to tokenize a pathname"
* splits the paths by the number of '/', stores the tokens and the number of tokens found.
* BASICALLY AN EXTENDED VERSION OF SPLITPATH FUNCTION.
* </summary>
*/
void tokenizePathname(char* pathname, char tokens[10][64], int* numTokens) {
	char* token = strtok(pathname, "/");
	while (token != NULL && *numTokens < 10) {
		strcpy(tokens[(*numTokens)++], token);
		token = strtok(NULL, "/");
	}
}

/*
* <summary>
* (Partially AI-generated), prompt: "how to find the last '/' in a pathname and split it."
* splits the path by only the last '/' and stores the dirname and basename in the respective variables.
* </summary>
*/

void splitPath(const char* pathname, char* dirname, char* basename) {
	int i, last_slash = -1;

	//finding the position of the last '/'
	for (i = 0; pathname[i] != 0; i++) {
		if (pathname[i] == '/') {
			last_slash = i;
		}
	}

	// if no '/' is found, then the dirname is '.' and basename is the entire pathname.
	if (last_slash == -1) {
		strcpy(dirname, ".");
		strcpy(basename, pathname);
	}
	else {
		//Extract dirname and basename
		strncpy(dirname, pathname, last_slash);
		dirname[last_slash] = '\0'; //to ensure that it's a string by ending with null char.
		strcpy(basename, pathname + last_slash + 1);
	}

}


bool isAbsolutePath(char* pathname) {
	return pathname[0] == '/';
}

bool isBasenameInFullpathname(char* fullpathname, char* basename) {
	char fulltokens[10][64]; // Assuming maximum 10 tokens in a pathname
	int fullnumTokens = 0;
	tokenizePathname(fullpathname, fulltokens, &fullnumTokens);

	char basetokens[10][64]; // Assuming maximum 10 tokens in a pathname
	int basenumTokens = 0;
	tokenizePathname(basename, basetokens, &basenumTokens);

	if (fullnumTokens >= basenumTokens) {
		// Compare the last basenumTokens tokens of fullpathname with basename
		int i = fullnumTokens - basenumTokens;
		int j = 0;
		while (i < fullnumTokens && j < basenumTokens) {
			if (strcmp(fulltokens[i], basetokens[j]) != 0) {
				return false;
			}
			i++;
			j++;
		}
		return true;
	}
	return false; // basename is longer than fullpathname
}



void rmdir_(char pathname[MAX_PATH])
{
	char dirname[MAX_PATH];
	char basename[MAX_PATH];
	bool foundInRelative = false;
	bool foundInAbsolute = false;
	bool foundPath = false;
	NODE* foundNode = (NODE*)NULL;

	if (isAbsolutePath(pathname)) {
		splitPath(pathname, dirname, basename);
		foundInAbsolute = searchNode(root, basename);
	}
	else if (!isAbsolutePath(pathname)) { //path is relative.
		splitPath(pathname, dirname, basename);
		foundInRelative = searchNode(cwd, basename);
	}

	if (foundInAbsolute) {
		foundPath = searchPath(root, pathname);
		if (foundPath) {
			printf("\nfile: '%s' exists", basename);
			foundNode = findNode(root, basename);
		}
		else
		{
			printf("\nfile: '%s' does not exist", basename);
			return;
		}
	}
	else if (foundInRelative) {
		foundPath = searchPath(cwd, pathname);
		if (foundPath) {
			printf("\nfile: '%s' exists", basename);
			foundNode = findNode(cwd, basename);
		}
		else
		{
			printf("\nfile: '%s' does not exist\n", basename);
			return;
		}
	}

	if (foundNode != NULL && foundNode->type == DIR) {

		//check if the directory is empty
		if (foundNode->child == NULL) {
			//if empty, remove directory and delete node from tree.
			deleteNode(&foundNode, basename);
			if (foundNode == NULL) {
				printf("\nfile: '%s' removed\n", basename);
			}
		}
		//else don't remove directory
		else
		{
			printf("\nerror: cannot remove file: '%s', because it is not empty.\n", basename);
			return;
		}
	}
	else
	{
		printf("\nfile: '%s' does not exist\n", basename);
		return;
	}

}

void mkdir_(char pathname[MAX_PATH])
{
	//Step1: distinguishing between absolute or relative
	//printf("inside mkdir");
	char dirname[MAX_PATH];
	char basename[MAX_PATH];
	bool isAbsolute = false;
	bool isRelative = false;

	char tokens[10][64]; // Assuming maximum 10 tokens in a pathname
	int numTokens = 0;
	char fullPathname[MAX_PATH];
	strcpy(fullPathname, pathname);
	tokenizePathname(pathname, tokens, &numTokens);

	NODE* parent = cwd;
	for (int i = 0; i < numTokens; i++) {
		bool found = searchNode(parent, tokens[i]);
		if (!found) {
			// Add a new DIR node under parent
			insertNode(&parent, tokens[i], 'D', false);
		}
		parent = findNode(parent, tokens[i]);
	}


	if (isAbsolutePath(fullPathname)) {
		isAbsolute = true;
		splitPath(fullPathname, dirname, basename);
	}
	else { //path is relative.
		isRelative = true;
		splitPath(fullPathname, dirname, basename);
	}

	//Step2: search for dirname node:
	if (isRelative || isAbsolute) {
		bool foundInRelative = false;
		bool foundInAbsolute = false;
		foundInRelative = searchNode(cwd, basename);
		foundInAbsolute = searchNode(root, basename);
		NODE* foundNode = (NODE*)NULL;

		if (foundInRelative) {
			foundNode = findNode(cwd, basename);
		}
		else if (foundInAbsolute) {
			foundNode = findNode(root, basename);
		}
		else { // if dirname not found in CWD or root. OR found in wrong place.
			if (foundNode != NULL && foundInAbsolute) {
				printf("error: %s found in root, instead of CWD\n", basename);
				return;
			}
			else if (foundNode != NULL && foundInRelative) {
				printf("error: %s found in CWD, instead of root\n", basename);
				return;
			}
			else {
				insertNode(&root, basename, DIR, false);
				return;
			}
		}
		if (foundNode != NULL && foundNode->type == DIR) {
			bool foundBasename = searchNode(foundNode, basename);
			if (!foundBasename) {
				// add a new DIR node under dirname(i.e. foundNode)
				insertNode(&foundNode, basename, DIR, false);
				return;
			}
			else {
				if (!isBasenameInFullpathname(fullPathname, basename)) {
					printf("error: %s already exists in %s\n", basename, dirname);
					return;
				}
				else {
					printf("folder %s successfully created in %s\n", basename, dirname);
					return;
				}
			
			}
		}
	}
	else {
		printf("error: %s not found in CWD or root\n", dirname);
		return;
	}
}



void ls_(char pathname[MAX_PATH])
{

	//list the contents of the directory, if not specified, list the contents of the CWD.
	if (strcmp(pathname, "''") == 0) {
		inorderTraversal(cwd);
	}
	else {
		char dirname[MAX_PATH];
		char basename[MAX_PATH];
		bool foundInRelative = false;
		bool foundInAbsolute = false;
		bool foundPath = false;
		NODE* foundNode = (NODE*)NULL;


		printf("\nyou are currently in %s\n", cwd->name);

		if (isAbsolutePath(pathname)) {
			splitPath(pathname, dirname, basename);
			foundInAbsolute = searchNode(root, basename);
		}
		else if ((strcmp(pathname, "..") != 0)) { //path is relative.
			splitPath(pathname, dirname, basename);
			foundInRelative = searchNode(cwd, basename);
		}
		if (foundInAbsolute) {
			foundPath = searchPath(root, pathname);
			if (foundPath) {
				printf("\nfile: '%s' exists", basename);
				foundNode = findNode(root, basename);
			}
			else
			{
				printf("\nfile: '%s' does not exist", basename);
				return;
			}
		}
		else if (foundInRelative) {
			foundPath = searchPath(cwd, pathname);
			if (foundPath) {
				printf("\nfile: '%s' exists", basename);
				foundNode = findNode(cwd, basename);
			}
			else
			{
				printf("\nfile: '%s' does not exist\n", basename);
				return;
			}
		}

		if (foundNode != NULL && foundNode->type == DIR) {
			//if so , change the cwd to the directory.
			printf("\nTYPE\tNAME\n");
			inorderTraversal(foundNode);
			return;
		}
		else
		{
			printf("\nerror: '%s' does not exist.\n", basename);
			return;

		}

	}

}

void cd_(char pathname[MAX_PATH])
{

	char dirname[MAX_PATH];
	char basename[MAX_PATH];
	bool foundInRelative = false;
	bool foundInAbsolute = false;
	bool foundPath = false;
	NODE* foundNode = (NODE*)NULL;


	printf("\nyou are currently in %s\n", cwd->name);

	if (isAbsolutePath(pathname)) {
		splitPath(pathname, dirname, basename);
		foundInAbsolute = searchNode(root, basename);
	}
	else if ((strcmp(pathname ,".." ) != 0 ) ) { //path is relative.
		splitPath(pathname, dirname, basename);
		foundInRelative = searchNode(cwd, basename);
	}

	if (strcmp(pathname, "") == 0) {
		cwd = root;
		printf("\nyou are now in '%s' directory!\n", cwd->name);
		return;
	}
	else if (strcmp(pathname,"..") == 0 && cwd->parent != NULL) {
		cwd = cwd->parent;
		printf("\nyou are now in '%s' directory!\n", cwd->name);
		return;

	}
	else if (strcmp(pathname,cwd->name) == 0) {
		printf("\nyou are already in that directory.\n");
		return;
	}
	else {
		//find if the directory exists
		if (foundInAbsolute) {
			foundPath = searchPath(root, pathname);
			if (foundPath) {
				printf("\nfile: '%s' exists", basename);
				foundNode = findNode(root, basename);
			}
			else
			{
				printf("\nfile: '%s' does not exist", basename);
				return;
			}
		}
		else if (foundInRelative) {
			foundPath = searchPath(cwd, pathname);
			if (foundPath) {
				printf("\nfile: '%s' exists", basename);
				foundNode = findNode(cwd, basename);
			}
			else
			{
				printf("\nfile: '%s' does not exist\n", basename);
				return;
			}
		}

		if (foundNode && foundNode->type == DIR) {
			//if so , change the cwd to the directory.
		

			//change the parent of the cwd to the current cwd.
			NODE* tempNode = (NODE*)NULL;
			tempNode = cwd;
			cwd = foundNode;
			cwd->parent = tempNode;

			if (strcmp(cwd->name, basename) == 0) {
				printf("\nyou are now in '%s' directory!\n", cwd->name);
				return;
			}
			else
			{
				printf("\nerror: '%s' is not a directory.\n", basename);
				return;
			}
		}
		else
		{
			printf("\nerror: '%s' does not exist.\n", basename);
			return;

		}
		
	}

}


void pwd_()
{
	inorderTraversal(root);
}

void creat_(char pathname[MAX_PATH])
{
	//Step1: distinguishing between absolute or relative
	//printf("inside mkdir");
	char dirname[MAX_PATH];
	char basename[MAX_PATH];
	bool isAbsolute = false;
	bool isRelative = false;

	char tokens[10][64]; // Assuming maximum 10 tokens in a pathname
	int numTokens = 0;
	char fullPathname[MAX_PATH];
	strcpy(fullPathname, pathname);
	tokenizePathname(pathname, tokens, &numTokens);

	NODE* parent = cwd;
	for (int i = 0; i < numTokens; i++) {
		if (i == numTokens - 1) {
			// Last part of the path, treat as file
			bool found = searchNode(parent, tokens[i]);
			if (!found) {
				// Add a new FILE node under parent
				insertNode(&parent, tokens[i], 'F', false);
			}
			else {
				printf("File already exists: %s\n", tokens[i]);
				return;
			}
		}
		else {
			// Not the last part of the path, treat as directory
			bool found = searchNode(parent, tokens[i]);
			if (!found) {
				// Add a new DIR node under parent
				insertNode(&parent, tokens[i], 'D', false);
			}
			parent = findNode(parent, tokens[i]);
		}
	}

	if (isAbsolutePath(fullPathname)) {
		isAbsolute = true;
		splitPath(fullPathname, dirname, basename);
	}
	else { //path is relative.
		isRelative = true;
		splitPath(fullPathname, dirname, basename);
	}

	//Step2: search for dirname node:
	if (isRelative || isAbsolute) {
		bool foundInRelative = false;
		bool foundInAbsolute = false;
		foundInRelative = searchNode(cwd, basename);
		foundInAbsolute = searchNode(root, basename);
		NODE* foundNode = (NODE*)NULL;

		if (foundInRelative) {
			foundNode = findNode(cwd, basename);
		}
		else if (foundInAbsolute) {
			foundNode = findNode(root, basename);
		}
		else { // if dirname not found in CWD or root. OR found in wrong place.
			if (foundNode != NULL && foundInAbsolute) {
				printf("error: %s found in root, instead of CWD\n", basename);
				return;
			}
			else if (foundNode != NULL && foundInRelative) {
				printf("error: %s found in CWD, instead of root\n", basename);
				return;
			}
			else {
				insertNode(&root, basename, FIL, false);
				return;
			}
		}
		if (foundNode != NULL && foundNode->type == FIL) {
			bool foundBasename = searchNode(foundNode, basename);
			if (!foundBasename) {
				// add a new DIR node under dirname(i.e. foundNode)
				insertNode(&foundNode, basename, FIL, false);
				return;
			}
			else {
				if (!isBasenameInFullpathname(fullPathname, basename)) {
					printf("error: %s already exists in %s\n", basename, dirname);
					return;
				}
				else {
					if (strcmp(dirname, " ") == 0) {
						strcpy(dirname, "/");
					}
					printf("file %s successfully created in %s\n", basename, dirname);
					return;
				}

			}
		}
	}
	else {
		printf("error: %s not found in CWD or root\n", dirname);
		return;
	}

}

void rm_(char pathname[MAX_PATH])
{
	char dirname[MAX_PATH];
	char basename[MAX_PATH];
	bool foundInRelative = false;
	bool foundInAbsolute = false;
	bool foundPath = false;
	NODE* foundNode = (NODE*)NULL;

	if (isAbsolutePath(pathname)) {
		splitPath(pathname, dirname, basename);
		foundInAbsolute = searchNode(root, basename);
	}
	else if (!isAbsolutePath(pathname)) { //path is relative.
		splitPath(pathname, dirname, basename);
		foundInRelative = searchNode(cwd, basename);
	}

	if (foundInAbsolute) {
		foundPath = searchPath(root, pathname);
		if (foundPath) {
			printf("\nfile: '%s' exists", basename);
			foundNode = findNode(root, basename);
		}
		else
		{
			printf("\nfile: '%s' does not exist", basename);
			return;
		}
	}
	else if (foundInRelative) {
		foundPath = searchPath(cwd, pathname);
		if (foundPath) {
			printf("\nfile: '%s' exists", basename);
			foundNode = findNode(cwd, basename);
		}
		else
		{
			printf("\nfile: '%s' does not exist\n", basename);
			return;
		}
	}

	if (foundNode != NULL && foundNode->type == FIL) {

		//check if the directory is empty
		if (foundNode->child == NULL) {
			//if empty, remove directory and delete node from tree.
			deleteNode(&foundNode, basename);
			if (foundNode == NULL) {
				printf("\nfile: '%s' removed\n", basename);
			}
		}
		//else don't remove directory
		else
		{
			printf("\nerror: cannot remove file: '%s', because it is not empty.\n", basename);
			return;
		}
	}
	else
	{
		printf("\nfile: '%s' does not exist\n", basename);
		return;
	}

}

void save_(char filename[MAX_FILENAME])
{
	FILE* file = fopen(filename, "w");
	if (file == NULL) {
		printf("Error creating file.\n");
		return;
	}

	saveTree(filename, root);
	printf("File saved successfully.\n");
	fclose(file);
}

void reload_(char filename[MAX_FILENAME])
{
	
}


int main() {
	// Start with root node
	initialize();

	int option = 0;

	while (option != 10) {
		displayCommands();
		scanf("%d", &option);

		if (option == 1) { //Implmented
			char mkdir_pathname[MAX_PATH];
			promptPathname(mkdir_pathname);
			mkdir_(mkdir_pathname);
		}
		if (option == 2) { //Implmented
			char rmdir_pathname[MAX_PATH];
			promptPathname(rmdir_pathname);
			rmdir_(rmdir_pathname);
		}
		if (option == 3) { //Implmented
			char cd_pathname[MAX_PATH];
			promptPathname(cd_pathname);
			cd_(cd_pathname);
		}
		if (option == 4) { //Implmented 
			//NOTE: if #4 isn't working as intended for '' scenario, hit enter and type in '' and it'll work.
			char ls_pathname[MAX_PATH];
			promptPathname(ls_pathname);
			ls_(ls_pathname);
		}
		if (option == 5) { //Implmented
			pwd_();
		}
		if (option == 6) { //Implmented
			char creat_pathname[MAX_PATH];
			promptPathname(creat_pathname);
			creat_(creat_pathname);
		}
		if (option == 7) { //Implmented
			char rm_pathname[MAX_PATH];
			promptPathname(rm_pathname);
			rm_(rm_pathname);
		}
		if (option == 8) {	
			char save_filename[MAX_FILENAME];
			promptFilename(save_filename);
			save_(save_filename);
		}
		if (option == 9) {
			char reload_filename[MAX_FILENAME];
			promptFilename(reload_filename);
			//reload_(reload_filename); unable to implement reload function on time
		}
		if (option == 10) {
			printf("Quit successfully.\n");
			exit(0);
		}
	}

	return 0;
}
