#include "tree.h"

int main()
{
  Tree* t = newTree();
  addUser(t, "james", 12);
  addUser(t, "ann", 12);
  addUser(t, "bob", 12);
  addUser(t, "robert", 12);
  addUser(t, "pop", 12);
  addUser(t, "teemo", 12);

  displayTree(t); 
  User* r1 = findUser(t, "robert");
  User* r2 = findUser(t, "bob");
  User* r3 = findUser(t, "teemo");
  User* r4 = findUser(t, "sdfgdsfg");
  User* r5 = findUser(t, "pop");
  User* r6 = findUser(t, "james");
  
  removeUser(t, r6);
  displayTree(t);
  
  return 0;
}
