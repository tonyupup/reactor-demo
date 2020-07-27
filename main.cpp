#include <iostream>
#include <memory>
#include <functional>
#include <cstring>
#include <algorithm>
#include <vector>
#include "src/server.h"
#include <initializer_list>
#include "src/chatRoom.h"

// Processer *process = nullptr;
using namespace std;
int main()
{
    auto pserver = make_shared<ServerHandle>();
    Chat *process = new Chat(pserver);
    process->Start();
    return 0;
}
