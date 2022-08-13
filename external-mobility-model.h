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
#ifndef EXTERNAL_MOBILITY_MODEL_H
#define EXTERNAL_MOBILITY_MODEL_H

#include "ns3/mobility-model.h"
#include "ns3/network-module.h"
#include "ns3/geographic-positions.h"

#include <vector>
#include "c_library_v2/standard/mavlink.h"


#define BUFFER_LENGTH 2041

namespace ns3 {

/**
 * \ingroup mobility
 *
 * \brief Mobility model for which the current speed does not change once it has been set and until it is set again explicitly to a new value.
 */
class ExternalMobilityModel : public MobilityModel
{
public:
  /**
   * Register this type with the TypeId system.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * Create position located at coordinates (0,0,0) with
   * speed (0,0,0).
   */
  ExternalMobilityModel ();
  virtual ~ExternalMobilityModel ();

  void StartListen (Ptr<Socket>, uint32_t port);

private:
  void ReceivePacketUdp (Ptr<Socket>);

  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const;



  unsigned int m_id;
  Ptr<Socket> socket;

  Vector m_position;

  mavlink_status_t mavlinkStatus;
  mavlink_message_t mavlinkMessage;
  mavlink_global_position_int_t mavlinkGlobalPosition;

    double baseLat;        //!< the uav start latitude
    double baseLng;        //!< the uav start longitude
    double baseAlt;        //!< the uav start altitude
    Vector3D base_coordinates; //!< the uav start cartesian coordinates

};

} // namespace ns3

#endif /* EXTERNAL_MOBILITY_MODEL_H */
