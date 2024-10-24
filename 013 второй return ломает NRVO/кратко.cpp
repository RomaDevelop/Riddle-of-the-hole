#include <iostream>
#include <string>
using namespace std;
 
class Object
{
public:
    string name {"undefined"};

    Object() { cout <<"Object()\n"; }
    
    //Object(Object & obj) = delete;
    Object(Object && obj) = delete;
    
    //Object(Object & obj) {name = obj.name; cout <<"Object(Object & obj)\n"; }
    //Object(Object && obj) {name = obj.name; cout <<"Object(Object && obj)\n"; }
    ~Object() { cout<< "~Object() " << name <<"\n"; }
};
 
Object foo()
{
    //if(0) return Object();
    
    Object retObj;
    retObj.name = "foo";
    return retObj;
}
 
int main() {
    
    Object obj1 = foo();
    
    cout << "return 0\n";
    return 0;
}