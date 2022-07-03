#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/netanim-module.h"
#include <ns3/packet.h>

//#include "../../src/network/model/socket.h"
//#include "../../src/core/model/log-macros-enabled.h"
//#include "../../src/network/model/packet.h"
//#include "../../src/core/model/ptr.h"
//#include "../../src/core/model/command-line.h"
//#include "../../src/core/model/global-value.h"
//#include "../../src/core/model/string.h"
//#include "../../src/core/model/boolean.h"
//#include "../../src/network/helper/node-container.h"
//#include "../../src/csma/helper/csma-helper.h"
//#include "../../src/mobility/helper/mobility-helper.h"
//#include "../../src/network/helper/net-device-container.h"
//#include "../../src/mobility/model/mobility-model.h"
//#include "../../src/core/model/vector.h"
//#include "../../src/internet/helper/internet-stack-helper.h"
//#include "../../src/internet/helper/ipv4-address-helper.h"
//#include "../../src/internet/helper/ipv4-interface-container.h"
//#include "../../src/tap-bridge/helper/tap-bridge-helper.h"
//#include "../../src/network/utils/inet-socket-address.h"
//#include "../../src/network/utils/ipv4-address.h"
//#include "../../src/network/utils/output-stream-wrapper.h"
//#include "../../src/internet/helper/ipv4-global-routing-helper.h"
//#include "../../src/netanim/model/animation-interface.h"
//#include "../../src/core/model/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Ardupilot");

Time                    m_startTime;

void ReceivePacketUdp (Ptr<Socket> socket)
{

    NS_LOG_UNCOND ("Received one packet!");

    Ptr<Packet> packet = socket->Recv (1472,0);
    std::cout<<"Packet Size:"<<packet->GetSize()<<std::endl;
    //  packet->Print (std::cout);
    //  std::cout << std::endl;
    uint8_t *buffer = new uint8_t[packet->GetSize ()-1];
    packet->CopyData(buffer, packet->GetSize ());

    std::string s = std::string((char*)buffer);
    std::cout<<"Data Received:"<<s<<std::endl;

}


void HandleAcceptTcp (Ptr<Socket> socket, const Address& from)
{
    NS_LOG_FUNCTION (from);
    socket->SetRecvCallback (MakeCallback (&ReceivePacketUdp));
}



int 
main (int argc, char *argv[])
{
    Packet::EnablePrinting ();
    //  Packet::EnableChecking ();
    std::string mode = "UseBridge";
    int port = 5760;
    bool isUdp = false;
    std::string tapName ="tap-test1";
    //  uint32_t packetSize = 1000; // bytes
    // uint32_t numPackets = 1;
    // double interval = 1.0; // seconds


    CommandLine cmd;
    cmd.AddValue ("port",  "port",port);
    cmd.AddValue ("udp", "use udp instead of default tcp",isUdp);
    cmd.AddValue ("mode", "Mode setting of TapBridge", mode);
    cmd.AddValue ("tapName", "Name of the OS tap device", tapName);
    cmd.Parse (argc, argv);

    std::cout << "Listening "<< (isUdp ? "UDP:" : "TCP:") << std::to_string(port) << std::endl;

    GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

    //
    // The topology has a csma network of two nodes on the left side.
    //
    NodeContainer nodesLeft;
    nodesLeft.Create (2);

    CsmaHelper csmaSN0;
    csmaSN0.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csmaSN0.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
    NetDeviceContainer devicesLeft = csmaSN0.Install (nodesLeft);



    MobilityHelper mobilityL;
    mobilityL.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobilityL.Install(nodesLeft);
    nodesLeft.Get(1)->GetObject<MobilityModel>()
    ->SetPosition(Vector(200.0, 200.0, 0.0));

    nodesLeft.Get(0)->GetObject<MobilityModel>()
    ->SetPosition(Vector(100.0, 200.0, 0.0));


    InternetStackHelper internetLeft;
    internetLeft.Install (nodesLeft);

    Ipv4AddressHelper ipv4Left;
    ipv4Left.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfacesLeft = ipv4Left.Assign (devicesLeft);

    TapBridgeHelper tapBridge (interfacesLeft.GetAddress (0));
    tapBridge.SetAttribute ("Mode", StringValue (mode));
    tapBridge.SetAttribute ("DeviceName", StringValue (tapName));
    tapBridge.Install (nodesLeft.Get (0), devicesLeft.Get (0));


    // Receive some data at ns-3 node

    std::string socketFactory = isUdp ? "ns3::UdpSocketFactory" : "ns3::TcpSocketFactory";
    TypeId tid = TypeId::LookupByName (socketFactory);
    Ptr<Socket> recvSink = Socket::CreateSocket (nodesLeft.Get (1), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),port);
    recvSink->Bind (local);
    recvSink->SetRecvCallback (MakeCallback (&ReceivePacketUdp));

    if(!isUdp) {
        recvSink->Listen();

        // AcceptCallback
        recvSink->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&HandleAcceptTcp)
        );
    }


    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


    AnimationInterface anim("tap-wifi-dumbbell-modified.xml");
    // Trace routing tables
    Ipv4GlobalRoutingHelper g;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (12), routingStream);

    m_startTime = Simulator::Now();
    Simulator::Stop (Seconds (600.));
    Simulator::Run ();
    Simulator::Destroy ();
}


// https://github.com/libremente/NS3-simulation/blob/master/WIFI_TCP_simulation.cc