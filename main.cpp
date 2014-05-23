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

int main()
{
    // FIXME this really must not be hardcoded, but this is due today...
    DgiInterface dgi_interface("localhost", "5000", "/dev/ttyS1");
    dgi_interface.Run();
}
