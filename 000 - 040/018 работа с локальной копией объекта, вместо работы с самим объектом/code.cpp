Работа с локальной копией объекта, вместо работы с самим объектом


C++Выделить код
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
void foo_create( int * Arr ) {
    Arr = new int [10];
    for ( int i = 0; i < 10; i++ ) Arr[i] = i;
}
 
void foo_delete( int * Arr ) {
    delete [] Arr;
    Arr = NULL;
}
 
int main() {
    int * Ptr;
    foo_create( Ptr );
    for ( int i = 0; i < 10; i++ ) cout << Ptr[i] << "  ";
    foo_delete( Ptr );
    return 0;
}
Здесь ошибка в том, что при вызове функции foo_create в нее передается копия указателя Ptr
(который еще не инициализирован). При выделении памяти, ее адрес помещается в эту копию,
т.е. значение переменной Ptr не изменится.
В результате, после возврата из функции, адрес потеряется и, соответственно, потеряется выделенная память.
__________________________________
Варианты решения проблемы:
1. Передавать в функцию адрес указателя
C++Выделить код
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
#include <iostream>
void foo_create( int ** ptr_to_ptr )
{
    *ptr_to_ptr = new int [10];
    for ( int i = 0; i < 10; i++ ) (*ptr_to_ptr)[i] = i;
}
 
void foo_delete( int ** ptr_to_ptr )
{
    delete [] *ptr_to_ptr;
    *ptr_to_ptr = nullptr;
}
 
int main()
{
    int * Ptr;
    foo_create( &Ptr );
    for ( int i = 0; i < 10; i++ ) std::cout << Ptr[i] << "  ";
    foo_delete( &Ptr );
    std::cout  << Ptr <<  std::endl;
}
2. Передавать в функцию сылку на указатель
C++Выделить код
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
#include <iostream>
 
void foo_create( int * & Arr )
{
    Arr = new int [10];
    for ( int i = 0; i < 10; i++ ) Arr[i] = i;
}
 
void foo_delete( int * & Arr )
{
    delete [] Arr;
    Arr = nullptr;
}
 
int main()
{
    int * Ptr;
    foo_create( Ptr );
    for ( int i = 0; i < 10; i++ ) std::cout << Ptr[i] << "  ";
    std::cout <<  std::endl;
    foo_delete( Ptr );
    std::cout <<  Ptr << std::endl;
}