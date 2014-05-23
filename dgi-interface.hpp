/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 *  dgi-interface.hpp
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

#ifndef DGI_INTERFACE_HPP
#define DGI_INTERFACE_HPP

#include "desd-interface.hpp"
#include "io-interface.hpp"

#include <string>

#include <boost/asio.hpp>

/**
 * A class that knows how to talk to the DGI. It uses the DESD interface
 * to exchange states and commands with the DESD.
 */
class DgiInterface : public IOInterface<boost::asio::ip::tcp::socket>
{
public:
    /// Constructor
    DgiInterface(std::string hostname, std::string port, std::string terminal);
    /// Destructor
    ~DgiInterface();
    /// Runs the plug and play session protocol
    void Run();

private:
    /// Establishes connection to the DGI
    void Connect();
    /// Disconnects from the DGI
    void Disconnect();
    /// Sends a Hello message to the DGI
    void SendHello();
    /// Receives a Start message from the DGI, in response to a Hello
    void ReceiveStart();
    /// Sends the DESD's power level to the DGI
    void SendState();
    /// Sends the DGI's power level command to the DESD
    void RelayCommand();
    /// Process one message from the DGI
    std::string ReadMessage();
    /// Writes to the DESD
    void Write(std::string message);

    /// Runs I/O operations for both the DGI interface and its DESD interface
    boost::asio::io_service m_io_service;
    /// Hostname to connect to
    std::string m_hostname;
    /// Remote port to connect to
    std::string m_port;
    /// Connected to the DGI
    boost::asio::ip::tcp::socket m_socket;
    /// Serial interface to the attached DESD
    DesdInterface m_desd_interface;
};

#endif
