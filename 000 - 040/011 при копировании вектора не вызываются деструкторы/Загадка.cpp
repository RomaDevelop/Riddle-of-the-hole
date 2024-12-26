// почему не вызываются конструкторы???

// obj()
// obj()
// obj()
// 56 56 56 
// 56 56 56 
// for(auto &i:vo) i.a++; did
// 57 57 57 
// 56 56 56 
// ~obj()
// ~obj()
// ~obj()
// ~obj()
// ~obj()
// ~obj()

#include <iostream>
#include <vector>
using namespace std;

class obj
{
    public:
    int a;
    obj() {cout << "obj()\n";}
    ~obj() {cout << "~obj()\n";}
};

vector<obj> get3obj()
{
    vector<obj> vo(3);
    return vo;
}

int main() {
    
    vector<obj> vo { get3obj() };
    for(auto &i:vo) i.a = 56;
    for(auto &i:vo) cout<<i.a<<" ";
    cout << "\n";
    
    vector<obj> vo2 = vo;
    for(auto &i:vo2) cout<<i.a<<" ";
    cout << "\n";
    
    for(auto &i:vo) i.a++;
    cout << "for(auto &i:vo) i.a++; did\n";
    
    for(auto &i:vo) cout<<i.a<<" ";
    cout << "\n";
    for(auto &i:vo2) cout<<i.a<<" ";
    cout << "\n";
    
    return 0;
}