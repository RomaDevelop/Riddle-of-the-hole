#include <iostream>
using namespace std;

struct Widget {
  explicit Widget(int) { cout << "explicit Widget(int)\n"; }
  Widget(double) { cout << "Widget(double)\n"; }
};

struct Thingy {
  Widget w1, w2;
};

int main() {
  Thingy thingy1 {3, 4}; // вызывает Widget(double)
  Thingy thingy3 {Widget(3), Widget(4)};  // explicit Widget(int) 
}
