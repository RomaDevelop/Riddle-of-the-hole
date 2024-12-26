#include <iostream>
using namespace std;

class T
{
    public:
    T() { cout << "T()" << endl; }
    T(const T &t) { cout << "T(T &t))" << endl; }
    T(T &&t) { cout << "T(T &&t)" << endl; }
};

void Foo(T& x) { cout << "Foo(T& x)" << endl; }
void Foo(T x) { cout << "Foo(T x)" << endl; }

// Для любых аргументов разрешение перегрузки завершится неудачей (хотя обе функции допустимы)
void Foo(const T& x);
void Foo(T x); 

int main() {
    /*const*/ T t;  // const устранит ошибку
    Foo(t);

    return 0;
}