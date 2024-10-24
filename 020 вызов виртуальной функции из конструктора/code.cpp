#include <iostream>
using namespace std;

class B
{
    int value;
    virtual int f() const { return 1; } 
 
public:
    B() { value = f(); }
    int getValue() const { return value; }
};
 
class D : public B
{
    virtual int f() const { return 2; } 
};
 
int main()
{
    D d; 
    cout << d.getValue() << endl; // Выводит 1, а не 2
}

Вызов виртуальной функции из конструктора
Суть в том, что при создании объекта D, сначала создается базовая часть (класс B), а в конструкторе базового класса ничего о классе D еще не известно, т.к. он еще не создан (и в том числе не заполнена таблица виртуальных функций).
