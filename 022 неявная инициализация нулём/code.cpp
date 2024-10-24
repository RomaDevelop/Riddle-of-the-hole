#include <iostream>
using namespace std;

struct Widget {
  int i;
  int j;
};

int main() {
  Widget widget = {1};  // что получим?
  cout << widget.j; 
  return 0;
}


#include <iostream>
using namespace std;

struct Widget {
  int i;
  int j;
};

struct Thingy {
  Widget w;
  int k;
};

int main() {
  Thingy t1 = {1, 2};   // что получим?
  Thingy t2 = {{1}, 2}; // а так?
  cout<< t1.k << endl ; 
  cout<< t2.k << endl ; 

  return 0;
}
