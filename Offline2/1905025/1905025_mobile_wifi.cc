#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/command-line.h"
#include <filesystem>

// topology
/*
           10.1.3.0            point to point  
        n7    n6    n5     n0-------------------  n1    n2    n3    n4  
                           Ap        10.1.1.0    Ap      |    |      |
        *      *     *                                   *    *      *
                                                             10.1.2.0

*/ 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MobileWifiExample");

// global variables
uint32_t totalRxPackets = 0;
uint32_t totalTxPackets = 0;
uint64_t totalRxBytes = 0;
uint64_t totalTxBytes = 0;
double totalSimulationtime = 10.0;

// file variables
std::ofstream writeThroughput;
std::ofstream writePacketDeliveryRatio;

void 
createFile(std::string argName){
    std::string dir1, dir2;
    dir1 = "scratch/files/mobile/" + argName + "/throughput/";
    dir2 = "scratch/files/mobile/" + argName + "/pktRatio/";

    std::filesystem::create_directories(dir1);
    std::filesystem::create_directories(dir2);
    

    writeThroughput.open(dir1+"sThroughput.txt", std::ios_base::app);
    writePacketDeliveryRatio.open(dir2+"sPktRatio.txt", std::ios_base::app);
}

double 
calculateThroughput(){
    double throughput = (totalRxBytes * 8.0) / totalSimulationtime / 1e3; // Kbps
    std::cout<<"throughput: "<<throughput<<" Kbps\n";
    return throughput;
}

double 
calculatePacketDeliveryRatio(){
    double ratio = totalRxPackets /(double)totalTxPackets;
    std::cout<<"packetDelivaryRatio: "<<ratio<<"\n";
    return ratio;
}

void 
tracePacketReceive(Ptr<const Packet> packet, const Address &address ){
    totalRxBytes += packet->GetSize();  // get the size of bytes of the packet
    totalRxPackets ++;                  // increase total received packets
}

void 
tracePacketSend(Ptr<const Packet> packet){
    totalTxPackets ++;
    totalTxBytes += packet->GetSize();
}

int main(int argc, char* argv[]){
    int totalNodes = 40;  
    int totalFlows = 20;
    int packetsPerSecond = 100;
    int speedOfNodes = 5;   // only in wireless mobile topology(ms^-1)

    CommandLine cmd(__FILE__);
    cmd.AddValue("nodes", "Total nodes in the topology: ", totalNodes);
    cmd.AddValue("flows", "Total no of flows in the topology", totalFlows);
    cmd.AddValue("packets", "Total packets per second", packetsPerSecond);
    cmd.AddValue("speed", "Speed of Nodes in ms-1 for mobile wifi network", speedOfNodes);
    cmd.Parse(argc, argv);

    std::string argName;
    if(argc == 2){
        for(int i = 2; i<(int)strlen(argv[1]); i++){
            if(argv[1][i] == '=') break;
            argName+= argv[1][i];
        }
    }

    createFile(argName);       // create files for storing the data
    
    // LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

    int sNodes = (totalNodes)/2;  // no of nodes in both sides

/****************************Nodes***************************/
    NodeContainer bottleNeckNodes;
    bottleNeckNodes.Create(2);

    NodeContainer receiverNodes;
    receiverNodes.Add(bottleNeckNodes.Get(1));   // n1
    receiverNodes.Create(sNodes);    // nodes + the accesspoint

    NodeContainer senderNodes;
    senderNodes.Add(bottleNeckNodes.Get(0));    // n0
    senderNodes.Create(sNodes);

    NodeContainer receiverApNode = receiverNodes.Get(0); // the first node is the access point
    NodeContainer senderApNode = senderNodes.Get(0);
    
/***********************Set up Bottleneck************************/
    PointToPointHelper bottleNeckHelper;
    NetDeviceContainer bottleneckDevices;    
    bottleNeckHelper.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    bottleNeckHelper.SetChannelAttribute("Delay", StringValue("2ms"));
    bottleneckDevices = bottleNeckHelper.Install(bottleNeckNodes);        // n0 ----- n1

/***************************Set up Wifi channel and phy***********************************/

    YansWifiChannelHelper senderChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper senderPhy;
    senderPhy.SetChannel(senderChannel.Create());

    YansWifiChannelHelper receiverChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper receiverPhy;
    receiverPhy.SetChannel(receiverChannel.Create());

/******************************MAC and Wifi************************************************/   
    
    WifiMacHelper senderMac, receiverMac;
    Ssid ssid = Ssid("ssid");
    
    senderMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    receiverMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));

    WifiHelper senderWifi, receiverWifi;

    NetDeviceContainer senderDevices, receiverDevices;
    senderDevices = senderWifi.Install(senderPhy, senderMac,senderNodes );
    receiverDevices = receiverWifi.Install(receiverPhy,receiverMac, receiverNodes);

    NetDeviceContainer senderApDevies, receiverApDevices;
    senderMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    receiverMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    senderApDevies = senderWifi.Install(senderPhy, senderMac, senderApNode);
    receiverApDevices = receiverWifi.Install(receiverPhy, receiverMac, receiverApNode );

