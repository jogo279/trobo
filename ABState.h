class ABState {
public:
    ABState();
    int getA() const;
    int getB() const;
    void setA(int val);
    void setB(int val);


private:
    int a;
    int b;
};