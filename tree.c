#include "tree.h"
#define DEBUG (1)

int validate(Tree* t, char* username)
{
#if DEBUG
  displayTree(t);
#endif
  return validateHelper(t->root, username);
}

int validateHelper(Node* current, char* data)
{
#if DEBUG
  printf("%s %s\n", "Validating user: ", data);
#endif
  if ( current == NULL )
  {
    return 1;//valid
  }
  
  int c = strcmp( current->data->id, data );
  if      ( c > 0 ) return  validateHelper( current->left,  data ); //current is greater, go left
  else if ( c < 0 ) return  validateHelper( current->right, data ); //current is smaller, go right
  
  
  return 0; //invalid
}

void displayTree(Tree* t)
{
  if (t->root == NULL) 
  {
    printf("<none>\n");
    return;
  }
  
  displayTreeHelper(t->root);
}

void displayTreeHelper(Node* current)
{
  if (current == NULL) return;
  displayTreeHelper(current->left);
  printf("%s ", current->data->id );
  displayTreeHelper(current->right);
}

int compare(User* a, User* b)
{
  return strcmp(a->id, b->id);
}

Tree* newTree()
{
  Tree* newTree  = (Tree*)calloc(sizeof(Tree), 1);

  if ( newTree == NULL  )
  {
    printf("%s\n", "Failed to malloc");
    exit(1);
  }
  
#if DEBUG 
  printf("%s\n", "New user tree created."); 
#endif
  
  return newTree;
}

Node* newNode(User* data)
{
  Node* new = (Node*)malloc(sizeof(Node));
  if ( new == NULL )
  {
    printf("%s\n", "Failed to malloc");
    exit(1);
  }
  
  new->data  = data;
  new->right = NULL;
  new->left  = NULL;

#if DEBUG 
  printf("%s\n", "New node created.");
#endif

  return new;
}

User* newUser(char* name, int socket)
{
  User* newUser = (User*)malloc(sizeof(User));
  if ( newUser == NULL )   
  {
    printf("%s\n", "Failed to malloc");
    exit(1);
  }
  
  newUser->id = calloc(sizeof(char), 20); strncpy(newUser->id, name, 20);
  newUser->socket = socket;
  
#if DEBUG 
  printf("%s\n", "New user created.");
#endif

  return newUser;
}

Tree* addUser(Tree* t, char* id, int socket)
{
#if DEBUG 
  printf("%s %s\n", "Adding user: ", id);
#endif

  User* u = newUser(id, socket);					 
  t->root = addUserHelper(t->root, u);
  return t;
}

Node* addUserHelper(Node* current, User* data)
{
  if ( current == NULL )
  {
    return newNode(data);
  }
  
  int c = compare( current->data, data );
  if      ( c > 0 ) current->left  = addUserHelper( current->left,  data ); //current is greater, go left
  else if ( c < 0 ) current->right = addUserHelper( current->right, data ); //current is smaller, go right
  //else //user already exists --> itll fail quietly, which is bad

  return current;
}

Node* removeUser(Tree* t, User* data)
{
  t->root = removeUserHelper(t->root, data);
  return t->root;
}

Node* removeUserHelper(Node* current, User* data)
{
  if ( current == NULL ) 
    return current; //not found
  
  int c = compare( current->data, data );
  if      ( c > 0 ) current->left  = removeUserHelper( current->left,  data ); //current is greater, go left
  else if ( c < 0 ) current->right = removeUserHelper( current->right, data ); //current is smaller, go right
  else //the target is found 
  {
    if ( current->left == NULL ) 
    {
      Node *temp = current->right;
      deleteNode( current );
      return temp;
    }
    else if ( current->right == NULL)
    {
      Node *temp = current->left;
      deleteNode( current );
      return temp;
    }
    
    Node* temp     = min( current->right );
    current->data  = temp->data;
    current->right = removeUserHelper( current->right, temp->data );
  }
  return current;
}

Node* deleteUser(Tree* t, char* data)
{
  t->root = deleteUserHelper(t->root, data);
  return t->root;
}

Node* deleteUserHelper(Node* current, char* data)
{
  if ( current == NULL ) 
    return current; //not found
  
  int c = strcmp( current->data->id, data );
  if      ( c > 0 ) current->left  = deleteUserHelper( current->left,  data ); //current is greater, go left
  else if ( c < 0 ) current->right = deleteUserHelper( current->right, data ); //current is smaller, go right
  else //the target is found 
  {
    if ( current->left == NULL ) 
    {
      Node *temp = current->right;
      deleteNodeData( current );
      return temp;
    }
    else if ( current->right == NULL)
    {
      Node *temp = current->left;
      deleteNodeData( current );
      return temp;
    }
  
    Node* temp = min( current->right );

#if DEBUG
    if (temp != NULL) printf("min: %s", temp->data->id);
#endif

    current->data = temp->data;
    //we can't delete the string, because its needed!!
    current->right = removeUserHelper( current->right, temp->data );
  }
  return current;
}

Node* min(Node* current)
{
  if (current->left == NULL) return current;
  else min(current->left);
}

User* findUser(Tree* t, char* name)
{
  return findUserHelper(t->root, name);
}

User* findUserHelper(Node* current, char* data)
{
  if ( current == NULL )
    return NULL;
  
  int c = strcmp( current->data->id, data );
  if      ( c > 0 ) return findUserHelper( current->left,  data ); //current is greater, go left
  else if ( c < 0 ) return findUserHelper( current->right, data ); //current is smaller, go right
  else return current->data; //user already exists
}

void deleteNode(Node* n)
{
  free( n );       //free node
}

void deleteNodeData(Node* n)
{
  free( n->data ); //free user
  free( n );       //free node
}

void deleteTree(Tree* t)
{
  deleteAll( t->root );
}

void deleteAll(Node* n)
{
  if ( n == NULL ) return;
  deleteAll( n->left  );
  deleteAll( n->right );
  deleteNodeData( n );
}
