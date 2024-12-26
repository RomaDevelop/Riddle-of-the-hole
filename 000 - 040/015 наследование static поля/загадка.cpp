#include <iostream>
#include <string>
using namespace std;

class A
{
    public:
    static string str;
};
string A::str = "it is A";

class A1: public A
{
    
};

class A2: public A
{
    
};

int main() {
    
    A1::str = "it is A1";
    A2::str = "it is A2";
    cout << << A1::str << endl << A2::str << endl;
    
    return 0;
}

выводится
it is A2
it is A2

потому что static в любом случае одно