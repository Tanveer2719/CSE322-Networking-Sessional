#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <filesystem>

/*
            n0                    n4
              \   "10.3.1.0"      /
  "10.1.1.0"   n2---------------n3  "10.2.1.0"
              /                  \
            n1                    n5  


*/

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("NETWORKSIMULATION");

// file variables
std::ofstream writeThroughput, writeCongestion;
std::string rateDir, exDir, conDir;

// task No
int task = 3;

void 
createFile(std::string argName){
   
    rateDir = "scratch/files/Offline3/" + argName + "/";
    exDir = "scratch/files/Offline3/" + argName + "/";
    conDir = "scratch/files/Offline3/congestion/";
  if(task == 1){
    rateDir += "taskA/";
    exDir +=  "taskA/";
    conDir += "taskA/";
  }else if(task == 2){
    rateDir += "taskB1/";
    exDir +=  "taskB1/";
    conDir += "taskB1/";
  }else{
    rateDir+= "taskB2/";
    exDir +=  "taskB2/";
    conDir += "taskB2/";
  }

    std::filesystem::create_directories(rateDir);
    std::filesystem::create_directories(exDir);
    std::filesystem::create_directories(conDir);
}

class TutorialApp : public Application
{
  public:
    TutorialApp();
    ~TutorialApp() override;

    /**
     * Register this type.
     * \return The TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Setup the socket.
     * \param socket The socket.
     * \param address The destination address.
     * \param packetSize The packet size to transmit.
     * \param simulTime The simulation time in seconds.
     * \param dataRate the data rate to use.
     */
    void Setup(Ptr<Socket> socket,
               Address address,
               uint32_t packetSize,
               uint32_t simulTime,
               DataRate dataRate);

  private:
    void StartApplication() override;
    void StopApplication() override;

    /// Schedule a new transmission.
    void ScheduleTx();
    /// Send a packet.
    void SendPacket();

    Ptr<Socket> m_socket;   //!< The transmission socket.
    Address m_peer;         //!< The destination address.
    uint32_t m_packetSize;  //!< The packet size.
    uint32_t simulTime;    //!< The simulation time.
    DataRate m_dataRate;    //!< The data rate to use.
    EventId m_sendEvent;    //!< Send event.
    bool m_running;         //!< True if the application is running.
    uint32_t m_packetsSent; //!< The number of packets sent.
};

TutorialApp::TutorialApp()
    : m_socket(nullptr),
      m_peer(),
      m_packetSize(0),
      simulTime(0),
      m_dataRate(0),
      m_sendEvent(),
      m_running(false),
      m_packetsSent(0)
{
}

TutorialApp::~TutorialApp()
{
    m_socket = nullptr;
}

/* static */
TypeId
TutorialApp::GetTypeId()
{
    static TypeId tid = TypeId("TutorialApp")
                            .SetParent<Application>()
                            .SetGroupName("Tutorial")
                            .AddConstructor<TutorialApp>();
    return tid;
}

void
TutorialApp::Setup(Ptr<Socket> socket,
                   Address address,
                   uint32_t packetSize,
                   uint32_t time,
                   DataRate dataRate)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    simulTime = time;
    m_dataRate = dataRate;
}

void
TutorialApp::StartApplication()
{
    m_running = true;
    m_packetsSent = 0;
    m_socket->Bind();
    m_socket->Connect(m_peer);
    SendPacket();
}

void
TutorialApp::StopApplication()
{
    m_running = false;

    if (m_sendEvent.IsRunning())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }
}

void
TutorialApp::SendPacket()
{
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

    if (Simulator::Now().GetSeconds() < simulTime)
    {
        ScheduleTx();
    }
}

void
TutorialApp::ScheduleTx()
{
    if (m_running)
    {
        Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &TutorialApp::SendPacket, this);
    }
}

static void
congestionWindow(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  // std::cout<< Simulator::Now ().GetSeconds () << " " << newCwnd<<std::endl;
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << newCwnd<<std::endl;
}

