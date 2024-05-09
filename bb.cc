#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UdpHandshakeExample");

// 定义一个函数用于发送握手消息
void SendHandshake(Ptr<Socket> socket, Ipv4Address destAddr, uint16_t destPort) {
    Ptr<Packet> packet = Create<Packet>(100); // 假定握手消息大小为100字节
    socket->SendTo(packet, 0, InetSocketAddress(destAddr, destPort));
    NS_LOG_INFO(Simulator::Now().GetSeconds() << "s: 发送握手消息到 " << destAddr);
}

int main() {
    // 启用日志
    LogComponentEnable("UdpHandshakeExample", LOG_LEVEL_INFO);

    // 创建5个节点
    NodeContainer nodes;
    nodes.Create(5);

    // 创建并配置点对点链路（简化为星形拓扑）
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("1ms"));

    // 创建一个网络设备容器并为每对节点安装网络设备
    NetDeviceContainer devices;
    for (int i = 1; i < 5; ++i) { // 跳过节点0，因为它是发送方
        devices.Add(pointToPoint.Install(nodes.Get(0), nodes.Get(i)));
    }

    // 安装网络协议栈
    InternetStackHelper stack;
    stack.Install(nodes);

    // 分配IP地址
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 启动路由
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 创建UDP Socket
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    uint16_t recvPort = 4000;
    for (uint32_t i = 1; i < nodes.GetN(); i++) {
        Ptr<Socket> recvSocket = Socket::CreateSocket(nodes.Get(i), tid); // 接收方Socket
        Ipv4Address recvAddr = interfaces.GetAddress(i); // 获取正确的接收方地址
        recvSocket->Bind(InetSocketAddress(recvAddr, recvPort));
    }
    uint32_t interfaceCount = interfaces.GetN();

// 打印接口数量
    std::cout << "Number of interfaces: " << interfaceCount << std::endl;
    Ptr<Socket> sendSocket = Socket::CreateSocket(nodes.Get(0), tid); // 发送方Socket

    // 模拟向其他四个节点发送握手消息
    for (uint32_t i = 1; i < interfaces.GetN(); i += 2) { // 选择正确的目的地址
        Simulator::Schedule(Seconds(0.5 + i-1), &SendHandshake, sendSocket, interfaces.GetAddress(i), recvPort);
    }
    AnimationInterface anim("first.xml");
    // 运行仿真
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
