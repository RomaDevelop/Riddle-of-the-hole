#include <iostream>
using namespace std;

class EasyData
{
	static const int sizePriv {20};
	int data[sizePriv];
public:
	static const int size {sizePriv};

	EasyData() 
	{ 
	for(int i=0; i<size; i++) 
		data[i] = 0; 
	}
	EasyData(int val5) 
	{ 
	EasyData(); 
	data[5] = val5; 
	}

    void Print() { for(int i=0; i<size; i++) cout << data[i] << " "; cout << endl; }	
};

int main() {
    
    cout << "Hello world!\n" <<endl;
    
    EasyData obj1;
    obj1.Print();
    
    EasyData obj2(555);
    obj2.Print();

    return 0;
}

загадка: почему в obj2 мусор???
разгадка: потому что команда EasyData(); создаёт локальный объект, а не обрабатывает текущий