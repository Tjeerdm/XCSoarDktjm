/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "DebugPort.hpp"
#include "Device/Port/Port.hpp"
#include "Device/Port/ConfiguredPort.hpp"
#include "Device/Driver.hpp"
#include "Device/Register.hpp"
#include "Device/Parser.hpp"
#include "Device/Driver/LX/LX1600.hpp"
#include "OS/PathName.hpp"
#include "OS/Args.hpp"
#include "Profile/DeviceConfig.hpp"
#include "Util/StringUtil.hpp"
#include "Operation/ConsoleOperationEnvironment.hpp"
#include "IO/Async/GlobalIOThread.hpp"

#include <stdio.h>

static void
SetMC(Port &port, OperationEnvironment &env)
{
  while (true) {
    fixed mc;

    fprintf(stdout, "Please enter the MC setting (0.0 - 5.0):\n");
    fprintf(stdout, "> ");

    char buffer[64];
    if (fgets(buffer, 64, stdin) == NULL || strlen(buffer) == 0) {
      fprintf(stdout, "Invalid input\n");
      continue;
    }

    TrimRight(buffer);

    char *end_ptr;
    mc = fixed(strtod(buffer, &end_ptr));
    if (end_ptr == buffer) {
      fprintf(stdout, "Invalid input\n");
      continue;
    }

    fprintf(stdout, "Setting MC to \"%.1f\" ...\n", (double)mc);

    if (LX1600::SetMacCready(port, env, mc))
      fprintf(stdout, "MC set to \"%.1f\"\n", (double)mc);
    else
      fprintf(stdout, "Operation failed!\n");

    return;
  }
}

static void
WriteMenu()
{
  fprintf(stdout, "------------------------------------\n"
                  "LX1600 Utils Menu\n"
                  "------------------------------------\n"
                  "Press any of the following commands:\n\n"
                  "h:  Display this menu\n"
                  "1:  Set the MC\n"
                  "q:  Quit this application\n"
                  "------------------------------------\n");
}

static void
RunUI(Port &port, OperationEnvironment &env)
{
  WriteMenu();

  while (true) {
    fprintf(stdout, "> ");

    char in[20];
    if (fgets(in, 20, stdin) == NULL || strlen(in) == 0) {
      fprintf(stdout, "Invalid input\n");
      continue;
    }

    switch (in[0]) {
    case '?':
    case 'h':
    case 'H':
      WriteMenu();
      break;
    case '1':
      SetMC(port, env);
      break;
    case 'q':
    case 'Q':
      fprintf(stdout, "Closing LX1600 Utils ...\n");
      return;
    default:
      fprintf(stdout, "Invalid input\n");
      break;
    }
  }
}

int
main(int argc, char **argv)
{
  Args args(argc, argv, "PORT BAUD");
  const DeviceConfig config = ParsePortArgs(args);
  args.ExpectEnd();

  InitialiseIOThread();

  std::unique_ptr<Port> port(OpenPort(config, *(DataHandler *)NULL));
  if (!port) {
    fprintf(stderr, "Failed to open COM port\n");
    return EXIT_FAILURE;
  }

  ConsoleOperationEnvironment env;
  RunUI(*port, env);

  DeinitialiseIOThread();

  return EXIT_SUCCESS;
}