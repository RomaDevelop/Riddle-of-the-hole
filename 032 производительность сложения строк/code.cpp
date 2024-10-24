#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <chrono>
#include <string>
using namespace std;

long long get_nanosec_duration(auto &start_time)
{
	auto end_time = chrono::steady_clock::now();
	auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time);
	return elapsed.count();
}

void print_result(auto &start_time, string prefix = "")
{
	string elapsed_str = to_string(get_nanosec_duration(start_time));
	for(int i=elapsed_str.size()-1, n=1; i>=0; i--, n++) if(n%3 == 0 && i!=0) elapsed_str.insert(i," ");
	if(!prefix.empty())
		cout << prefix+" ";
	cout << elapsed_str << endl;
}

int main() {

	string s = "sddsd";
	string f = "sddsd";
	string x = "sddsd";

	for(int t=0; t<10; t++)
	{
		auto start_time = chrono::steady_clock::now();
		for(int i=0; i< 3'000'000; i++)
		{
			string str = s + f + x;
			//string str = s
			//str += f;
			//str += x;
		}
		print_result(start_time);
	}

	for(int t=0; t<10; t++)
	{
		auto start_time = chrono::steady_clock::now();
		for(int i=0; i< 3'000'000; i++)
		{
			string str = s;
			str += f;
			str += x;
		}
		print_result(start_time);
	}

	return 0;
}
