#ifndef MANTID_ALGORITHMS_DIFFRACTIONEVENTCALIBRATEDETECTORS_H_
#define MANTID_ALGORITHMS_DIFFRACTIONEVENTCALIBRATEDETECTORS_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_blas.h>

namespace Mantid
{
namespace Algorithms
{
/**
 Find the offsets for each detector

 @author Vickie Lynch SNS, ORNL
 @date 12/02/2010

 Copyright &copy; 2010 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

 This file is part of Mantid.

 Mantid is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 Mantid is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
 Code Documentation is available at: <http://doxygen.mantidproject.org>
 */
class DLLExport DiffractionEventCalibrateDetectors: public API::Algorithm
{
public:
  /// Default constructor
  DiffractionEventCalibrateDetectors();
  /// Destructor
  virtual ~DiffractionEventCalibrateDetectors();
  /// Algorithm's name for identification overriding a virtual method
  virtual const std::string name() const { return "DiffractionEventCalibrateDetectors"; }
  /// Algorithm's version for identification overriding a virtual method
  virtual int version() const { return 1; }
  /// Algorithm's category for identification overriding a virtual method
  virtual const std::string category() const { return "Diffraction"; }
  /// Function to optimize
  double intensity(double x, double y, double z, double rotx, double roty, double rotz, std::string detname, std::string inname, std::string outname, std::string instname, std::string rb_param);
  void movedetector(double x, double y, double z, double rotx, double roty, double rotz, std::string detname, API::MatrixWorkspace_sptr inputW);

private:
  // Overridden Algorithm methods
  void init();
  void exec();
  // Matrix workspace pointer
  //API::MatrixWorkspace_sptr inputW;

};

} // namespace Algorithms
} // namespace Mantid

#endif /*MANTID_ALGORITHM_DIFFRACTIONEVENTCALIBRATEDETECTORS_H_*/
