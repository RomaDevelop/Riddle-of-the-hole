#include <iostream>
using namespace std;


auto add(const auto& a, const auto& b)
{
    return a + b;
}

int main() {
    
    cout << "Hello world!\n" <<endl;
    
    cout << add(5.3,5);
    
    return 0;
}