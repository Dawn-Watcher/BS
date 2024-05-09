#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "node.cc"  // Include the header file for your custom application


using namespace ns3;

int main() {
    Ptr<MyCustomNode> node = CreateObject<MyCustomNode>();
    node->SetId(1);
    node->SetName("Node1");

    std::cout << *node << std::endl;

    return 0;
}

