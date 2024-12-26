// если определено const T operator+ (const T& left, const T& right)
// то можно ли сделать T t2 = t0 + t1; ? И что при этом происходит?

// да можно, и ничего лишнего не происходит,
// потому что const r-value и оно и должно нормально записываться

// а вот если вызов t2 = t0 + t1 для ранее созданного t2 происходит вызов operator=, а затем уничтожение временного объекта t0 + t1

#include <iostream>
#include <vector>
#include <type_traits>
using namespace std;

int counter = 0;
class T
{
	int index = -1;
public:
	int val = -1;
	T() {
		index = counter;
		counter++;
		cout << "T() " << index << " " << val << " " << this << "\n"; }



	T(const T& src) {
		index = counter;
		val = src.val;
		counter++;
		cout << "copy T " << index << " " << val << " " << this << "\n";
	}
	T(T&& src) {
		index = counter;
		val = src.val;
		counter++;
		cout << "move T " << index << " " << val << " " << this << "\n"; }

	void add() {val++;}

	T& operator=(const T& t)
	{
		cout << "operator=(const T& t)\n";
		index = t.index;
		val = t.val;
		return *this;
	};
	T& operator=(T&& t)
	{
		cout << "operator=(T&& t)\n";
		index = t.index;
		val = t.val;
		t.index = -1;
		t.val = -1;
		return *this;
	};


	~T() {
		cout << "~T() " << index << " " << val << " " << this << "\n";
	}
};

const T operator+ (const T& left, const T& right)
{
	cout << "start cr in op" << endl;
	T tmp;
	cout << "end cr in op" << endl;
	return tmp;
}

int main() {


	T t0;
	T t1;

	t0.val = 0;
	t1.val = 1;

	T t2 = t0 + t1;
	t2.val = 2;

	t2 = t0 + t1;


	cout << "\nend\n\n";
	return 0;
}
