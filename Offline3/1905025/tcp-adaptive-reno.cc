/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>,
 *          Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 *          Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "tcp-adaptive-reno.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE("TcpAdaptiveReno");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(TcpAdaptiveReno);

TypeId
TcpAdaptiveReno::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpAdaptiveReno")
            .SetParent<TcpNewReno>()
            .SetGroupName("Internet")
            .AddConstructor<TcpAdaptiveReno>()
            .AddAttribute(
                "FilterType",
                "Use this to choose no filter or Tustin's approximation filter",
                EnumValue(TcpAdaptiveReno::TUSTIN),
                MakeEnumAccessor(&TcpAdaptiveReno::m_fType),
                MakeEnumChecker(TcpAdaptiveReno::NONE, "None", TcpAdaptiveReno::TUSTIN, "Tustin"))
            .AddTraceSource("EstimatedBW",
                            "The estimated bandwidth",
                            MakeTraceSourceAccessor(&TcpAdaptiveReno::m_currentBW),
                            "ns3::TracedValueCallback::DataRate");
    return tid;
}

TcpAdaptiveReno::TcpAdaptiveReno()
    : TcpWestwoodPlus(),
      curRTT(Time(0)),
      oldRTT(Time(0)),
      minRTT(Time(0)),
      congestionRTT(Time(0)),
      oldCongRTT(Time(0)),
      jPacketLossRTT(Time(0)),
      incWindow(0),
      baseWindow(0),
      probeWindow(0)
{
    NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::TcpAdaptiveReno(const TcpAdaptiveReno& sock)
    : TcpWestwoodPlus(sock),
      curRTT(Time(0)),
      oldRTT(Time(0)),
      minRTT(Time(0)),
      congestionRTT(Time(0)),
      oldCongRTT(Time(0)),
      jPacketLossRTT(Time(0)),
      incWindow(0),
      baseWindow(0),
      probeWindow(0)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_LOGIC("Invoked the copy constructor");
}

TcpAdaptiveReno::~TcpAdaptiveReno()
{
}

/**
 * \brief function is called each time a packet 
 * is Acked. It will increase the count of acked 
 * segments and update the current 
 * estimated bandwidth.
*/
void
TcpAdaptiveReno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt)
{
    NS_LOG_FUNCTION(this << tcb << packetsAcked << rtt);

    if (rtt.IsZero())
    {
        NS_LOG_WARN("RTT measured is zero!");
        return;
    }

    m_ackedSegments += packetsAcked;
    
    // update the current and old RTT
    oldRTT = curRTT;
    curRTT = rtt;
    

    // update minimum round trip time
    if(minRTT.IsZero()){
        minRTT = rtt;
    }else if(minRTT >= rtt){
        minRTT = rtt;
    }

    NS_LOG_LOGIC("MIN RTT : "<<minRTT.GetMilliSeconds()<<"ms");
    NS_LOG_LOGIC("CUR RTT : "<<rtt.GetMilliSeconds()<<"ms");

    // update the current estimated bandwidth.
    TcpWestwoodPlus::EstimateBW(rtt, tcb);
    
}

/**
 * The slow start threshold(SsThresh) is a 
 * congestion control parameter that determines when a TCP connection 
 * transitions from the slow start phase 
 * to the congestion avoidance phase.
*/
uint32_t
TcpAdaptiveReno::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight [[maybe_unused]])
{
    
    /**
     * Firstly, here we need to update the RTT_cong^j−1
     * & RTT^j . Then we need to estimate the new reduced
     * congestion window size according to the congestion level.
    */
    
    oldCongRTT = congestionRTT;
    jPacketLossRTT = curRTT;

    double c = EstimateCongestionLevel();

    /**
     * the minimum value the ssthresh can take 
     * is 2*segmentSize
     * W_base = W / (1+c), W_probe = 0
    */
    
    uint32_t ssThresh = std::max((2*tcb->m_segmentSize), (uint32_t)(tcb->m_cWnd / (1+c)));
    baseWindow = ssThresh;
    probeWindow = 0;
    
    NS_LOG_LOGIC("CurrentBW: " << m_currentBW << " minRtt: " << tcb->m_minRtt
                               << " ssThresh: " << ssThresh);

    return ssThresh;
}

double 
TcpAdaptiveReno::EstimateCongestionLevel()
{
    /**
     * a = exponential smoothing factor.
    */
    
    double a = 0.85;        // given in spec
    double rtt_conj;
    Time congestionRTT;
    if(oldRTT.IsZero()) a = 0;  // 
    
    rtt_conj = a*oldRTT.GetSeconds() + (1-a)*jPacketLossRTT.GetSeconds();
    congestionRTT = Seconds(rtt_conj);

    return (std::min(
    (
        (curRTT.GetSeconds() - minRTT.GetSeconds())
        / 
        (congestionRTT.GetSeconds() - minRTT.GetSeconds())
    ) , 1.0));

}

void 
TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState>tcb){
    
    /**
     * 
     * MSS is maximum segment size that is assumed
     * as the square of the original SegmentSize.
     * W_inc = B/M ∗ MSS
     * α = 10
     * β = 2W_inc(1/α − (1/α + 1)/e^α )
     * γ = 1 − 2W_inc(1/α − (1/α + 1/2)/e^α )
     * W_inc (c) = W_inc/e^(cα) + cβ + γ
     * 
    */
    double c = EstimateCongestionLevel();
    double alpha = 10, M = 1000;     // given in spec
    double m_ss = static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize);
    
    DataRate tmpBw = m_currentBW.Get();
    double B = tmpBw.GetBitRate();
    double maxIncWindow = B / M * m_ss;
    double beta = 2 * maxIncWindow * (1/alpha - (1/alpha + 1)/(std::exp(alpha)));
    double gamma = 1 - 2 * maxIncWindow * (1/alpha - (1/alpha + 1/2)/(std::exp(alpha)));

    incWindow = (int) (maxIncWindow / (std::exp(c*alpha)) + c*beta + gamma);
}


void
TcpAdaptiveReno::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked){
    if (segmentsAcked > 0){

        EstimateIncWnd(tcb);    // updates W_inc
        // set the base window
        // copied from https://www.nsnam.org/docs/release/3.34/models/html/tcp.html
        
        /**
        * W_base = W)base + 1MSS/W
        * W_probe = max(W_probe + W_inc/W, 0)
        * W = W_base + W_probe
        */
        
        double adder = static_cast<double> (tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get ();
        adder = std::max (1.0, adder);
        baseWindow += static_cast<uint32_t> (adder);

        probeWindow = std::max(
            (double)(probeWindow + incWindow / (int) tcb->m_cWnd.Get () 
            ), (double) 0);

        
        // NS_LOG_LOGIC("Before "<<tcb->m_cWnd<< " ; base "<<m_baseWnd<<" ; probe "<<m_probeWnd);

        tcb->m_cWnd = baseWindow + probeWindow;
        

        //NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
 }
}

Ptr<TcpCongestionOps>
TcpAdaptiveReno::Fork()
{
    return CreateObject<TcpAdaptiveReno>(*this);
}

} // namespace ns3
