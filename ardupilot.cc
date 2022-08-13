#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/tap-bridge-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/netanim-module.h"
#include <ns3/packet.h>

#include "./external-mobility-model.h"
#include "./signal_power.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Ardupilot");


int
main (int argc, char *argv[])
{
    Packet::EnablePrinting ();

    std::string mode = "ConfigureLocal";
    uint32_t basePort = 5760;
    uint32_t nodeNumber = 1;
    int simulationTime = 20;
    std::string tapName ="tap-test1";
    std::string csvFileName;

    // Start location
    double lat = 41.1028715;
    double lng = 29.0240369;
    double alt = 16.87;

    CommandLine cmd;
    cmd.AddValue ("port",  "Base port to listen",basePort);
    cmd.AddValue ("number",  "UAV number to run",nodeNumber);
    cmd.AddValue ("mode", "Mode setting of TapBridge", mode);
    cmd.AddValue ("tapName", "Name of the OS tap device", tapName);
    cmd.AddValue ("time", "Simulation time", simulationTime);
    cmd.AddValue ("csv", "File name for csv export", csvFileName);

    // Start location
    cmd.AddValue ("lat", "Start Location Latitude", lat);
    cmd.AddValue ("lng", "Start Location Longitude", lng);
    cmd.AddValue ("alt", "Start Location Altitude", alt);

    cmd.Parse (argc, argv);

    std::cout << "Creating " << nodeNumber <<" nodes to listen with UDP from 10.1.1.0:" << basePort << std::endl;

    GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

    //
    // The topology has a wifi network.
    //
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (nodeNumber);

    NodeContainer wifiApNode;
    wifiApNode.Create (1);

    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());

    Ssid ssid = Ssid ("ns-3-ssid");
    WifiHelper wifi;
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager");

    NetDeviceContainer apDevices;
    wifiMac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));
    apDevices = wifi.Install (wifiPhy, wifiMac, wifiApNode);

    NetDeviceContainer staDevices;
    wifiMac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));
    staDevices = wifi.Install (wifiPhy, wifiMac, wifiStaNodes);

    NetDeviceContainer allDevices = NetDeviceContainer (apDevices, staDevices);


    MobilityHelper constantMobility, externalMobility;
    constantMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    constantMobility.Install(wifiApNode);


    InternetStackHelper stack;
    stack.Install (wifiStaNodes);
    stack.Install (wifiApNode);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign (allDevices);

    TapBridgeHelper tapBridge (interfaces.GetAddress (0));
    tapBridge.SetAttribute ("Mode", StringValue (mode));
    tapBridge.SetAttribute ("DeviceName", StringValue (tapName));
    tapBridge.Install (wifiApNode.Get (0), allDevices.Get (0));


    // Receive some data at ns-3 node
    externalMobility.SetMobilityModel ("ns3::ExternalMobilityModel",
                                       "Latitude", DoubleValue (lat),
                                       "Longitude", DoubleValue (lng),
                                       "Altitude", DoubleValue (alt)
                                       );
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

    for (uint32_t i = 0; i < nodeNumber; ++i) {
        Ptr<Node> node = wifiStaNodes.Get(i);
        externalMobility.Install(node);

        Ptr<Socket> recvSink = Socket::CreateSocket (node, tid);
        node->GetObject<ExternalMobilityModel>()->StartListen(recvSink, basePort + i);
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
        std::cout<< "Node "<< i << " is listening on " << iaddr.GetLocal () << ":" << basePort + i << std::endl;
    }

    Ptr<OutputStreamWrapper> stream;
    if (csvFileName.empty()) {
        std::cout << "No csv file name given, no csv export" << std::endl;
        stream = Create<OutputStreamWrapper> (&std::cout);
    } else {
        std::cout << "Exporting csv to " << csvFileName << std::endl;
        stream = Create<OutputStreamWrapper> (csvFileName.c_str(), std::ios::out);
    }


    AnimationInterface anim("ardupilot.xml");

    std::vector<SignalPower> deviceSignalPowers;
    deviceSignalPowers.reserve(nodeNumber);

    for (uint32_t i = 0; i < staDevices.GetN(); ++i) {
        deviceSignalPowers.push_back(SignalPower(i));

        Ptr<WifiPhy> phy = staDevices.Get(i)->GetObject<WifiNetDevice>()->GetPhy();
        phy->TraceConnectWithoutContext("MonitorSnifferRx", MakeCallback (&SignalPower::MonitorSniffRx, &deviceSignalPowers[i]) );
        Simulator::ScheduleNow(
                csvFileName.empty()
                    ? &SignalPower::StreamSignal
                    : &SignalPower::StreamSignalAsCsv,
                &deviceSignalPowers[i],
                stream
        );
    }

    Simulator::Stop (Seconds (simulationTime));
    Simulator::Run ();
    Simulator::Destroy ();
}
