/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 *  main.cpp
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

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    // FIXME catch SIGINT, SIGTERM and stop DESD before quitting
    // (may or may not be important :-)

    po::options_description od;
    po::variables_map vm;
    std::string hostname, port, serial_port;

    od.add_options()
        ("dgi-hostname,h",
         po::value<std::string>(&hostname)->default_value("localhost"),
         "hostname of DGI")
        ("dgi-port,p",
         po::value<std::string>(&port)->default_value("53000"),
         "DGI TCP port to connect to")
        ("serial-port,t",
         po::value<std::string>(&serial_port)->default_value("/dev/ttyS0"),
         "serial terminal connected to DESD")
        ("help,h", "print help")
        ("version,V", "print version");

    po::store(po::command_line_parser(argc, argv).options(od).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << od << std::endl;
        return 0;
    }
    else if (vm.count("version"))
    {
        std::cout << "1.0" << std::endl;
        return 0;
    }

    DgiInterface dgi_interface(hostname, port, serial_port);
    dgi_interface.Run();
}
