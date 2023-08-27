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
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 * and Greeshma Umapathi
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

#ifndef TCP_ADAPTIVERENO_H
#define TCP_ADAPTIVERENO_H

#include "tcp-congestion-ops.h"

#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/traced-value.h"
#include "tcp-westwood-plus.h"

namespace ns3
{

class Time;

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of TCP Adaptive-Reno.
 *
 * TCP-AReno is based on TCP-Westwood-BBE(Buffer and Bandwidth Estimation)
 * It estimates congestion level via RTT to determine whether a packet loss 
 * is due to congestion or not
 * 
 * TCP-AReno introduces a fast window expansion mechanism to quickly 
 * increase congestion window whenever it finds low network underutilization.
 * 
 * The TCP-AReno calculates a value 'c' as the congestion level and then 
 * according to that level it decrease or increase the congestion window
 * 
 * There are two parts of congestion window management 1. W_base 2. W_probe
 * W_base is similar to TCP-Reno and W_probe is new to TCP-AReno (1MSS/RTT)
 * For a underutilized network TCP-AReno exponentially increases the W_probe
 * value so that more packets are supplied
 * 
 * When there is a possibility for congestion the window size is halved in order
 * to mitigate the packet losses.
 * 
 * 
 * The two main methods in the implementation are the CountAck (const TCPHeader&)
 * and the EstimateBW (int, const, Time). The CountAck method calculates
 * the number of acknowledged segments on the receipt of an ACK.
 * The EstimateBW estimates the bandwidth based on the value returned by CountAck
 * and the sampling interval (last RTT).
 *
 * WARNING: this TCP model lacks validation and regression tests; use with caution.
 */
class TcpAdaptiveReno : public TcpWestwoodPlus
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    TcpAdaptiveReno();
    /**
     * \brief Copy constructor
     * \param sock the object to copy
     */
    TcpAdaptiveReno(const TcpAdaptiveReno& sock);
    virtual ~TcpAdaptiveReno(void);

    /**
     * \brief Filter type (None or Tustin)
     */
    enum FilterType
    {
        NONE,
        TUSTIN
    };

    virtual uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb, 
                        uint32_t bytesInFlight);

    virtual void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt) ;

    virtual Ptr<TcpCongestionOps> Fork();
    
  private:
    // /**
    //  * Update the total number of acknowledged packets during the current RTT
    //  *
    //  * \param [in] acked the number of packets the currently received ACK acknowledges
    //  */
    // void UpdateAckedSegments(int acked);
    
    /**
     * \brief c = min((RTT − RTT_min/RTT_cong − RTT_min), 1)
    */
    double EstimateCongestionLevel();

    void EstimateIncWnd(Ptr<TcpSocketState>tcb);


  protected:
    /**
    * \brief This function increases congestion window in congestion avoidance phase.
    */
    void CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

    Time curRTT;               // RTT_j
    Time oldRTT;               // RTT_j-1
    Time minRTT;               // RTT_min
    Time congestionRTT;        // RTT_cong
    Time oldCongRTT;           // RTT_cong^j-1 
    Time jPacketLossRTT;       // RTT_j

    uint32_t m_ackedSegments;  //!< The number of segments ACKed between RTTs
    uint32_t incWindow;         // W_inc
    uint32_t baseWindow;        // W_base
    uint32_t probeWindow;       // W_probe

};

} // namespace ns3

#endif /* TCP_ADAPTIVERENO_H */
