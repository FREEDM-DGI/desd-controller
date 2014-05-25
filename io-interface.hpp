/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 *  io-interface.hpp
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

#ifndef IO_INTERFACE_HPP
#define IO_INTERFACE_HPP

#include <string>

#include <boost/array.hpp>
#include <boost/asio.hpp>

/**
 * Contains a reference to a stream from which to perform file I/O, plus a
 * buffer to read into.
 */
template <typename SyncReadStream>
class IOInterface
{
public:
    /**
     * Destructor
     */
    virtual ~IOInterface()
    {
    }

    /**
     * Reads from the peer
     *
     * @param until the character to read until (returned in result)
     *
     * @return data from the peer
     */
    virtual std::string ReadUntil(char until)
    {
        boost::asio::read_until(m_stream, m_streambuf, until);

        std::istream is(&m_streambuf);
        std::string result;
        std::getline(is, result, until);
        result += until; // getline discards delimiter

        return result;
    }

    /**
     * Reads a line from the peer
     *
     * @param until the string to read until (included in result)
     *
     * @return line from the peer, line ending included
     */
    virtual std::string ReadLine()
    {
        boost::asio::read_until(m_stream, m_streambuf, '\n');

        std::istream is(&m_streambuf);
        std::string result;
        std::getline(is, result);
        result += '\n'; // getline discards delimiter

        return result;
    }

    /**
     * Writes to the peer
     *
     * @param msg string to write
     */
    virtual void Write(std::string message)
    {
        boost::asio::write(m_stream, boost::asio::buffer(message));
    }

protected:
    /**
     * Constructor
     *
     * @param stream to be used for I/O
     */
    IOInterface(SyncReadStream& stream) : m_stream(stream) {}

private:
    /// An I/O stream such as a normal file, a socket, or a serial port
    SyncReadStream& m_stream;
    /// Resizable buffer which may contain excess data for the next read call.
    boost::asio::streambuf m_streambuf;
};

#endif
