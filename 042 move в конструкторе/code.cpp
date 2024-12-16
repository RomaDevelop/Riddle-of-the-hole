#include <iostream>
using namespace std;
 
struct C
{
    string value;
    C(string arg): value{arg} { cout<< "[" << arg << "]"; }
    C(string arg): value{move(arg)} { cout<< "[" << arg << "]"; }
};


https://www.youtube.com/watch?v=PNRju6_yn3o
 
int main()
{
 
    C c("sdvsdv");
    
    return 0;
}
 