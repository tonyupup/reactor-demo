#include <iostream>
#include <memory>
#include <functional>
#include <type_traits>
#include "server.h"
#include <boost/optional.hpp>


using namespace std;
#define CONCETION(x,y) x##y
#define CONCET(x,y) CONCETION(x,y)
#define DEFER(func) auto CONCET(_defer_,__LINE__) = std::shared_ptr<void *>(0,func)
int main()
{

    auto pserver = new ServerHandle();
    DEFER([&](void *){
        delete pserver;
    });
    pserver->Start();
    return 0;
}
