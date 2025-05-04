#include <iostream>
#include <string>
using namespace std;

string& foo(string &str)
{
    return str;
}

int main() {
    string str1 = "str1";
    //string str2 = foo(str1);
    //string &str2 = foo(str1);
    //auto str2 = foo(str1);  // когда auto - легче запутаться, можно подумать, что ауто выведет и ссылку
    //auto &str2 = foo(str1);
    str2 = "str2";
    cout << str1 << endl << str2;
}


#include <iostream>
using namespace std;

class T
{
    public:
    int var = 100;
    T() { cout << "T()" << endl; }
    T(const T &t) { cout << "T(const T &t))" << endl; }
    T(T &&t) { cout << "T(T &&t)" << endl; }
    ~T() { cout << "~T()" << endl; }
};

T& foo(T &t)
{
    return t;
}

int main() 
{
    T t;
    auto t2 = foo(t);
    //auto &t2 = foo(t); // чтобы избежать копирования и пориметь ссылку
    t2.var = 200;

    cout << "t.var" << t.var << endl;

    return 0;
}