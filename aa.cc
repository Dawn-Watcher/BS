#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DynamicNetworkExample");

// 函数原型声明
void SetupCommunication(NodeContainer& nodes, uint16_t port, CsmaHelper& csma, InternetStackHelper& stack);
void NewNodeCommunication(NodeContainer& nodes, Ipv4AddressHelper& address, uint16_t port, CsmaHelper& csma, InternetStackHelper& stack);

int main () {
    // 日志级别设置
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 创建10个初始节点
    NodeContainer nodes;
    nodes.Create(10);

    // 创建CSMA通道并安装设备和协议栈
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("1Gbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer devices = csma.Install(nodes);

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 设置端口号
    uint16_t port = 9; 

    // 设置节点间的通信
    SetupCommunication(nodes, port, csma, stack);
    //AnimationInterface需要设置所有节点的位置，通过SetConstantPosition(Ptr<Node> n,double x, double y);在移动模型中设置节点静态位置的x、y坐标
        AnimationInterface anim("first.xml");
    // 在模拟开始10秒后添加新节点并与其他节点通信
    Simulator::Schedule(Seconds(2.40), &NewNodeCommunication, std::ref(nodes), std::ref(address), port, std::ref(csma), std::ref(stack));

    Simulator::Stop(Seconds(3.0));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

void SetupCommunication(NodeContainer& nodes, uint16_t port, CsmaHelper& csma, InternetStackHelper& stack) {
    // 每个节点既是客户端也是服务器，用于相互通信
    for (uint32_t i = 0; i < nodes.GetN(); ++i) {
        UdpEchoServerHelper echoServer(port);
        echoServer.Install(nodes.Get(i));

        for (uint32_t j = 0; j < nodes.GetN(); ++j) {
            if (i != j) {
                UdpEchoClientHelper echoClient(nodes.Get(j)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port);
                echoClient.SetAttribute("MaxPackets", UintegerValue(1));
                echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
                echoClient.SetAttribute("PacketSize", UintegerValue(1024));

                ApplicationContainer clientApp = echoClient.Install(nodes.Get(i));
                clientApp.Start(Seconds(2.0));
            }
        }
    }
}

void NewNodeCommunication(NodeContainer& nodes, Ipv4AddressHelper& address, uint16_t port, CsmaHelper& csma, InternetStackHelper& stack) {
    NS_LOG_INFO("Adding new node and initiating communication with existing nodes.");

    // 添加新节点
    Ptr<Node> newNode = CreateObject<Node>();
    nodes.Add(newNode);
    NetDeviceContainer device = csma.Install(newNode);
    stack.Install(newNode);
    Ipv4InterfaceContainer iface = address.Assign(device);

    // 新节点与每一个原有节点通信
    for (uint32_t i = 0; i < nodes.GetN() - 1; ++i) { // 排除新节点自身
        UdpEchoClientHelper echoClient(nodes.Get(i)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port);
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(4)); // 传输内容为"1111"
        ApplicationContainer clientApp = echoClient.Install(newNode);
        clientApp.Start(Seconds(11.0 + i)); // 保证依次通信
    }
}
