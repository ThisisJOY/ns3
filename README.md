# ns3
  This is a simple example to test TCP over 802.11b.
  Use the following command to run the default configuration:
  
  ./waf --run tcp-80211b.cc
  
  In this example, n wifi stations send TCP packets to the access point. 
  We report the total throughput received by the access point during simulation time. 
  
  The user can change the following parameters through command line:
   1. the number of STA nodes 
   (Example: ./waf --run "tcp-80211b --nWifi=50"),
   2. the payload size 
   (Example: ./waf --run "tcp-80211b --payloadSize=2000"),
   3. application data rate 
   (Example: ./waf --run "tcp-80211b --dataRate="1Mbps""),
   4. variant of TCP i.e. congestion control algorithm to use 
   (Example: ./waf --run "tcp-80211b --tcpVariant="TcpTahoe""),
   5. physical layer transmission rate, i.e. four different data rates of 1, 2, 5.5 or 11 Mbps
   (Example: ./waf --run "tcp-80211b --"DsssRate5_5Mbps""),
   6. simulation time 
   (Example: ./waf --run "tcp-80211b --simulationTime=10"),
   7. enable/disable pcap tracing 
   (Example: ./waf --run "tcp-80211b --pcapTracing=true").
