//
// Created by Furkan Guvenc on 09/08/2022.
//

#ifndef NS3_SIGNAL_POWER_H
#define NS3_SIGNAL_POWER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include <ns3/packet.h>

namespace ns3 {
    struct SignalPower {
        unsigned int id = 0;

        double signalDbm = 0;
        double signalDbmAvg = 0;
        double noiseDbm = 0;
        double noiseDbmAvg = 0;
        unsigned int samples = 0;

        void MonitorSniffRx (Ptr<const Packet> packet,
                             uint16_t channelFreqMhz,
                             WifiTxVector txVector,
                             MpduInfo aMpdu,
                             SignalNoiseDbm signalNoise,
                             uint16_t staId);


        void PrintSignal() const ;
    };
}

#endif //NS3_SIGNAL_POWER_H
