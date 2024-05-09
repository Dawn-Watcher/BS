#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Node0ToAllCommunication");

void ReceivePacket (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> packet;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          InetSocketAddress addr = InetSocketAddress::ConvertFrom (from);
          NS_LOG_UNCOND ("Received one packet from " << addr.GetIpv4 ());
          
          // 检查是否来自节点0
          if (addr.GetIpv4 () == Ipv4Address ("10.1.1.1"))
            {
              // 向节点0发送回应
              uint32_t packetSize = packet->GetSize ();
              Ptr<Packet> replyPacket = Create<Packet> (packetSize);
              socket->SendTo (replyPacket, 0, from); // 使用 from 地址回复
            }
        }
    }
}


static void GenerateTraffic (Ptr<Socket> socket, Address destAddress, uint32_t pktSize, Time pktInterval)
{
  Ptr<Packet> packet = Create<Packet> (pktSize);
  socket->SendTo (packet, 0, destAddress);
}

int main (int argc, char *argv[])
{
  uint32_t numNodes = 20;
  uint32_t packetSize = 1024; // bytes
  double interval = 1.0; // seconds

  Time interPacketInterval = Seconds (interval);

  NodeContainer nodes;
  nodes.Create (numNodes);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer devices = csma.Install (nodes);

  InternetStackHelper internet;
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  // 设置接收socket
  for (uint32_t i = 0; i < numNodes; ++i)
    {
      Ptr<Socket> recvSink = Socket::CreateSocket (nodes.Get (i), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
      recvSink->Bind (local);
      recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));
    }

  // 节点0向每个节点发送消息，并设置接收回应
  for (uint32_t j = 1; j <= numNodes; ++j)
    {
      Ptr<Socket> sourceNode0 = Socket::CreateSocket (nodes.Get (0), tid);
      InetSocketAddress remote = InetSocketAddress (i.GetAddress (j % numNodes), 80);
      Simulator::ScheduleWithContext (sourceNode0->GetNode ()->GetId (),
                                      Seconds (2.0 + j*0.1), // 为了避免消息同时到达，稍微错开发送时间
                                      &GenerateTraffic,
                                      sourceNode0, remote, packetSize, interPacketInterval);
    }
    AnimationInterface anim ("ss.xml");
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
