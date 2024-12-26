    
// no progress
	for(int i=0; i<count; i++)
    {
        test = i;
    }  // 1.781  1.784  1.767  1.773  1.816

// % progress
    for(int i=0; i<count; i++)
    {
        if(i%percent == 0) cout << i << " did\n";
        test = i;
    }  // 3.640  3.569  3.556  3.667  3.575


// best!!! i >= curStep
#include <iostream>
using namespace std;

int main() {
    
    int count = 1000'000'000;
    int step = count / 10;
    int curStep = step;
    int test;
    
    for(int i=0; i<count; i++)
    {
        if(i >= curStep) 
            {
                curStep += step;
                cout << i << " did\n";
            }
        test = i;
    }  // 1.873  1.214  1.896  1.872  1.871

    return 0;
}

 // huita polnaya, bolshe 10 sec
class Progresser 
{
	int countOperations {0};
	int countSteps {0};
	list<int> stepValues;
	bool empty {true};

public:
	Progresser() = default;
	Progresser(uint countOperations_, uint countSteps_) { Init(countOperations_, countSteps_); }
	void Init(uint countOperations_, uint countSteps_)
	{
		countOperations = countOperations_;
		countSteps  = countSteps_;
		stepValues.clear();
		empty = true;

		int shag = countOperations_ / countSteps_;
		if(!shag) shag = 1;
		for(uint i=shag; i<=countOperations_; i+=shag)
		{
			stepValues.push_back(i);
			empty = false;
		}
	}

	inline bool CheckProgress(int currentOperation)
	{
		if(!empty && currentOperation >= stepValues.front())
		{
			stepValues.pop_front();
			empty = stepValues.empty();
			return true;
		}
		return false;
	}
};

int main() {
    
    int count = 1000'000'000;
    int test;
    
	Progresser pr(count, 10);
	
    for(int i=0; i<count; i++)
    {
        if(pr.CheckProgress(i)) cout << i << " did\n";
        test = i;
    } 
	
    return 0;
}

// poluchshe, but loses to %

#include <iostream>
using namespace std;

class Progresser
{
	int countOperations {0};
	int countSteps {0};
	int step {0};
	int curStep {0};

public:
	Progresser() = default;
	Progresser(uint countOperations_, uint countSteps_) { Init(countOperations_, countSteps_); }
	void Init(uint countOperations_, uint countSteps_)
	{
		countOperations = countOperations_;
		countSteps  = countSteps_;
    	step = countOperations / countSteps;
    	if(!step) step = 1;
    	curStep = step;
	}

	inline bool CheckProgress(int currentOperation)
	{
		if(currentOperation >= curStep)
		{
            curStep += step;
			return true;
		}
		return false;
	}
};

int main() {
    
    int count = 1000'000'000;
    int test;
    
	Progresser pr(count, 10);
	
    for(int i=0; i<count; i++)
    {
        if(pr.CheckProgress(i)) cout << i << " did\n";
        test = i;
    } 

    return 0;
}