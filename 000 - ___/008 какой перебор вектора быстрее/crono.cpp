#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

int main() {
	// подготовка измерителей
    auto start_time = chrono::steady_clock::now();
    auto end_time = chrono::steady_clock::now();
    auto elapsed = end_time - start_time;
    string elapsed_str;
    
	// подготовка измеряемых
    vector<int> vect;
    long count = 30'000'000;
    vect.resize(count);
    
    for(int i=0; i<5; i++)
    {
        cout<< "Round " <<i+1<<"! Fight!!!\n"; 
		
		// test 1
    	start_time = chrono::steady_clock::now();
		// start
        for(auto &v:vect)
            v = 11;
    	// finish
    	end_time = chrono::steady_clock::now();
    	elapsed = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time);
    	elapsed_str = to_string(elapsed.count());
    	for(int i=elapsed_str.size()-1, n=1; i>=0; i--, n++) if(n%3 == 0 && i!=0) elapsed_str.insert(i," ");
    	cout << elapsed_str << endl;
		// end test
    	
		// test 2
    	start_time = chrono::steady_clock::now();
		// start
        for(long i=0; i<vect.size(); i++)
            vect[i] = 10;
		// finish
    	end_time = chrono::steady_clock::now();
    	elapsed = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time);
    	elapsed_str = to_string(elapsed.count());
    	for(int i=elapsed_str.size()-1, n=1; i>=0; i--, n++) if(n%3 == 0 && i!=0) elapsed_str.insert(i," ");
    	cout << elapsed_str << endl;
    	// end test
		
    	start_time = chrono::steady_clock::now();
    	// start
        for(long i=0; i<count; i++)
            vect[i] = 500;
    	// finish
    	end_time = chrono::steady_clock::now();
    	elapsed = chrono::duration_cast<chrono::nanoseconds>(end_time - start_time);
    	elapsed_str = to_string(elapsed.count());
    	for(int i=elapsed_str.size()-1, n=1; i>=0; i--, n++) if(n%3 == 0 && i!=0) elapsed_str.insert(i," ");
    	cout << elapsed_str << endl;
		// end test
    }
    return 0;
}