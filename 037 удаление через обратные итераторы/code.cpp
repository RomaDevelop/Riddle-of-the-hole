#include <iostream>
#include <vector>
#include <list>
#include <memory>
using namespace std;

void print(vector<int> &v) { for(auto i:v) cout <<i << " "; cout << endl; }

int main() {

    vector<int> v {1,2,3,4,5,6,7,8,9};
    print(v);

    for(auto it = v.rbegin(); it != v.rend(); it++)
    {
        //cout << *it << " " << *it.base() << "\n";
        if(*it == 4 || *it == 8) v.erase(it.base());
    }
    print(v);

    return 0;
}

// загадка - удаляется не 4 и 8, а 5 и 9
// разгадка - it.base() возвращает итератор на следующий элемент
// при то cout << *it << " " << *it.base() << "\n"; выводит разное в случае с vector и list