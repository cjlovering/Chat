
#ifndef _TREE_H
#define _TREE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct binaryTree
{
  struct node* root;
  int counter;
} Tree;

typedef struct node
{
  struct user *data;
  struct node *right;
  struct node *left;
} Node;

typedef struct user
{
  char* id;
  int socket;
} User;

Node* newNode(User* data);
void displayTree(Tree* t);
void displayTreeHelper(Node* current);
int compare(User* a, User* b);
Tree* newTree();
User* newUser(char* name, int socket);
Tree* addUser(Tree* current, char* name, int socket);
Node* addUserHelper(Node* t, User* data);
Node* removeUser(Tree* t, User* data);
Node* removeUserHelper(Node* current, User* data);
Node* min(Node* current);
User* findUser(Tree* t, char* name);
User* findUserHelper(Node* current, char* name);
void deleteNode(Node* n);
void deleteTree(Tree* t);
void deleteAll(Node* n);
void deleteNodeData(Node* n);

#endif 
