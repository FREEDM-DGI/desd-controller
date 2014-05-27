/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 *  desd-interface.hpp
 *
 *  Author: Michael Catanzaro <michael.catanzaro@mst.edu>
 *
 *  These source code files were created at Missouri University of Science and
 *  Technology, and are intended for use in teaching or research. They may be
 *  freely copied, modified, and redistributed as long as modified versions are
 *  clearly marked as such and this notice is not removed. Neither the authors
 *  nor Missouri S&T make any warranty, express or implied, nor assume any legal
 *  responsibility for the accuracy, completeness, or usefulness of these files
 *  or any information distributed with these files.
 *
 *  Suggested modifications or questions about these files can be directed to
 *  Dr. Bruce McMillin, Department of Computer Science, Missouri University of
 *  Science and Technology, Rolla, MO 65409 <ff@mst.edu>.
 */

#ifndef DESD_INTERFACE_HPP
#define DESD_INTERFACE_HPP

#include "io-interface.hpp"

#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>

/**
 * A class that knows how to talk to the DESD.
 */
class DesdInterface : public IOInterface<boost::asio::serial_port>
{
public:
    /// Constructor
    DesdInterface(boost::asio::io_service& io_service, std::string serial_port);
    /// Destructor
    ~DesdInterface();
    /// Start the DESD's current injection
    void Start();
    /// Stop the DESD's current injection
    void Stop();
    /// Get the power level of the DESD
    float GetPowerLevel();
    /// Change the power level of the DESD
    void SetPowerLevel(float power_level);

private:
    /// Configures the serial port with the correct settings
    void ConfigureSerialPort();
    /// Flush all data currently in the serial port buffer
    void FlushSerialPort();
    /// Reads from the DESD up through until
    std::string ReadUntil(char until);
    /// Writes to the DESD
    void Write(std::string command);

    /// Serial terminal connected to the DESD
    boost::asio::serial_port m_serial_port;
};

#endif
