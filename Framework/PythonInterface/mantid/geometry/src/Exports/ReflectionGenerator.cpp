#include "MantidGeometry/Crystal/ReflectionGenerator.h"
#include "MantidPythonInterface/kernel/Converters/PySequenceToVector.h"
#include <boost/python/class.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/list.hpp>

using namespace Mantid::Geometry;
using namespace Mantid::Kernel;
using namespace Mantid::PythonInterface;
using namespace boost::python;

namespace {
boost::python::list getListFromV3DVector(const std::vector<V3D> &hkls) {
  boost::python::list hklList;
  for (auto it = hkls.begin(); it != hkls.end(); ++it) {
    hklList.append(*it);
  }
  return hklList;
}

boost::python::list getHKLsDefaultFilter(ReflectionGenerator &self, double dMin,
                                         double dMax) {
  return getListFromV3DVector(self.getHKLs(dMin, dMax));
}

boost::python::list getHKLsUsingFilter(ReflectionGenerator &self, double dMin,
                                       double dMax,
                                       ReflectionConditionFilter filter) {
  return getListFromV3DVector(
      self.getHKLs(dMin, dMax, self.getReflectionConditionFilter(filter)));
}

boost::python::list getUniqueHKLsDefaultFilter(ReflectionGenerator &self,
                                               double dMin, double dMax) {
  return getListFromV3DVector(self.getUniqueHKLs(dMin, dMax));
}

boost::python::list getUniqueHKLsUsingFilter(ReflectionGenerator &self,
                                             double dMin, double dMax,
                                             ReflectionConditionFilter filter) {
  return getListFromV3DVector(self.getUniqueHKLs(
      dMin, dMax, self.getReflectionConditionFilter(filter)));
}

std::vector<double> getDValues(ReflectionGenerator &self,
                               const boost::python::object &hkls) {
  Converters::PySequenceToVector<V3D> converter(hkls);

  return self.getDValues(converter());
}

std::vector<double> getFsSquared(ReflectionGenerator &self,
                                 const boost::python::object &hkls) {
  Converters::PySequenceToVector<V3D> converter(hkls);

  return self.getFsSquared(converter());
}
}

void export_ReflectionGenerator() {
  enum_<ReflectionConditionFilter>("ReflectionConditionFilter")
      .value("None", ReflectionConditionFilter::None)
      .value("Centering", ReflectionConditionFilter::Centering)
      .value("SpaceGroup", ReflectionConditionFilter::SpaceGroup)
      .value("StructureFactor", ReflectionConditionFilter::StructureFactor)
      .export_values();

  class_<ReflectionGenerator>("ReflectionGenerator", no_init)
      .def(init<const CrystalStructure &, optional<ReflectionConditionFilter>>(
          (arg("crystalStructure"), arg("defaultFilter"))))
      .def("getHKLs", &getHKLsDefaultFilter)
      .def("getHKLsUsingFilter", &getHKLsUsingFilter)
      .def("getUniqueHKLs", &getUniqueHKLsDefaultFilter)
      .def("getUniqueHKLsUsingFilter", &getUniqueHKLsUsingFilter)
      .def("getDValues", &getDValues)
      .def("getFsSquared", &getFsSquared);
}