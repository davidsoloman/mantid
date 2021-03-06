#include "MantidAlgorithms/AddPeak.h"
#include "MantidAPI/Axis.h"
#include "MantidAPI/IPeaksWorkspace.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/Run.h"
#include "MantidAPI/DetectorInfo.h"
#include "MantidGeometry/Crystal/IPeak.h"
#include "MantidGeometry/IDetector.h"
#include "MantidGeometry/Instrument/Goniometer.h"
#include "MantidKernel/Unit.h"
#include "MantidKernel/System.h"

using namespace Mantid::PhysicalConstants;

namespace Mantid {
namespace Algorithms {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(AddPeak)

using namespace Mantid::Kernel;
using namespace Mantid::API;

/** Initialize the algorithm's properties.
 */
void AddPeak::init() {
  declareProperty(make_unique<WorkspaceProperty<IPeaksWorkspace>>(
                      "PeaksWorkspace", "", Direction::InOut),
                  "A peaks workspace.");
  declareProperty(make_unique<WorkspaceProperty<MatrixWorkspace>>(
                      "RunWorkspace", "", Direction::Input),
                  "An input workspace containing the run information.");
  declareProperty("TOF", 0.0, "Peak position in time of flight.");
  declareProperty("DetectorID", 0, "ID of a detector at the peak centre.");
  declareProperty("Height", 0.0, "Height of the peak.");
  declareProperty("BinCount", 0.0, "Bin count.");
}

/** Execute the algorithm.
 */
void AddPeak::exec() {
  IPeaksWorkspace_sptr peaksWS = getProperty("PeaksWorkspace");
  MatrixWorkspace_sptr runWS = getProperty("RunWorkspace");

  const int detID = getProperty("DetectorID");
  double tof = getProperty("TOF");
  const double height = getProperty("Height");
  const double count = getProperty("BinCount");

  const DetectorInfo &detectorInfo = runWS->detectorInfo();
  const size_t detectorIndex = detectorInfo.indexOf(detID);

  double theta2 = detectorInfo.twoTheta(detectorIndex);
  const Mantid::Geometry::IDetector &det = detectorInfo.detector(detectorIndex);
  double phi = det.getPhi();

  // In the inelastic convention, Q = ki - kf.
  double Qx = -sin(theta2) * cos(phi);
  double Qy = -sin(theta2) * sin(phi);
  double Qz = 1.0 - cos(theta2);
  double l1 = detectorInfo.l1();
  double l2 = detectorInfo.l2(detectorIndex);

  Mantid::Kernel::Unit_sptr unit = runWS->getAxis(0)->unit();
  if (unit->unitID() != "TOF") {
    const Mantid::API::Run &run = runWS->run();
    int emode = 0;
    double efixed = 0.0;
    if (run.hasProperty("Ei")) {
      emode = 1; // direct
      if (run.hasProperty("Ei")) {
        Mantid::Kernel::Property *prop = run.getProperty("Ei");
        efixed = boost::lexical_cast<double, std::string>(prop->value());
      }
    } else if (det.hasParameter("Efixed")) {
      emode = 2; // indirect
      try {
        const Mantid::Geometry::ParameterMap &pmap =
            runWS->constInstrumentParameters();
        Mantid::Geometry::Parameter_sptr par =
            pmap.getRecursive(&det, "Efixed");
        if (par) {
          efixed = par->value<double>();
        }
      } catch (std::runtime_error &) { /* Throws if a DetectorGroup, use single
                                          provided value */
      }
    } else {
      // m_emode = 0; // Elastic
      // This should be elastic if Ei and Efixed are not set
      // TODO
    }
    std::vector<double> xdata(1, tof);
    std::vector<double> ydata;
    unit->toTOF(xdata, ydata, l1, l2, theta2, emode, efixed, 0.0);
    tof = xdata[0];
  }

  double knorm = NeutronMass * (l1 + l2) / (h_bar * tof * 1e-6) / 1e10;
  Qx *= knorm;
  Qy *= knorm;
  Qz *= knorm;

  Mantid::Geometry::IPeak *peak =
      peaksWS->createPeak(Mantid::Kernel::V3D(Qx, Qy, Qz), l2);
  peak->setDetectorID(detID);
  peak->setGoniometerMatrix(runWS->run().getGoniometer().getR());
  peak->setBinCount(count);
  peak->setRunNumber(runWS->getRunNumber());
  peak->setIntensity(height);
  if (height > 0.)
    peak->setSigmaIntensity(std::sqrt(height));

  peaksWS->addPeak(*peak);
  delete peak;
  // peaksWS->modified();
}

} // namespace Mantid
} // namespace Algorithms
