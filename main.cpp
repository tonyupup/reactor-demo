#include <iostream>
#include <memory>
#include <functional>
#include <cstring>
#include <algorithm>
#include <vector>
#include "src/server.h"
#include <initializer_list>
#include "src/chatRoom.h"
#include <sys/signal.h>

// Processer *process = nullptr;
using namespace std;
static shared_ptr<ServerHandle> pserver;
void ps(int)
{
    pserver->Stop();
};
int main()
{

    pserver = make_shared<ServerHandle>();
    signal(SIGINT, ps);
    Chat *process = new Chat(pserver);
    process->Start();
    return 0;
}
