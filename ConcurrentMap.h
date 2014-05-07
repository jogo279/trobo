#include <unordered_map>
#include <boost/thread/mutex.hpp>

using namespace std;

class ConcurrentMap {
public:
	ConcurrentMap();
	void insert(pair<int,char> p);
	unordered_map<int,char>::const_iterator find(int i);
private:
	unordered_map<int, char> map;
	boost::mutex mutex;
}