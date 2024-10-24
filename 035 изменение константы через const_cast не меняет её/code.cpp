 
#include <iostream>
using namespace std;
 
int main() {
    
    const int a=5;

    const int& const_ref_a = a;
    
    int& ref_a = const_cast<int &>(const_ref_a);

    ref_a = 10;

    int* a_ptr = &ref_a;
    
    cout << "a:\t\t" << a << "\t" << &a << endl;
    cout << "const_ref_a:\t" << const_ref_a << "\t" << &const_ref_a << endl;
    cout << "ref_a: \t\t" << ref_a << "\t" << &ref_a << endl;
    cout << "*a_ptr: \t" << *a_ptr << "\t" << a_ptr << endl;

    return 0;
}


выовд:


a:				5	0x7ffdb7b3eb54
const_ref_a:	10	0x7ffdb7b3eb54
ref_a: 			10	0x7ffdb7b3eb54
*a_ptr: 		10	0x7ffdb7b3eb54