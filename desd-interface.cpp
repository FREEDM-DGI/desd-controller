/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 *  desd-interface.cpp
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

#include "desd-interface.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <termios.h>

/**
 * Constructs a DesdInterface. Reads and discards the prompt it sends us.
 *
 * @param io_service the io_service to use for the serial connection
 * @param serial_port the name of the terminal to open (e.g. /dev/ttyS0)
 */
DesdInterface::DesdInterface(boost::asio::io_service& io_service,
                             std::string serial_port)
    : IOInterface(m_serial_port),
      m_serial_port(io_service, serial_port)
{
    ConfigureSerialPort();
    FlushSerialPort();

    std::cout << "Discarding DESD's intro prompt" << std::endl;
    // The end of the prompt is the string "DESD"
    (void) ReadUntil('D');
    (void) ReadUntil('D');

    Start();
}

/**
 * Stops current injection and closes the serial port.
 */
DesdInterface::~DesdInterface()
{
    Stop();
    m_serial_port.close();
}

/**
 * Reads and discards the welcome header sent to us by the DESD, starts
 * current injection (don't know what this means), and blocks until all this
 * has been done.
 */
void DesdInterface::Start()
{
    std::cout << "Sending start command to DESD" << std::endl;
    Write("000001s");

    std::cout << "Discarding DESD's response to start command" << std::endl;
    (void) ReadUntil('1');
}

/**
 * Stops the DESD's current injection. (Don't know what this means.) It is
 * harmless to call this function if the DESD is already stopped.
 */
void DesdInterface::Stop()
{
    std::cout << "Sending stop command to DESD" << std::endl;
    Write("000000s");
}

/**
 * Gets the power level of the DESD
 *
 * @ErrorHandling throws std::runtime_error if the state does not exist
 *
 * @return Watts
 */
float DesdInterface::GetPowerLevel()
{
    std::cout << "Sending a power state request" << std::endl;
    Write("000000m");

    std::cout << "Discarding DESD's state preamble" << std::endl;
    (void) ReadUntil(':');

    std::cout << "Reading DESD state response" << std::endl;
    std::string response = ReadUntil('W');
    // cut off the W
    response.resize(response.length() - 1);
    // trim leading spaces
    response.erase(std::remove(response.begin(), response.end(), ' '),
                   response.end());

    std::cout << "Converting " << response << " to float..." << std::endl;
    float result = boost::lexical_cast<float>(response);
    std::cout << "Result: " << result << std::endl;

    return result;
}

/**
 * Commands the DESD to assume a new power level, whatever this means
 *
 * @param power_level the desired power level
 */
void DesdInterface::SetPowerLevel(float power_level)
{
    const std::streamsize COMMAND_FIELD_WIDTH = 6;

    std::ostringstream ss;
    // If value is negative, first character is the minus sign
    if (power_level < 0)
    {
        ss << '-';
        ss.width(COMMAND_FIELD_WIDTH - 1);
    }
    else
    {
        ss.width(COMMAND_FIELD_WIDTH);
    }
    ss.fill('0');
    ss << static_cast<int>(std::abs(::round(power_level))) << 'i';

    std::cout << "Sending power command: " << power_level << std::endl;
    Write(ss.str());

    std::cout << "Discarding DESD's response to power command" << std::endl;
    (void) ReadUntil('A');
}

/**
 * Configures the serial port with the settings expected by the DESD.
 */
void DesdInterface::ConfigureSerialPort()
{
    m_serial_port.set_option(
        boost::asio::serial_port::baud_rate(9600));
    m_serial_port.set_option(
        boost::asio::serial_port::flow_control(
            boost::asio::serial_port::flow_control::none));
    m_serial_port.set_option(
        boost::asio::serial_port::parity(
            boost::asio::serial_port::parity::none));
    m_serial_port.set_option(
        boost::asio::serial_port::stop_bits(
            boost::asio::serial_port::stop_bits::one));
    m_serial_port.set_option(boost::asio::serial_port::character_size(8));
}

/**
 * Flushes all data currently in the serial port buffer, out of an abundence of
 * caution. All incomplete I/O will be discarded.
 */
void DesdInterface::FlushSerialPort()
{
    ::tcflush(m_serial_port.native_handle(), TCIOFLUSH);
}

/**
 * Writes a command to the DESD
 *
 * @param command the command to write
 */
void DesdInterface::Write(std::string command)
{
    std::cout << "Writing to DESD: " << command << std::endl;
    IOInterface::Write(command);
    std::cout << "Write complete" << std::endl;
}

/**
 * Reads a response from the DESD
 *
 * @param until character to read until (returned with result)
 *
 * @return the DESD's response
 */
std::string DesdInterface::ReadUntil(char until)
{
    std::cout << "Reading from DESD until: " << until << std::endl;
    std::string result = IOInterface::ReadUntil(until);
    std::cout << "Read: " << result << std::endl;

    if (result.find("unrecognized command") != std::string::npos)
    {
        throw std::runtime_error("Confused the DESD: " + result);
    }

    return result;
}
