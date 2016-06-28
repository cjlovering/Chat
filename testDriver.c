#include "tree.h"
#include "utility.h"

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

  //trim

  char* s1 = malloc(sizeof(char) * 20);//"hello";
  char* s2 = malloc(sizeof(char) * 20);//"hello";"   hello\n ";
  strcpy(s1, "hello");
  strcpy(s2, "   hello\n ");
  
  if (strcmp(s1, trim( s2 )) == 0) printf("trim worked\n");
  else printf("trim did not work\n");
  
  //strcmpc
  char* s3 = "_EXIT_ user";
  char* s4 = "_EXIT";

  char* t1;

#if 1
//more tests on delete user
  printf("testing removeUser\n");
  Tree* t4 = newTree();
  addUser(t4, "james", 12);
  addUser(t4, "ann", 12);
  addUser(t4, "bob", 12);
  addUser(t4, "robert", 12);
  addUser(t4, "pop", 12);
  addUser(t4, "teemo", 12);

  displayTree(t4);
  
  removeUser(t4, findUser(t4, "james"));
  removeUser(t4, findUser(t4, "ann"));
  removeUser(t4, findUser(t4, "bob"));
  removeUser(t4, findUser(t4, "robert"));
  removeUser(t4, findUser(t4, "pop"));
// removeUser(t4, findUser(t4, "teemo"));
  displayTree(t4);
//  removeUser(t4, findUser(t4, "teemo"));

  char* ss1 = calloc(sizeof(char),10); strcat(ss1, "james");
  char* ss2 = calloc(sizeof(char),10); strcat(ss2, "ann");
  char* ss3 = calloc(sizeof(char),10); strcat(ss3, "bob");
  char* ss4 = calloc(sizeof(char),10); strcat(ss4, "robert");
  char* ss5 = calloc(sizeof(char),10); strcat(ss5, "pop");
  char* ss6 = calloc(sizeof(char),10); strcat(ss6, "teemo");
  
  //more tests on delete user
  printf("testing deleteUser\n");
  Tree* t3 = newTree();
  addUser(t3, ss1, 12);
  addUser(t3, ss2, 12);
  addUser(t3, ss3, 12);
  addUser(t3, ss4, 12);
  addUser(t3, ss5, 12);
  addUser(t3, ss6, 12);

  displayTree(t3);
  
  deleteUser(t3, ss1);
  deleteUser(t3, ss2);
  deleteUser(t3, ss3);
  deleteUser(t3, ss4);
  deleteUser(t3, ss5);
  deleteUser(t3, ss6);

  displayTree(t3);
#endif
  printf("testing deleteUser again\n");

  Tree* t5 = newTree();
  displayTree(t5);
  addUser(t5, "bob", 12);
  displayTree(t5);
  addUser(t5, "robert", 12);
  displayTree(t5);
  addUser(t5, "aaa", 12);
  displayTree(t5);
  addUser(t5, "bbb", 12);
  displayTree(t5);
  addUser(t5, "ccc", 12);
  displayTree(t5);
  
  deleteUser(t5, "bob");
  displayTree(t5);
  deleteUser(t5, "robert");
  displayTree(t5);
  deleteUser(t5, "aaa");
  displayTree(t5);
  deleteUser(t5, "bbb");
  displayTree(t5);
  deleteUser(t5, "ccc");
  displayTree(t5);


  return 0;
}
