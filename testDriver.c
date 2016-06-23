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

  if (validate(t, "james")) printf("Correctly validated.\n");
  else printf("ERROR\n");
  
  if (!validate(t, "pop")) printf("Correctly did not validate.\n");
  else printf("ERROR\n");

  // testing 
  Tree* t2 = newTree();
  addUser(t2, "Xeno", 12);  
  addUser(t2, "Rock", 12);
  if (validate(t2, "james")) printf("Correctly validated.\n");
  else printf("ERROR\n");

  if (!validate(t2, "Xeno")) printf("Correctly did not validate.\n");
  else printf("ERROR\n");

  if (!validate(t2, "Rock")) printf("Correctly did not validate.\n");
  else printf("ERROR\n");

  return 0;
}