int 
main(int argc, char* argv[])
{
  
    // LogComponentEnable("PacketSink", LOG_LEVEL_ALL);
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(1024));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1024));

    uint32_t nLeftLeaf = 2;
    uint32_t nRightLeaf = 2;
    uint32_t packetSize = 1024;
    int bottleNeckDelay = 100;
    int bottleNeckRate = 50;
    int exp = 6;


    CommandLine cmd(__FILE__);
    cmd.AddValue("task", "1.westwoodPlus vs newReno\n2.adaptiveReno vs highSpeed\n3.adaptiveReno vs newReno", task);
    cmd.AddValue("datarate", "Data Rate in the BottleNeck nodes", bottleNeckRate);
    cmd.AddValue("exp", "Exponent of the Error rate(give 2 for 10^(-2))", exp);
    cmd.Parse(argc, argv);
    std::string argName;
    if(argc == 3){
        for(int i = 2; i<(int)strlen(argv[2]); i++){
            if(argv[2][i] == '=') break;
            argName+= argv[2][i];
        }
    }

    // create the file
    createFile(argName);

    exp *= -1;
    double errorRate = std::pow(10,exp);
    

    /**********copied from dumbell-animation.cc********/

    // Create the point-to-point link helpers for bottleneck
    PointToPointHelper pointToPointRouter;
    pointToPointRouter.SetDeviceAttribute("DataRate", StringValue(std::to_string(bottleNeckRate)+"Mbps"));
    pointToPointRouter.SetChannelAttribute("Delay", StringValue("100ms"));
    
    // Create the point-to-point link helpers for left and right side nodes
    PointToPointHelper pointToPointLeaf;
    pointToPointLeaf.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    pointToPointLeaf.SetChannelAttribute("Delay", StringValue("1ms"));

    double bandwidth_delay_product = bottleNeckDelay * bottleNeckRate;
    
    // as given in the spec
    // DropTailQueue means the packets arriving after the queue is full will be discarded
    pointToPointLeaf.SetQueue ("ns3::DropTailQueue", "MaxSize",
    StringValue (std::to_string (bandwidth_delay_product) + "p"));


    PointToPointDumbbellHelper d(nLeftLeaf,
                                 pointToPointLeaf,
                                 nRightLeaf,
                                 pointToPointLeaf,
                                 pointToPointRouter);
    // add reateError model
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(errorRate));
    d.m_routerDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    InternetStackHelper stackHelper;

    // set up the congestion control algorithms
    /**
      for taskA:
      flow 1 between left0 and right0(TcpWestwoodPlus)
      flow 2 between left1 and right1(TcpNewReno)

      for taskB.1:
      flow 1 between left0 and right0(TcpAdaptiveReno)
      flow 2 between left1 and right1(TcpHighSpeed)

      for taskB.2:
      flow 1 between left0 and right0(TcpAdaptiveReno)
      flow 2 between left1 and right1(TcpNewReno)

    */
  if(task == 1){
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwoodPlus::GetTypeId ()));
  }else if(task == 2 || task == 3){
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpAdaptiveReno::GetTypeId ()));
  }
    stackHelper.Install(d.GetLeft(0));
    stackHelper.Install(d.GetRight(0));

  if(task == 1 || task == 3){
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
  }else if(task == 2){
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHighSpeed::GetTypeId ()));
  }
    stackHelper.Install(d.GetLeft(1));
    stackHelper.Install(d.GetRight(1));

    stackHelper.Install(d.m_routers);   // to the bottleneck routers


    // Assign IP Addresses
    d.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                          Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                          Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

    

    // addFlowMonitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    uint32_t port = 9; 
    int senderId = 0, receiverId = 0;  
    ApplicationContainer receiverApps;
    // Install Applications
    for(uint32_t flow = 0; flow < 2; flow++){
      port += flow;       // At this Port the server will wait for clients

      Address sinkAddress (InetSocketAddress (d.GetRightIpv4Address (receiverId), port));
      // PacketSinkHelper(protocol name, address of the node that acts as the sink)
      PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
      receiverApps.Add(sinkHelper.Install(d.GetRight(receiverId)));

      Ptr<Socket> socket = Socket::CreateSocket (d.GetLeft (senderId), 
                                            TcpSocketFactory::GetTypeId ());
      Ptr<TutorialApp> app = CreateObject<TutorialApp>();
      app->Setup(socket, sinkAddress, 
                              packetSize, 10, DataRate("1Gbps"));
      d.GetLeft(senderId)->AddApplication(app);

      app->SetStartTime(Seconds(2.0));

      std::ostringstream oss;
      oss << conDir<< flow+1 <<  ".txt";
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (oss.str());
      socket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&congestionWindow,stream));
      senderId++; receiverId++;
    }

    receiverApps.Start(Seconds(1.0));

    Simulator::Stop(Seconds(12.0));

    // // Set up the actual simulation
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Run();
    
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (
                                          flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    double throughput[2]; // 0 for flow=0 ,and 1 for flow=1
    int receivedPackets[2] = {0};


    for (auto iter = stats.begin (); iter != stats.end (); ++iter) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
      // std::cout<<t.sourceAddress<<std::endl;
      
      // std::cout<<"flow Id: "<<iter->first<<std::endl;
      // std::cout<<"src addr: "<<t.sourceAddress<< " dst addr : "<<t.destinationAddress<<std::endl;
      // std::cout<<"Sent Packets: "<<iter->second.txPackets<<std::endl;
      // std::cout<<"Received Packets: "<<iter->second.rxPackets<<std::endl;
      // std::cout<<"Lost Packets = " <<iter->second.lostPackets<<std::endl;
      // std::cout<<"Packet delivery ratio = " <<iter->second.rxPackets*100.0/iter->second.txPackets << "%"<<std::endl;
      
      if((t.sourceAddress == "10.1.1.1" && t.destinationAddress == "10.2.1.1")){
        // std::cout<<"source Address found\n";
        receivedPackets[0] += iter->second.rxPackets;
      }
      else if(t.sourceAddress == "10.1.2.1" && t.destinationAddress == "10.2.2.1"){
        receivedPackets[1] += iter->second.rxPackets;
      }
      
    }

    throughput[0] = receivedPackets[0]*packetSize*8/(12 *1e3); // convert throughput to Kbps 
    throughput[1] = receivedPackets[1]*packetSize*8/(12 *1e3); // 12 because the ended in 12 seconds
    
    std::cout<<"task: "<<task<<" throughput1: "<<throughput[0] << " throughput 2: "<<throughput[1]<<"\n";
    
    if(argName == "datarate"){
        writeThroughput.open(rateDir+"datarate.txt",std::ios_base::app);
        writeThroughput << bottleNeckRate<< "\t"<<throughput[0]<<"\t"<<throughput[1]<<"\n";
        // std::cout << bottleNeckRate<< "\t"<<throughput[0]<<"\t"<<throughput[1]<<"\n";

        writeThroughput.close();

    }
    else{
      writeThroughput.open(exDir+"exp.txt",std::ios_base::app);
      writeThroughput <<std::to_string(exp*(-1)) << "\t"<< throughput[0]<<"\t"<<throughput[1]<<"\n";
      writeThroughput.close();
    }
    Simulator::Destroy();

    return 0;


}