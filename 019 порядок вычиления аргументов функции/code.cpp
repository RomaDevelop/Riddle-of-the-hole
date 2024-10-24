#include <iostream>

int nhash = 0; 
 
int nkeyshash()
{
    nhash += 100;
    return nhash;
}
 
int main() {
    
    printf("\n%d\t%d\n", nkeyshash(), nhash);
    
    return 0;
}

gcc выдаёт 100 0
clang 100 100

#include <iostream>

int foo() { std::cout << "foo\n"; return 0; }
int bar() { std::cout << "bar\n"; return 0; }

void some(int a, int b) {}
 
int main() {
    
    some(foo(), bar());
    
    return 0;
}