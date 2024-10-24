#include <iostream>
#include <vector>
using namespace std;

struct CountFruits
{
    int apples {0};
    int bananas {0};
    int oranges {0};
    int limons {0};

    const int size {4};
    vector<int*>allValues { &apples,&bananas,&oranges,&limons };
    
    int CountAll()
    {
   	    int ret = 0;
   	    for(int i=0; i<size; i++)
   		    ret += *allValues[i];
   	    return ret;
    }
    

};

// чего нехватает в такой системе хранения указателей на собственные поля?
    // нужно обязательно определить конструктор копирования и перемещенияесли
    // если этого не сделать allValues нового будут ссылаться на поля старого
    /*
    CountFruits() = default;
    CountFruits(CountFruits &cd)
    {
    	allValues = { &apples,&bananas,&oranges,&limons };
    	for(int i=0; i<size; i++)
        	*allValues[i] = *cd.allValues[i];
    }
	*/


int main() {
    
	CountFruits count;
	count.apples = 10;
	cout << count.CountAll() << endl;
    
	CountFruits tmp_count = count;
	tmp_count.apples += 200;
	cout << tmp_count.CountAll() << endl;
    
	return 0;
}
