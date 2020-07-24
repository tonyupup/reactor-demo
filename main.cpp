#include <iostream>
#include <memory>
#include <functional>
#include <cstring>
#include <algorithm>
#include <vector>
#include "src/server.h"
#include <initializer_list>

// #include "src/server.h"
using namespace std;
int main()
{

    auto pserver = make_shared<ServerHandle>();
    pserver->Start();
    return 0;
}
