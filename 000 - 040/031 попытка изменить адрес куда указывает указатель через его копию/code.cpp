#include <iostream>
using namespace std;
 
class Container
{
    int *a;
    
    public:
    Container(int *ptr): a{ptr} {}
    int* Get() {return a;}
};
 
int main() {
    
    int a=532;
    int b=34;
    
    Container container(&a);
    int *tmpPtr = container.Get();
    tmpPtr = &b;
    
    cout << *container.Get() <<endl;
    
    return 0;
}
