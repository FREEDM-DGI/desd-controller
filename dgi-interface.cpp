/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 *  dgi-interface.cpp
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

#include "dgi-interface.hpp"

#include <cmath>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace {

/* We pretend to be an SST for compatibility with DGI v1.6 */
const std::string DEVICE_NAME = "DESD1";
const std::string device_type = "Sst";
const std::string device_signal = "gateway";

const float null_command = std::pow(10, 8);

}

/**
 * A non-fatal error: the DgiInterface should attempt to reconnect to the DGI.
 */
class MustReconnect : public std::runtime_error
{
public:
    MustReconnect(std::string what)
        : std::runtime_error(what)
    {
    }
};

/**
 * Constructs a DgiInterface.
 */
DgiInterface::DgiInterface(std::string hostname, std::string port,
                           std::string terminal)
    : IOInterface(m_socket),
      m_io_service(),
      m_hostname(hostname),
      m_port(port),
      m_socket(m_io_service),
      m_desd_interface(m_io_service, terminal)
{
}

/**
 * Closes the connection to the DGI
 */
DgiInterface::~DgiInterface()
{
    Disconnect();
}

/**
 * Runs the plug and play session protocol
 */
void DgiInterface::Run()
{
    for (;;)
    {
        m_io_service.post(boost::bind(&DgiInterface::Connect, this));

        try
        {
            m_io_service.run();
        }
        catch (MustReconnect& e)
        {
            std::cout << "Reconnecting after DGI error:\n" << e.what();
            std::cout.flush();
            Disconnect();
        }
    }
}

/**
 * Establishes connection to the DGI
 */
void DgiInterface::Connect()
{
    std::cout << "Connecting to the DGI..." << std::endl;
    boost::asio::ip::tcp::resolver resolver(m_io_service);
    boost::asio::ip::tcp::resolver::query query(m_hostname, m_port);
    boost::asio::connect(m_socket, resolver.resolve(query));
    std::cout << "Connection successful" << std::endl;

    m_io_service.post(boost::bind(&DgiInterface::SendHello, this));
}

/**
 * Disconnects from the DGI.
 *
 * Note that we do not implement polite disconnects because the DGI has no
 * special polite disconnect implementation for us -- there would be no point.
 */
void DgiInterface::Disconnect()
{
    m_socket.close();
    std::cout << "Disconnected from the DGI" << std::endl;
}

/**
 * Sends a Hello message to the DGI
 */
void DgiInterface::SendHello()
{
    std::cout << "Sending Hello message to DGI..." << std::endl;
    Write("Hello\r\n"
          "DESD Controller\r\n" +
          device_type + " " + DEVICE_NAME + "\r\n");
    std::cout << "Successfully sent Hello" << std::endl;

    m_io_service.post(boost::bind(&DgiInterface::ReceiveStart, this));
}

/**
 * Receives a Start message from the DGI
 */
void DgiInterface::ReceiveStart()
{
    std::cout << "Awaiting start message from DGI..." << std::endl;
    if (ReadMessage() != "Start\r\n")
        throw std::runtime_error("Received malformed start mesasge");
    std::cout << "Received start message, starting..." << std::endl;

    m_io_service.post(boost::bind(&DgiInterface::SendState, this));
}

/**
 * Sends the DESD's power level to the DGI.
 */
void DgiInterface::SendState()
{
    std::cout << "Requesting power level from DESD..." << std::endl;
    std::string power_level =
        boost::lexical_cast<std::string>(m_desd_interface.GetPowerLevel());
    std::cout << "Got power level from DESD, sending to DGI..." << std::endl;
    Write("DeviceStates\r\n" +
          DEVICE_NAME + " " + device_signal + " " + power_level + "\r\n"
          "\r\n");
    std::cout << "Successfully sent power level to DGI" << std::endl;

    // FIXME timeout must not be hardcoded
    boost::asio::deadline_timer timer(m_io_service,
                                      boost::posix_time::seconds(2));
    timer.async_wait(boost::bind(&DgiInterface::RelayCommand, this));
}

/**
 * Receives a power level command from the DGI and sends it to the DESD.
 */
void DgiInterface::RelayCommand()
{
    std::cout << "Receiving command from DGI..." << std::endl;
    std::istringstream iss(ReadMessage());

    std::string word;
    iss >> word;
    if (!iss || word != "DeviceCommands")
        throw std::runtime_error("Received unexpected message type");
    iss >> std::ws >> word;
    if (!iss || word != DEVICE_NAME)
        throw std::runtime_error("Unexpected device in DeviceCommands message");
    iss >> word;
    if (!iss || word != device_signal)
        throw std::runtime_error("Unexpected signal in DeviceCommands message");
    float power_level;
    iss >> power_level;
    if (!iss)
        throw std::runtime_error("Bad power level in DeviceCommands message");
    iss >> word;
    if (!iss || word != "\r\n\r\n")
        throw std::runtime_error("Extra content in DeviceCommands message");

    if (power_level != null_command)
    {
        std::cout << "Received DGI command, forwarding to DESD..." << std::endl;
        m_desd_interface.SetPowerLevel(power_level);
        std::cout << "Successfully forwarded command to DESD" << std::endl;
    }
    else
    {
        std::cout << "Dropping null command from DGI" << std::endl;
    }

    // FIXME timeout must not be hardcoded
    boost::asio::deadline_timer timer(m_io_service,
                                      boost::posix_time::seconds(2));
    timer.async_wait(boost::bind(&DgiInterface::SendState, this));
}

/**
 * Receives a message from the DGI. (A message is terminated by \r\n\r\n.)
 *
 * @return the received message
 */
std::string DgiInterface::ReadMessage()
{
    std::string message, line;

    do
    {   // In this order, to trim the trailing CRLF
        message += line;
        line = ReadLine();
    } while (line != "\r\n");

    std::cout << "Received message from DGI:\n" << message;
    std::cout.flush();

    if (message.find("BadRequest") == 0)
    {
        message.erase(0, std::strlen("BadRequest\r\n"));
        throw std::runtime_error("Confused the DGI:\n" + message);
    }
    else if (message.find("Error") == 0)
    {
        message.erase(0, std::strlen("Error\r\n"));
        if (message.find("Duplicate session") != std::string::npos ||
            message.find("Connection closed") != std::string::npos)
        {
            throw MustReconnect(message);
        }
        else
        {
            throw std::runtime_error("Unknown DGI error:\n" + message);
        }
    }

    return message;
}

/**
 * Sends a message to the DGI
 *
 * @param message the message to be sent, without CRLF delimiter
 */
void DgiInterface::Write(std::string message)
{
    std::cout << "Sending message to DGI: " << message;
    std::cout.flush();
    IOInterface::Write(message + "\r\n");
    std::cout << "Send complete" << std::endl;
}
