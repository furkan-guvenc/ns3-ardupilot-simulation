//
// Created by Furkan Guvenc on 09/08/2022.
//

#include "signal_power.h"
#include <iostream>

namespace ns3 {

    void SignalPower::MonitorSniffRx (Ptr<const Packet> packet,
                                      uint16_t channelFreqMhz,
                                      WifiTxVector txVector,
                                      MpduInfo aMpdu,
                                      SignalNoiseDbm signalNoise,
                                      uint16_t staId)

    {
        samples++;
        signalDbm = signalNoise.signal;
        noiseDbm = signalNoise.noise;
        signalDbmAvg += ((signalNoise.signal - signalDbmAvg) / samples);
        noiseDbmAvg += ((signalNoise.noise - noiseDbmAvg) / samples);
    }

    void SignalPower::PrintSignal() const {
        std::cout << "Node "<< id <<
        " Signal: " << signalDbm << " (dBm) Avg Signal: " << signalDbmAvg <<
        " (dBm) Noise: " << noiseDbm << " (dBm) Avg Noise: " << noiseDbmAvg << " (dBm)" << std::endl;

        Simulator::Schedule(Seconds(1), &SignalPower::PrintSignal, this);

    }

}
