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

typedef struct user
{
  char* id;
  int socket;
} User;

Tree* newUserTree(User* root)
{
  Tree* new  = malloc(sizeof(Tree));
  new->root  = root;
  new->right = NULL;
  new->left  = NULL;

  return new;
}

User* newUser(



int add(Tree* t, User* data)
{
  return addData(t->root, data);
}

int addData(Node* current, User* data)
{
  if ( current->data == NULL )
    {
      current->data = data;
      count++;
      return 1;
    }

  int c = compare( Node->data, data );
  if      ( c == 1)  addData( Node->left,  data ); //current is greater, go left
  else if ( c == 0 ) addData( Node->right, data ); //current is smaller, go right
  else return -1; //user already exists
}

int remove(Tree* t, User* data)
{
  return removeData(t->root, data);
}

int removeData(Tree* t, User* data)
{
  if ( current->data == NULL ) 
    return -1; //not found
  
  int c = compare( Node->data, data );
  if      ( c == 1 ) addRemote( Node->left,  data ); //current is greater, go left
  else if ( c == 0 ) addRemote( Node->right, data ); //current is smaller, go right
  else //the target is found 
  {
    current->data = data;
    count++;
    return 1;
  }
}

User* find(Tree* t, char* name)
{
  return findData(t->root, name);
}
user* findData(Node* current, char* name)
{
  int c = strcmp( current->root->id, name )

}

void display(Tree* t);


