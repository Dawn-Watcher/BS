
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include <iostream>
#include <string>
#include "node.cc"
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");
void AddNodeAndCommunicate(NodeContainer& nodes, CsmaHelper& csma, Ipv4AddressHelper& address, uint16_t port) {
    // 创建并添加新节点
    Ptr<MyCustomNode> newNode = CreateObject<MyCustomNode>();
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

void 
send(Ptr<Socket> sock,string str,Ipv4Address destAddr, uint16_t destPort)
{ 
//   uint8_t buffer[sizeof(str)] ;//也可以根据字符串大小创建包
  uint8_t buffer[255] ;
  uint32_t len = str.length();
  for(uint32_t i=0;i<len;i++)
  {
    buffer[i]=str[i];//char 与 uint_8逐个赋值
  }
  buffer[len]='\0';
  Ptr<Packet> p = Create<Packet>(buffer,sizeof(buffer));//把buffer写入到包内
  sock->SendTo(p,0,InetSocketAddress(destAddr, destPort));
  NS_LOG_INFO(Simulator::Now().GetSeconds() << "s: 发送握手消息到 " << destAddr);
} 

void
RecvString(Ptr<Socket> sock)//回调函数
{
   
    Address from;
    Ptr<Packet> packet = sock->RecvFrom (from);
    packet->RemoveAllPacketTags ();
    packet->RemoveAllByteTags ();
    InetSocketAddress address = InetSocketAddress::ConvertFrom (from);

    // uint8_t data[sizeof(packet)];
    uint8_t data[255];
    packet->CopyData(data,sizeof(data));//将包内数据写入到data内
    cout <<sock->GetNode()->GetId()<<" "<<"receive : '" << data <<"' from "<<address.GetIpv4 ()<< endl;  
    
    char a[sizeof(data)];
    for(uint32_t i=0;i<sizeof(data);i++){
        a[i]=data[i];
    }
    string strres = string(a);
    cout<<"接受到的字符串为 "<<strres<<endl;
 
}


void send_node(NodeContainer nodes,Ipv4InterfaceContainer interfaces,uint16_t send_node_id,uint16_t recv_node_id,TypeId tid,uint16_t port){
  Ptr<Socket> Send_sock = Socket::CreateSocket(nodes.Get (send_node_id), tid);
	InetSocketAddress RecvAddr = InetSocketAddress(interfaces.GetAddress(recv_node_id), port);
	Send_sock->Connect(RecvAddr);
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  uint32_t numNodes = 20;

  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (numNodes);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));


  NetDeviceContainer devices;
  devices = csma.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  uint16_t recvPort = 80;
      
  //UDP传输



  // //接收端
  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
  for (uint32_t i = 0; i < numNodes; ++i) {
        Ptr<Socket> recvSocket = Socket::CreateSocket(nodes.Get(i), tid); // 接收方Socket
        Ipv4Address recvAddr = interfaces.GetAddress(i); // 获取正确的接收方地址
        recvSocket->Bind(InetSocketAddress(recvAddr, recvPort));
        recvSocket->SetRecvCallback(MakeCallback(&RecvString));
    }
	/*Ptr<Socket> Recv_sock = Socket::CreateSocket(nodes.Get (0), tid);
	// InetSocketAddress addr = InetSocketAddress(Ipv4Address::GetAny(), 10000);
  InetSocketAddress addr = InetSocketAddress(interfaces.GetAddress(0), 10000);
	Recv_sock->Bind(addr);
  Recv_sock->SetRecvCallback(MakeCallback(&RecvString)); //设置回调函数*/
   uint32_t interfaceCount = interfaces.GetN();

// 打印接口数量
    std::cout << "Number of interfaces: " << interfaceCount << std::endl;
  //发送端
  Ptr<Socket> Send_sock = Socket::CreateSocket(nodes.Get (0), tid);
	InetSocketAddress RecvAddr = InetSocketAddress(interfaces.GetAddress(0), recvPort);
	Send_sock->Connect(RecvAddr);

  for (uint32_t i = 2; i < interfaces.GetN(); i++) { // 选择正确的目的地址
      Simulator::ScheduleWithContext(0,Seconds(1+0.1*i),&send,Send_sock,"Hello Ns3!",interfaces.GetAddress(i),recvPort); 
    }

   
  csma.EnablePcapAll ("test_packet");
  AnimationInterface anim ("test_packet.xml");
  Simulator::Run ();
  Simulator::Stop(Seconds(6));
  Simulator::Destroy ();
  return 0;
}

