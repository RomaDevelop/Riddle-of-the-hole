#include <iostream>
using namespace std;
 
struct C
{
    string value;
    C(string arg): value{arg} { cout<< "[" << arg << "]"; }
    C(string arg): value{move(arg)} { cout<< "[" << arg << "]"; }
    template <typename T>
    C(T &&arg): value(std::forward<T>(arg)) 
    { 
        cout<< "[" << arg << "] " << &arg << " " << typeid(arg).name() << endl; 
        arg += "sdvsdv"; // можно и это проблема
    }
};

https://www.youtube.com/watch?v=PNRju6_yn3o
предлагает вообще
template <typename T>
C(T &&arg): value(std::forward<T>arg) { cout<< "[" << arg << "]"; }
 
int main()
{
 
    C c("sdvsdv");
    
    return 0;
}
 