/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2007 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "external-mobility-model.h"

#include <iostream>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


namespace ns3 {

// 41.15242,-8.64096,72
Vector3D base_coordinates = GeographicPositions::GeographicToCartesianCoordinates(
        41.15242,
        -8.64096,
        72,
        GeographicPositions::EarthSpheroidType::WGS84
);


void ReceivePacketUdp (Ptr<Socket> socket);

NS_OBJECT_ENSURE_REGISTERED (ExternalMobilityModel);

TypeId ExternalMobilityModel::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ExternalMobilityModel")
            .SetParent<MobilityModel> ()
            .SetGroupName ("Mobility")
            .AddConstructor<ExternalMobilityModel> ();
    return tid;
}


ExternalMobilityModel::ExternalMobilityModel ()
{
    static unsigned id = 0;
    m_id = id++;
}

ExternalMobilityModel::~ExternalMobilityModel ()
{
}

void
ExternalMobilityModel::StartListen (Ptr<Socket> socket, uint32_t port)
{
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),port);
    socket->Bind (local);
    socket->SetRecvCallback (MakeCallback (&ExternalMobilityModel::ReceivePacketUdp, this));
}

Vector
ExternalMobilityModel::DoGetPosition (void) const
{
    return m_position;
}

void
ExternalMobilityModel::DoSetPosition (const Vector &position)
{
    m_position = position;
    NotifyCourseChange ();
}

Vector
ExternalMobilityModel::DoGetVelocity (void) const
{
    return Vector(0.0, 0.0, 0.0);
}

void ExternalMobilityModel::ReceivePacketUdp (Ptr<Socket> socket)
{
    Ptr<Packet> packet = socket->Recv (BUFFER_LENGTH,0);
    const size_t packet_len = packet->GetSize ();
    //  packet->Print (std::cout);
    //  std::cout << std::endl;
    auto *buffer = new uint8_t[packet_len];
    packet->CopyData(buffer, packet_len);


    for (size_t i = 0; i < packet_len; ++i) {
        uint8_t byte = buffer[i];
        if (mavlink_parse_char(MAVLINK_COMM_0, byte, &this->mavlinkMessage, &this->mavlinkStatus))
        {
//            std::cout<<"Received message with ID "<<msg.msgid<<", sequence: "<<msg.seq<<" from component "<<msg.compid<<" of system "<<msg.sysid<<std::endl;

            switch(this->mavlinkMessage.msgid) {
                case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: // ID for GLOBAL_POSITION_INT 33
                {
                    // Get all fields in payload (into global_position)
                    mavlink_msg_global_position_int_decode(&this->mavlinkMessage, &this->mavlinkGlobalPosition);
                    // Print all fields
                    std::cout<< "Node "<< this->m_id <<" Position: "<<this->mavlinkGlobalPosition.lat<<", "<<this->mavlinkGlobalPosition.lon<<", "<<this->mavlinkGlobalPosition.alt<<std::endl;
                    Vector3D coordinates = GeographicPositions::GeographicToCartesianCoordinates(
                            (double) this->mavlinkGlobalPosition.lat / 10000000,
                            (double) this->mavlinkGlobalPosition.lon / 10000000,
                            (double) this->mavlinkGlobalPosition.alt / 1000,
                            GeographicPositions::EarthSpheroidType::WGS84
                    );
                    std::cout<<"Diff: "<< coordinates - base_coordinates <<std::endl;
                    std::cout<<"Distance: "<< CalculateDistance(coordinates, base_coordinates) <<std::endl;

                }
                    break;
                default:
                    break;
            }
        }
    }
    delete[] buffer;

}

} // namespace ns3
