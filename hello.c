#include <stdio.h>
#include <string.h>

char *str1 = "I love ";
char *str2 = "bananas\n";

int main() {
  volatile int x = 1;
	volatile int y = 2;
	if(y - 1 == x){
	printf("hello world %d\n", (x+y)*(x+y));
  char* newStr = strdup(str1);
	strcat(newStr, str2);
	printf("%s", newStr);
	y--;
	}

  return 0;
}

