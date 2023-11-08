#include<stdio.h>

int main() {
  char name[] = "Daniel Sturm";
  printf("Mein Name ist %s\n",name);
  int day = 24;
  int month = 9;
  int year = 2002;
  printf("Mein Geburtstag ist der %02d.%02d.%04d\n",day,month,year);
  return 0;
}
