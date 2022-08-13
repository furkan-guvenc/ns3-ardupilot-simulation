//
// Created by Furkan Guvenc on 09/08/2022.
//

#include "signal_power.h"
#include <iostream>

namespace ns3 {

    SignalPower::SignalPower(unsigned int id) {
        this->id = id;
        signalDbm = 0;
        signalDbmAvg = 0;
        noiseDbm = 0;
        noiseDbmAvg = 0;
        samples = 0;
    }

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

    void SignalPower::StreamSignal(Ptr<OutputStreamWrapper> stream) {
        std::ostream* os = stream->GetStream ();
        *os << Simulator::Now().GetSeconds() << " Node "<< id+1 <<
        " Signal: " << signalDbm << " (dBm) Avg Signal: " << signalDbmAvg <<
        " (dBm) Noise: " << noiseDbm << " (dBm) Avg Noise: " << noiseDbmAvg << " (dBm)" << std::endl;

        Simulator::Schedule(Seconds(1), &SignalPower::StreamSignal, this, stream);

    }

    void SignalPower::StreamSignalAsCsv(Ptr<OutputStreamWrapper> stream) {
        std::ostream* os = stream->GetStream ();

        *os << "Time,Node,Signal (dBm),Avg Signal (dBm),Noise (dBm),Avg Noise (dBm)" << std::endl;

        Simulator::Schedule(Seconds(1), &SignalPower::StreamSignalAsCsvRow, this, stream);
    }

    void SignalPower::StreamSignalAsCsvRow(Ptr<OutputStreamWrapper> stream) {
        std::ostream* os = stream->GetStream ();
        *os << Simulator::Now().GetSeconds() << ","<< id+1 << "," << signalDbm << "," << signalDbmAvg
        << "," << noiseDbm << "," << noiseDbmAvg << std::endl;

        Simulator::Schedule(Seconds(1), &SignalPower::StreamSignalAsCsvRow, this, stream);
    }


}
