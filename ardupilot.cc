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

#include "./external-mobility-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Ardupilot");


int 
main (int argc, char *argv[])
{
    Packet::EnablePrinting ();
    //  Packet::EnableChecking ();
    std::string mode = "UseBridge";
    uint32_t basePort = 5760;
    uint32_t nodeNumber = 1;
    std::string tapName ="tap-test1";
    //  uint32_t packetSize = 1000; // bytes
    // uint32_t numPackets = 1;
    // double interval = 1.0; // seconds


    CommandLine cmd;
    cmd.AddValue ("port",  "Base port to listen",basePort);
    cmd.AddValue ("number",  "UAV number to run",nodeNumber);
    cmd.AddValue ("mode", "Mode setting of TapBridge", mode);
    cmd.AddValue ("tapName", "Name of the OS tap device", tapName);
    cmd.Parse (argc, argv);

    std::cout << "Creating " << nodeNumber <<" nodes to listen with UDP from 10.1.1.0:" << basePort << std::endl;

    GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

    //
    // The topology has a csma network.
    //
    NodeContainer nodesLeft;
    nodesLeft.Create (nodeNumber + 1);

    CsmaHelper csmaSN0;
    csmaSN0.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csmaSN0.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
    NetDeviceContainer devicesLeft = csmaSN0.Install (nodesLeft);


    MobilityHelper constantMobility, externalMobility;
    constantMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    constantMobility.Install(nodesLeft.Get(0));


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
    externalMobility.SetMobilityModel ("ns3::ExternalMobilityModel");
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

    for (uint32_t i = 0; i < nodeNumber; ++i) {
        Ptr<Node> node = nodesLeft.Get(i + 1);
        externalMobility.Install(node);

        Ptr<Socket> recvSink = Socket::CreateSocket (node, tid);
        node->GetObject<ExternalMobilityModel>()->StartListen(recvSink, basePort + i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
        std::cout<< "Node "<< i << " is listening on " << iaddr.GetLocal () << ":" << basePort + i << std::endl;
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


    AnimationInterface anim("tap-wifi-dumbbell-modified.xml");
    // Trace routing tables
    Ipv4GlobalRoutingHelper g;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (12), routingStream);

    Simulator::Stop (Seconds (600.));
    Simulator::Run ();
    Simulator::Destroy ();
}
