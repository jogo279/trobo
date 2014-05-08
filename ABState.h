#include <tuple>

class ABState {
public:
    ABState();
    int getA() const;
    int getB() const;
    void setA(int val);
    void setB(int val);

    ABState(ABState * par);

    int isAborted() const;
    std::tuple<int,int,bool> bestAB() const;

    void abort();

private:
    int a;
    int b;

	ABState *parent;
	bool aborted;
	int pollGranularity;
	int count;
};