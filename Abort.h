class Abort {
public:
    Abort(Abort * par);

    int isAborted() const;

    void abort();

private:
  Abort *parent;
  bool aborted;
  int pollGranularity;
  int count;
};