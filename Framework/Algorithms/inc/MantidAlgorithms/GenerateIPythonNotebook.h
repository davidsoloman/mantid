#ifndef MANTID_ALGORITHMS_GENERATEIPYTHONNOTEBOOK_H
#define MANTID_ALGORITHMS_GENERATEIPYTHONNOTEBOOK_H

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"

namespace Mantid {
namespace Algorithms {

/** GenerateIPythonNotebook

  An Algorithm to generate an IPython notebook file to record the history of a
  workspace.

  Properties:
  <ul>
  <li>Filename - the name of the file to write to. </li>
  <li>InputWorkspace - the workspace name who's history is to be saved.</li>
  </ul>

  Copyright &copy; 2011 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
  National Laboratory & European Spallation Source

  This file is part of Mantid.

  Mantid is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Mantid is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.

  File change history is stored at: <https://github.com/mantidproject/mantid>
  Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport GenerateIPythonNotebook : public API::Algorithm {
public:
  GenerateIPythonNotebook() : Mantid::API::Algorithm() {}
  virtual ~GenerateIPythonNotebook() {}

  /// Algorithm's name for identification
  virtual const std::string name() const { return "GenerateIPythonNotebook"; };
  /// Summary of algorithms purpose
  virtual const std::string summary() const {
    return "An Algorithm to generate an IPython notebook file to record the "
           "history of a workspace.";
  }

  /// Algorithm's version for identification
  virtual int version() const { return 1; };
  /// Algorithm's category for identification
  virtual const std::string category() const { return "Utility\\Python"; }

protected:
  /// Initialise the properties
  void init();
  /// Run the algorithm
  void exec();
};

} // namespace Algorithms
} // namespace Mantid

#endif // MANTID_ALGORITHMS_GENERATEIPYTHONNOTEBOOK_H