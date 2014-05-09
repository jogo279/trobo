#include <atomic>
#include <climits>

class ABState {
public:
    // ABState();
    int getA() const;
    int getB() const;
    void setA(int val);
    void setB(int val);

    ABState(ABState * par, std::atomic<int> *A, std::atomic<int> *B);

    int bestA() const;
    int bestB() const;

    int isAborted() const;
    // std::tuple<int,int,bool> bestAB() const;

    void abort();  

private:
    std::atomic<int> *a;
    std::atomic<int> *b;  

	ABState *parent;
	bool aborted;
	int pollGranularity;
	int count;
};