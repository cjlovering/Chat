#ifndef _LISTS_H
#define _LISTS_H


typedef struct binaryTree
{
  struct node* root;
  int count;
} Tree;

typedef struct node
{
  struct user* data;
  struct node *right;
  struct node *left;
} Node;
  
void add(Tree t, user* data);
void remove(Tree t, user* data);
void find(Tree t, char* name);
void display(Tree t);

#endef
