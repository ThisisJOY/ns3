/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Author: Qiaoqiao Li <joyinbritish@me.com>
 *
 * This is a simple example to test TCP over 802.11b.
 * Use the following command to run the default configuration:
 * ./waf --run tcp-80211b.cc
 * 
 * In this example, n wifi stations send TCP packets to the access point. 
 * We report the total throughput received by the access point during simulation time. 
 * 
 * The user can change the following parameters through command line:
 * 1. the number of STA nodes (Example: ./waf --run "tcp-80211b --nWifi=50"),
 * 2. the payload size (Example: ./waf --run "tcp-80211b --payloadSize=2000"),
 * 3. application data rate (Example: ./waf --run "tcp-80211b --dataRate="1Mbps""),
 * 4. variant of TCP i.e. congestion control algorithm to use 
 *    (Example: ./waf --run "tcp-80211b --tcpVariant="TcpTahoe""),
 * 5. physical layer transmission rate, i.e. four different data rates of 1, 2, 5.5 or 11 Mbps
 *    (Example: ./waf --run "tcp-80211b --"DsssRate5_5Mbps""),
 * 6. simulation time (Example: ./waf --run "tcp-80211b --simulationTime=10"),
 * 7. enable/disable pcap tracing (Example: ./waf --run "tcp-80211b --pcapTracing=true").
 *
 * Network topology:
 *
 *   STA        AP     
 *   *          *       
 *   |          |       
 *   nWifi      nWifi+1   
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include <vector> 

NS_LOG_COMPONENT_DEFINE ("tcp-80211b");

using namespace ns3;

int
main(int argc, char *argv[])
{
  double throughput = 0;
  uint32_t totalPacketsThrough = 0;

  uint32_t nWifi = 50;
  uint32_t payloadSize = 1024;                       /* Transport layer payload size in bytes. */
  std::string dataRate = "100Mbps";                  /* Application layer datarate. */
  std::string tcpVariant = "ns3::TcpNewReno";        /* TCP variant type. */
  std::string phyRate = "DsssRate11Mbps";            /* Physical layer bitrate. */
  double simulationTime = 1;                         /* Simulation time in seconds. */
  bool pcapTracing = false;                          /* PCAP Tracing is enabled or not. */

  /* Command line argument parser setup. */
  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of STA nodes", nWifi);
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("dataRate", "Application data rate", dataRate);
  cmd.AddValue ("tcpVariant", "Transport protocol to use: TcpTahoe, TcpReno, TcpNewReno, TcpWestwood, TcpWestwoodPlus ", tcpVariant);
  cmd.AddValue ("phyRate", "Physical layer bitrate: DsssRate11Mbps, DsssRate5_5Mbps, DsssRate2Mbps, DsssRate1Mbps ", phyRate);  
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("pcap", "Enable/disable PCAP Tracing", pcapTracing);
  cmd.Parse (argc, argv);

  /* No fragmentation */
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("999999"));
  
  /* Disable RTS/CTS */
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("999999"));
  /* Enable RTS/CTS for frames larger than 1000 */
  //Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("1000"));


  /* Configure TCP Options */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));

  WifiMacHelper wifiMac;
  WifiHelper wifiHelper;
  wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211b);

  /* Set up Legacy Channel */
  YansWifiChannelHelper wifiChannel ;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));

  /* Setup Physical Layer */
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.Set ("TxPowerStart", DoubleValue (10.0));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (10.0));
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1));
  wifiPhy.Set ("TxGain", DoubleValue (0));
  wifiPhy.Set ("RxGain", DoubleValue (0));
  wifiPhy.Set ("RxNoiseFigure", DoubleValue (10));
  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-79));
  wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-79 + 3));
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode", StringValue (phyRate),
                                      "ControlMode", StringValue (phyRate));

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  /* Configure AP */
  Ssid ssid = Ssid ("network");
  wifiMac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevice;
  apDevice = wifiHelper.Install (wifiPhy, wifiMac, wifiApNode);

  /* Configure STA */
  wifiMac.SetType ("ns3::StaWifiMac",
                    "Ssid", SsidValue (ssid),
                    "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifiHelper.Install (wifiPhy, wifiMac, wifiStaNodes);

  /* Mobility model */
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-500, 500, -500, 500)),
                              "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=2]"),
                              "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"));

  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);


  /* Internet stack */
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterface;
  StaInterface = address.Assign (staDevices);
  Ipv4InterfaceContainer ApInterface;
  ApInterface = address.Assign (apDevice);

  /* Populate routing table */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* Install TCP Receiver on the access point */
  uint16_t port = 50000;
  Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
      
  ApplicationContainer sinkApp = packetSinkHelper.Install (wifiApNode.Get (0));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (simulationTime + 1));

  /* Install TCP Transmitter on the stations */
  OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());
  onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  onoff.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
      
  AddressValue remoteAddress (InetSocketAddress (ApInterface.GetAddress (0), port));
  onoff.SetAttribute ("Remote", remoteAddress);
      
  ApplicationContainer apps;
  apps.Add (onoff.Install (wifiStaNodes));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (simulationTime + 1));

  /* Enable Traces */
  if (pcapTracing)
    {
      wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
      wifiPhy.EnablePcap ("AccessPoint", apDevice);
      wifiPhy.EnablePcap ("Station", staDevices);
    }

  /* Start Simulation */
  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();
  Simulator::Destroy ();

  totalPacketsThrough = DynamicCast<PacketSink> (sinkApp.Get (0))->GetTotalRx ();
  throughput = totalPacketsThrough * 8 / (simulationTime * 1000000.0);
  
  std::cout << "\nThroughtput: " << throughput << " Mbit/s" << std::endl;
  return 0;
}