/*********************Set up Mobility*********************************/  
    
    // nodes move based on a time interval.
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Mode", StringValue("Time"));
    
    // after 1s each of the nodes will move
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Time", StringValue("1s"));
    
    std::string speedString = "ns3::ConstantRandomVariable[Constant="+ std::to_string(speedOfNodes) +"]";
    // after 1s each node will move "speedOfNodes" unit
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Speed", StringValue(speedString));
    
    // the lowest point of the Rectangle(0,0) highest(200,200)
    Config::SetDefault("ns3::RandomWalk2dMobilityModel::Bounds", StringValue("0|200|0|200"));
    
    // make the access point static
    MobilityHelper mobilityHelper;
    mobilityHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");   
    mobilityHelper.Install(senderApNode);
    mobilityHelper.Install(receiverApNode);

    // copied from "main-random-walk.cc"
    mobilityHelper.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                  "X",
                                  StringValue("100.0"),
                                  "Y",
                                  StringValue("100.0"),
                                  "Rho",
                                  StringValue("ns3::UniformRandomVariable[Min=0|Max=30]"));

    // make both the sender and receiver nodes mobile
    mobilityHelper.SetMobilityModel("ns3::RandomWalk2dMobilityModel");
    mobilityHelper.Install(senderNodes);
    mobilityHelper.Install(receiverNodes);

/*********************install protocol*************/
    InternetStackHelper stack;
    stack.Install(senderNodes);
    stack.Install(receiverNodes);
    stack.Install(senderApNode);
    stack.Install(receiverApNode);

/***********************Assign Ip Address***************************/

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    
    Ipv4InterfaceContainer bottleneckInterfaces;    
    bottleneckInterfaces = address.Assign(bottleneckDevices);
    
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer senderInterFaces, senderApInterface;     // set interface for the devices on the right
    senderInterFaces = address.Assign(senderDevices);
    senderApInterface = address.Assign(senderApDevies);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer receiverInterfaces, receiverApInterface;     // set interface for the devices on the right
    receiverInterfaces = address.Assign(receiverDevices);
    receiverApInterface = address.Assign(receiverApDevices);

/************************Set up OnOff and PacketSink Application*********************************/
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));
    
    // server will be created in the receiver side
    // so PacketSinkApplication will be created in the receiver side
    
    // client sends packets to the server
    // so OnOffApplication will be created in the sender side

    // create a set of flows from the sender(OnOffApp) to the receiver(PacketSink)
    // create a distinct flow from a senderNode to a receiverNode
    // each flow should start at a different time stamp
    
    uint32_t port = 9;   
    int senderId = 0, receiverId = 0;
    ApplicationContainer receiverApps, senderApps;
    double  intervalBetPackets = 0.5; //simulationTime = 10.0,      // in seconds

    for(uint32_t flow = 0; flow < (uint32_t)totalFlows; flow++){
        port += flow;       // At this Port the server will wait for clients

        // PacketSinkHelper(protocol name, address of the node that acts as the sink)
        PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress(receiverInterfaces.GetAddress(receiverId), port));
        receiverApps.Add(sinkHelper.Install(receiverNodes.Get(receiverId)));
        receiverApps.Get(flow)->SetStartTime(Seconds(1.0));

        // std::cout<<"for Flow : "<< flow << " sender: "<< senderInterFaces.GetAddress(senderId) << " receiver: "<< receiverInterfaces.GetAddress(receiverId) << "\n";
        
        
        // OnOffHelper(protocol name, address of the server to send the packet)
        OnOffHelper onOffHelper("ns3::TcpSocketFactory", 
                            InetSocketAddress(receiverInterfaces.GetAddress(receiverId), port));
        
        onOffHelper.SetAttribute("PacketSize", UintegerValue(1024));    // set packetsize = 1024 bytes
        double dataRateBps = (1024 * 8) * packetsPerSecond;
        onOffHelper.SetAttribute("DataRate", DataRateValue(DataRate(dataRateBps)));
        double startTime = 1.5 + flow*intervalBetPackets;
        onOffHelper.SetAttribute("StartTime", TimeValue(Seconds(startTime)));
        senderApps.Add(onOffHelper.Install(senderNodes.Get(senderId)));
        
        // update the new sender and receiver
        senderId++; receiverId++;
        senderId %= senderNodes.GetN(); 
        receiverId %= receiverNodes.GetN();
    }

    senderApps.Stop(Seconds(totalSimulationtime));
    receiverApps.Stop(Seconds(totalSimulationtime));


    // populate the routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(totalSimulationtime));
    
/************************Tracing*******************/

    // trace the packets send from the OnOffApplication
    Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx",
                                  MakeCallback(&tracePacketSend));

    // trace the packets received in the Packetsink Application
    Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                                  MakeCallback(&tracePacketReceive));


    Simulator::Run();
    Simulator::Destroy();


    // std::cout<<"Total Rx Bytes: "<<totalRxBytes<< " total Rx packets: "<< totalRxPackets<<"\n";
    // std::cout<<"Total Tx Packets: "<<totalTxPackets<<" total Tx bytes: "<<totalTxBytes << "\n";
    
    if(argName == "nodes"){
        writeThroughput<<totalNodes<<"\t"<<calculateThroughput()<<"\n";
        writePacketDeliveryRatio<<totalNodes<<"\t"<<calculatePacketDeliveryRatio()<<"\n";
    }
    else if(argName == "flows"){
        writeThroughput<<totalFlows<<"\t"<<calculateThroughput()<<"\n";
        writePacketDeliveryRatio<<totalFlows<<"\t"<<calculatePacketDeliveryRatio()<<"\n";
    }
    else if(argName == "packets"){
        writeThroughput<<packetsPerSecond<<"\t"<<calculateThroughput()<<"\n";
        writePacketDeliveryRatio<<packetsPerSecond<<"\t"<<calculatePacketDeliveryRatio()<<"\n";
    }
    else{
        writeThroughput<<speedOfNodes<<"\t"<<calculateThroughput()<<"\n";
        writePacketDeliveryRatio<<speedOfNodes<<"\t"<<calculatePacketDeliveryRatio()<<"\n";
    }
    
    writeThroughput.close();
    writePacketDeliveryRatio.close();
    
    return 0;
 

}