#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/csma-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("UdpHandshakeExample");

void AddNodeAndCommunicate(NodeContainer& nodes, CsmaHelper& csma, Ipv4AddressHelper& address, uint16_t port) {
    // 创建并添加新节点
    Ptr<Node> newNode = CreateObject<Node>();
    nodes.Add(newNode);

    // 安装网络设备和协议栈
    NetDeviceContainer newDevice = csma.Install(newNode);
    InternetStackHelper stack;
    stack.Install(newNode);

    // 分配IP地址
    Ipv4InterfaceContainer newInterface = address.Assign(newDevice);

    // 为新节点创建Socket并发送消息
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    Ptr<Socket> sendSocket = Socket::CreateSocket(newNode, tid);
    InetSocketAddress remoteAddr = InetSocketAddress(Ipv4Address("255.255.255.255"), port);
    sendSocket->SetAllowBroadcast(true);
    sendSocket->Connect(remoteAddr);

    // 发送一条消息
    std::string message = "Hello from new node!";
    Ptr<Packet> packet = Create<Packet>((uint8_t*)message.c_str(), message.length());
    sendSocket->Send(packet);

    // 输出信息
    NS_LOG_INFO("New node added and sent a message.");
}
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

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer devices = csma.Install(nodes);

  

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

    
    
    Simulator::Schedule(Seconds(1), &AddNodeAndCommunicate, nodes, csma,address, 80);
    AnimationInterface anim("first.xml");
    // 运行仿真
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}