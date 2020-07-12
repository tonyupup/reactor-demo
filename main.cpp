#include <iostream>
#include <memory>
#include <functional>

#include "src/server.h"
using namespace std;

class S
{
public:
    S(int f) : _m(f){};
    void f(int x)
    {
        _m += x;
    }
    friend ostream &operator<<(ostream &out, const S &s)
    {
        out << s._m << endl;
        return out;
    }

private:
    int _m;
};
using Func = void(int);

void callback(function<void(int)> f)
{
    f(2);
}
int main()
{
    auto pserver = make_shared<ServerHandle>();

    pserver->Start();
    return 0;
}
