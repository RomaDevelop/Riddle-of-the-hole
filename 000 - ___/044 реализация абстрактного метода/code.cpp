#include <iostream>
#include <vector>
#include <memory>
using namespace std;

#include <iostream>
using namespace std;

struct A
{
    virtual void foo() = 0;
};

void A::foo()
{
    cout << "strange";
}

struct B: public A
{
    virtual void foo() {cout << "foo"; A::foo(); }
};
int main() {
    B b;
    b.foo();
         
    return 0;
}

// РАБОТАЕТ!!!