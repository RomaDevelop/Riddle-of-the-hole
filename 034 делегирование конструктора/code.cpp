#include <iostream>
#include <vector>
using namespace std;

class T
{
public:
    int v = -1;

    T(): v{0} { v = 100; cout << v << " T()\n"; v = 1;  }

    T(int): T() { cout << "T(int)\n"; }
	//T(char) { T(); cout << "T(char)\n"; }  // T(); создает временный объект
    //T(char): T(), v{100} { cout << v << " T(int)\n"; }  // не компилируется
    //T(int): T(), T("dsfbdfb") { cout << v << " T(int)\n"; }  // не компилируется

    void print() { cout << v << "\n"; }
};

int main() {
   
    T t(1);
    t.print();
    
    return 0;
}