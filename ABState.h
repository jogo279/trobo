#include <atomic>
#include <climits>

class ABState {
public:


    ABState(ABState * par);



    int isAborted() const;

    void abort();  

private:

	ABState *parent;
	bool aborted;
};