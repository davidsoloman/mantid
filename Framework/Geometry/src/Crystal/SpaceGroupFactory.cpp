#include "MantidGeometry/Crystal/SpaceGroupFactory.h"
#include "MantidGeometry/Crystal/SymmetryOperationSymbolParser.h"
#include "MantidKernel/Exception.h"

#include "MantidGeometry/Crystal/ProductOfCyclicGroups.h"
#include "MantidGeometry/Crystal/CenteringGroup.h"

#include "MantidKernel/LibraryManager.h"

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>

namespace Mantid {
namespace Geometry {

/// Free function that tries to parse the given list of symmetry operations and
/// returns true if successfull.
bool isValidGeneratorString(const std::string &generatorString) {
  std::vector<std::string> generatorStrings;
  boost::split(generatorStrings, generatorString, boost::is_any_of(";"));

  for (auto it = generatorStrings.begin(); it != generatorStrings.end(); ++it) {
    try {
      SymmetryOperationSymbolParser::parseIdentifier(*it);
    } catch (Kernel::Exception::ParseError) {
      return false;
    }
  }

  return true;
}

/// Constructor for AbstractSpaceGroupGenerator
AbstractSpaceGroupGenerator::AbstractSpaceGroupGenerator(
    size_t number, const std::string &hmSymbol,
    const std::string &generatorInformation)
    : m_number(number), m_hmSymbol(hmSymbol),
      m_generatorString(generatorInformation), m_prototype() {}

/// Returns the internally stored prototype, generates one if necessary.
SpaceGroup_const_sptr AbstractSpaceGroupGenerator::getPrototype() {
  if (!hasValidPrototype()) {
    m_prototype = generatePrototype();
  }

  return m_prototype;
}

/**
 * Generates a prototype space group object
 *
 * Constructs a space group object using a Group generated by the pure virtual
 * generateGroup()-method along with the stored number and
 * Hermann-Mauguin-symbol.
 *
 * generateGroup() has to be implemented by sub-classes and should probably use
 * the generatorInformation supplied to the constructor.
 *
 * @return SpaceGroup prototype object
 */
SpaceGroup_const_sptr AbstractSpaceGroupGenerator::generatePrototype() {
  Group_const_sptr generatingGroup = generateGroup();

  if (!generatingGroup) {
    throw std::runtime_error(
        "Could not create group from supplied symmetry operations.");
  }

  return boost::make_shared<const SpaceGroup>(m_number, m_hmSymbol,
                                              *generatingGroup);
}

/// Constructor of AlgorithmicSpaceGroupGenerator which throws an
/// std::runtime_error exception when the generatorInformation string cannot be
/// interpreted as symmetry operations.
AlgorithmicSpaceGroupGenerator::AlgorithmicSpaceGroupGenerator(
    size_t number, const std::string &hmSymbol,
    const std::string &generatorInformation)
    : AbstractSpaceGroupGenerator(number, hmSymbol, generatorInformation) {
  if (!isValidGeneratorString(generatorInformation)) {
    throw std::runtime_error("Generator string could not be parsed: " +
                             generatorInformation);
  }
}

/// Uses an algorithm based on Shmueli, U. Acta Crystallogr. A 40, 559–567
/// (1984) to generate the group.
Group_const_sptr AlgorithmicSpaceGroupGenerator::generateGroup() const {
  Group_const_sptr baseGroup =
      GroupFactory::create<ProductOfCyclicGroups>(getGeneratorString());
  Group_const_sptr centeringGroup =
      GroupFactory::create<CenteringGroup>(getCenteringSymbol());

  return baseGroup * centeringGroup;
}

/// Returns the centering symbol, which is extracted from the
/// Hermann-Mauguin-symbol.
std::string AlgorithmicSpaceGroupGenerator::getCenteringSymbol() const {
  return getHMSymbol().substr(0, 1);
}

/// Constructor of TabulatedSpaceGroupGenerator, throws an std::runtime_error
/// exception if generatorInformation cannot be intepreted as symmetry
/// operations.
TabulatedSpaceGroupGenerator::TabulatedSpaceGroupGenerator(
    size_t number, const std::string &hmSymbol,
    const std::string &generatorInformation)
    : AbstractSpaceGroupGenerator(number, hmSymbol, generatorInformation) {
  if (!isValidGeneratorString(generatorInformation)) {
    throw std::runtime_error("Generator string could not be parsed: " +
                             generatorInformation);
  }
}

/// Returns a group that contains the symmetry operations in
Group_const_sptr TabulatedSpaceGroupGenerator::generateGroup() const {
  return GroupFactory::create<Group>(getGeneratorString());
}

/// Creates a space group given the Hermann-Mauguin symbol, throws
/// std::invalid_argument if symbol is not registered.
SpaceGroup_const_sptr
SpaceGroupFactoryImpl::createSpaceGroup(const std::string &hmSymbol) {
  if (!isSubscribed(hmSymbol)) {
    throw std::invalid_argument("Space group with symbol '" + hmSymbol +
                                "' is not registered.");
  }

  return constructFromPrototype(getPrototype(hmSymbol));
}

/// Returns true if space group with given symbol is subscribed.
bool SpaceGroupFactoryImpl::isSubscribed(const std::string &hmSymbol) const {
  return m_generatorMap.find(hmSymbol) != m_generatorMap.end();
}

/// Returns true if space group with given number is subscribed.
bool SpaceGroupFactoryImpl::isSubscribed(size_t number) const {
  return m_numberMap.find(number) != m_numberMap.end();
}

/// Returns a vector with all subscribed space group symbols.
std::vector<std::string>
SpaceGroupFactoryImpl::subscribedSpaceGroupSymbols() const {
  std::vector<std::string> symbols;
  symbols.reserve(m_generatorMap.size());

  for (auto it = m_generatorMap.begin(); it != m_generatorMap.end(); ++it) {
    symbols.push_back(it->first);
  }

  return symbols;
}

/// Returns a vector with all symbols that correspond to a space group number
std::vector<std::string>
SpaceGroupFactoryImpl::subscribedSpaceGroupSymbols(size_t number) const {
  std::vector<std::string> symbols;

  auto keyPair = m_numberMap.equal_range(number);

  for (auto it = keyPair.first; it != keyPair.second; ++it) {
    symbols.push_back(it->second);
  }

  return symbols;
}

/// Returns a vector with all subscribed space group numbers.
std::vector<size_t> SpaceGroupFactoryImpl::subscribedSpaceGroupNumbers() const {
  std::vector<size_t> numbers;
  numbers.reserve(m_numberMap.size());

  for (auto it = m_numberMap.begin(); it != m_numberMap.end();
       it = m_numberMap.upper_bound(it->first)) {
    numbers.push_back(it->first);
  }

  return numbers;
}

std::vector<std::string> SpaceGroupFactoryImpl::subscribedSpaceGroupSymbols(
    const PointGroup_sptr &pointGroup) {
  if (m_pointGroupMap.empty()) {
    fillPointGroupMap();
  }

  std::string pointGroupSymbol = pointGroup->getSymbol();

  std::vector<std::string> symbols;
  auto keyPair = m_pointGroupMap.equal_range(pointGroupSymbol);

  for (auto it = keyPair.first; it != keyPair.second; ++it) {
    symbols.push_back(it->second);
  }

  return symbols;
}

/// Unsubscribes the space group with the given Hermann-Mauguin symbol, but
/// throws std::invalid_argument if symbol is not registered.
void SpaceGroupFactoryImpl::unsubscribeSpaceGroup(const std::string &hmSymbol) {
  if (!isSubscribed(hmSymbol)) {
    throw std::invalid_argument(
        "Cannot unsubscribe space group that is not registered.");
  }

  auto eraseGenerator = m_generatorMap.find(hmSymbol);
  AbstractSpaceGroupGenerator_sptr generator = eraseGenerator->second;

  auto eraseNumber = m_numberMap.find(generator->getNumber());

  m_numberMap.erase(eraseNumber);
  m_generatorMap.erase(eraseGenerator);
}

/**
 * Subscribes a space group into the factory using generators
 *
 * With this method one can register a space group that is generated by an
 * algorithm based on the instructions in [1]. Currently it's important that
 * the Herrman-Mauguin symbol starts with an upper case letter, because that is
 * used to generate centering translations (it should be upper case anyway).
 *
 * The method will throw an exception if the number or symbol is already
 * registered.
 *
 * [1] Shmueli, U. Acta Crystallogr. A 40, 559–567 (1984).
 *     http://dx.doi.org/10.1107/S0108767384001161
 *
 * @param number :: Space group number (ITA)
 * @param hmSymbol :: Herrman-Mauguin symbol with upper case first letter
 * @param generators :: Generating symmetry operations.
 */
void SpaceGroupFactoryImpl::subscribeGeneratedSpaceGroup(
    size_t number, const std::string &hmSymbol, const std::string &generators) {
  subscribeUsingGenerator<AlgorithmicSpaceGroupGenerator>(number, hmSymbol,
                                                          generators);
}

/// Subscribes a "tabulated space group" into the factory where all symmetry
/// operations need to be supplied, including centering.
void SpaceGroupFactoryImpl::subscribeTabulatedSpaceGroup(
    size_t number, const std::string &hmSymbol,
    const std::string &symmetryOperations) {
  subscribeUsingGenerator<TabulatedSpaceGroupGenerator>(number, hmSymbol,
                                                        symmetryOperations);
}

/// Returns a copy-constructed instance of the supplied space group prototype
/// object.
SpaceGroup_const_sptr SpaceGroupFactoryImpl::constructFromPrototype(
    const SpaceGroup_const_sptr prototype) const {
  return boost::make_shared<const SpaceGroup>(*prototype);
}

/// Fills the internal multimap that maintains the mapping between space and
/// point groups.
void SpaceGroupFactoryImpl::fillPointGroupMap() {
  m_pointGroupMap.clear();

  for (auto it = m_generatorMap.begin(); it != m_generatorMap.end(); ++it) {
    SpaceGroup_const_sptr spaceGroup = getPrototype(it->first);

    m_pointGroupMap.insert(
        std::make_pair(spaceGroup->getPointGroup()->getSymbol(), it->first));
  }
}

/// Returns a prototype object for the requested space group.
SpaceGroup_const_sptr
SpaceGroupFactoryImpl::getPrototype(const std::string &hmSymbol) {
  AbstractSpaceGroupGenerator_sptr generator =
      m_generatorMap.find(hmSymbol)->second;

  if (!generator) {
    throw std::runtime_error("No generator for symbol '" + hmSymbol + "'");
  }

  return generator->getPrototype();
}

/// Puts the space group factory into the factory.
void SpaceGroupFactoryImpl::subscribe(
    const AbstractSpaceGroupGenerator_sptr &generator) {
  if (!generator) {
    throw std::runtime_error("Cannot register null-generator.");
  }

  m_numberMap.insert(
      std::make_pair(generator->getNumber(), generator->getHMSymbol()));
  m_generatorMap.insert(std::make_pair(generator->getHMSymbol(), generator));

  // Clear the point group map
  m_pointGroupMap.clear();
}

/// Constructor cannot be called, since SingletonHolder is used.
SpaceGroupFactoryImpl::SpaceGroupFactoryImpl()
    : m_numberMap(), m_generatorMap(), m_pointGroupMap() {
  Kernel::LibraryManager::Instance();
}

/* Space groups according to International Tables for Crystallography,
 * using the generators specified there.
 *
 * When two origin choices are possible, only the first is given.
 */
// Triclinic
DECLARE_TABULATED_SPACE_GROUP(1, "P 1", "x,y,z")
DECLARE_GENERATED_SPACE_GROUP(2, "P -1", "-x,-y,-z")

// Monoclinic
DECLARE_GENERATED_SPACE_GROUP(3, "P 1 2 1", "-x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(4, "P 1 21 1", "-x,y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(5, "C 1 2 1", "-x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(6, "P 1 m 1", "x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(7, "P 1 c 1", "x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(8, "C 1 m 1", "x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(9, "C 1 c 1", "x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(10, "P 1 2/m 1", "-x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(11, "P 1 21/m 1", "-x,y+1/2,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(12, "C 1 2/m 1", "-x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(13, "P 1 2/c 1", "-x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(14, "P 1 21/c 1", "-x,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(15, "C 1 2/c 1", "-x,y,-z+1/2; -x,-y,-z")

// Orthorhombic
DECLARE_GENERATED_SPACE_GROUP(16, "P 2 2 2", "-x,y,-z; x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(17, "P 2 2 21", "-x,-y,z+1/2; -x,y,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(18, "P 21 21 2", "-x,-y,z; -x+1/2,y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(19, "P 21 21 21",
                              "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(20, "C 2 2 21", "-x,-y,z+1/2; -x,y,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(21, "C 2 2 2", "-x,-y,z; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(22, "F 2 2 2", "-x,-y,z; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(23, "I 2 2 2", "-x,-y,z; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(24, "I 21 21 21",
                              "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(25, "P m m 2", "-x,-y,z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(26, "P m c 21", "-x,-y,z+1/2; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(27, "P c c 2", "-x,-y,z; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(28, "P m a 2", "-x,-y,z; x+1/2,-y,z")
DECLARE_GENERATED_SPACE_GROUP(29, "P c a 21", "-x,-y,z+1/2; x+1/2,-y,z")
DECLARE_GENERATED_SPACE_GROUP(30, "P n c 2", "-x,-y,z; x,-y+1/2,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(31, "P m n 21", "-x+1/2,-y,z+1/2; x+1/2,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(32, "P b a 2", "-x,-y,z; x+1/2,-y+1/2,z")
DECLARE_GENERATED_SPACE_GROUP(33, "P n a 21", "-x,-y,z+1/2; x+1/2,-y+1/2,z")
DECLARE_GENERATED_SPACE_GROUP(34, "P n n 2", "-x,-y,z; x+1/2,-y+1/2,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(35, "C m m 2", "-x,-y,z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(36, "C m c 21", "-x,-y,z+1/2; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(37, "C c c 2", "-x,-y,z; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(38, "A m m 2", "-x,-y,z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(39, "A e m 2", "-x,-y,z; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(40, "A m a 2", "-x,-y,z; x+1/2,-y,z")
DECLARE_GENERATED_SPACE_GROUP(41, "A e a 2", "-x,-y,z; x+1/2,-y+1/2,z")
DECLARE_GENERATED_SPACE_GROUP(42, "F m m 2", "-x,-y,z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(43, "F d d 2", "-x,-y,z; x+1/4,-y+1/4,z+1/4")
DECLARE_GENERATED_SPACE_GROUP(44, "I m m 2", "-x,-y,z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(45, "I b a 2", "-x,-y,z; x+1/2,-y+1/2,z")
DECLARE_GENERATED_SPACE_GROUP(46, "I m a 2", "-x,-y,z; x+1/2,-y,z")
DECLARE_GENERATED_SPACE_GROUP(47, "P m m m", "-x,-y,z; -x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(48, "P n n n",
                              "-x,-y,z; -x,y,-z; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(49, "P c c m", "-x,-y,z; -x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(50, "P b a n",
                              "-x,-y,z; -x,y,-z; -x+1/2,-y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(51, "P m m a", "-x+1/2,-y,z; -x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(52, "P n n a",
                              "-x+1/2,-y,z; -x+1/2,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(53, "P m n a",
                              "-x+1/2,-y,z+1/2; -x+1/2,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(54, "P c c a",
                              "-x+1/2,-y,z; -x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(55, "P b a m",
                              "-x,-y,z; -x+1/2,y+1/2,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(56, "P c c n",
                              "-x+1/2,-y+1/2,z; -x,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(57, "P b c m",
                              "-x,-y,z+1/2; -x,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(58, "P n n m",
                              "-x,-y,z; -x+1/2,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(59, "P m m n",
                              "-x,-y,z; -x+1/2,y+1/2,-z; -x+1/2,-y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(60, "P b c n",
                              "-x+1/2,-y+1/2,z+1/2; -x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(61, "P b c a",
                              "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(62, "P n m a",
                              "-x+1/2,-y,z+1/2; -x,y+1/2,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(63, "C m c m",
                              "-x,-y,z+1/2; -x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(64, "C m c e",
                              "-x,-y+1/2,z+1/2; -x,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(65, "C m m m", "-x,-y,z; -x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(66, "C c c m", "-x,-y,z; -x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(67, "C m m e",
                              "-x,-y+1/2,z; -x,y+1/2,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(68, "C c c e",
                              "-x+1/2,-y+1/2,z; -x,y,-z; -x,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(69, "F m m m", "-x,-y,z; -x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(70, "F d d d",
                              "-x,-y,z; -x,y,-z; -x+1/4,-y+1/4,-z+1/4")
DECLARE_GENERATED_SPACE_GROUP(71, "I m m m", "-x,-y,z; -x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(72, "I b a m",
                              "-x,-y,z; -x+1/2,y+1/2,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(73, "I b c a",
                              "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(74, "I m m a",
                              "-x,-y+1/2,z; -x,y+1/2,-z; -x,-y,-z")

// Tetragonal
DECLARE_GENERATED_SPACE_GROUP(75, "P 4", "-x,-y,z; -y,x,z")
DECLARE_GENERATED_SPACE_GROUP(76, "P 41", "-x,-y,z+1/2; -y,x,z+1/4")
DECLARE_GENERATED_SPACE_GROUP(77, "P 42", "-x,-y,z; -y,x,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(78, "P 43", "-x,-y,z+1/2; -y,x,z+3/4")
DECLARE_GENERATED_SPACE_GROUP(79, "I 4", "-x,-y,z; -y,x,z")
DECLARE_GENERATED_SPACE_GROUP(80, "I 41", "-x+1/2,-y+1/2,z+1/2; -y,x+1/2,z+1/4")
DECLARE_GENERATED_SPACE_GROUP(81, "P -4", "-x,-y,z; y,-x,-z")
DECLARE_GENERATED_SPACE_GROUP(82, "I -4", "-x,-y,z; y,-x,-z")
DECLARE_GENERATED_SPACE_GROUP(83, "P 4/m", "-x,-y,z; -y,x,z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(84, "P 42/m", "-x,-y,z; -y,x,z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(85, "P 4/n",
                              "-x,-y,z; -y+1/2,x+1/2,z; -x+1/2,-y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(
    86, "P 42/n", "-x,-y,z; -y+1/2,x+1/2,z+1/2; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(87, "I 4/m", "-x,-y,z; -y,x,z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    88, "I 41/a", "-x+1/2,-y+1/2,z+1/2; -y,x+1/2,z+1/4; -x,-y+1/2,-z+1/4")
DECLARE_GENERATED_SPACE_GROUP(89, "P 4 2 2", "-x,-y,z; -y,x,z; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(90, "P 4 21 2",
                              "-x,-y,z; -y+1/2,x+1/2,z; -x+1/2,y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(91, "P 41 2 2",
                              "-x,-y,z+1/2; -y,x,z+1/4; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    92, "P 41 21 2", "-x,-y,z+1/2; -y+1/2,x+1/2,z+1/4; -x+1/2,y+1/2,-z+1/4")
DECLARE_GENERATED_SPACE_GROUP(93, "P 42 2 2", "-x,-y,z; -y,x,z+1/2; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    94, "P 42 21 2", "-x,-y,z; -y+1/2,x+1/2,z+1/2; -x+1/2,y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(95, "P 43 2 2",
                              "-x,-y,z+1/2; -y,x,z+3/4; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    96, "P 43 21 2", "-x,-y,z+1/2; -y+1/2,x+1/2,z+3/4; -x+1/2,y+1/2,-z+3/4")
DECLARE_GENERATED_SPACE_GROUP(97, "I 4 2 2", "-x,-y,z; -y,x,z; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    98, "I 41 2 2", "-x+1/2,-y+1/2,z+1/2; -y,x+1/2,z+1/4; -x+1/2,y,-z+3/4")
DECLARE_GENERATED_SPACE_GROUP(99, "P 4 m m", "-x,-y,z; -y,x,z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(100, "P 4 b m", "-x,-y,z; -y,x,z; x+1/2,-y+1/2,z")
DECLARE_GENERATED_SPACE_GROUP(101, "P 42 c m",
                              "-x,-y,z; -y,x,z+1/2; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(102, "P 42 n m",
                              "-x,-y,z; -y+1/2,x+1/2,z+1/2; x+1/2,-y+1/2,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(103, "P 4 c c", "-x,-y,z; -y,x,z; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(104, "P 4 n c",
                              "-x,-y,z; -y,x,z; x+1/2,-y+1/2,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(105, "P 42 m c", "-x,-y,z; -y,x,z+1/2; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(106, "P 43 b c",
                              "-x,-y,z; -y,x,z+1/2; x+1/2,-y+1/2,z")
DECLARE_GENERATED_SPACE_GROUP(107, "I 4 m m", "-x,-y,z; -y,x,z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(108, "I 4 c m", "-x,-y,z; -y,x,z; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(109, "I 41 m d",
                              "-x+1/2,-y+1/2,z+1/2; -y,x+1/2,z+1/4; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(110, "I 41 c d",
                              "-x+1/2,-y+1/2,z+1/2; -y,x+1/2,z+1/4; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(111, "P -4 2 m", "-x,-y,z; y,-x,-z; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(112, "P -4 2 c", "-x,-y,z; y,-x,-z; -x,y,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(113, "P -4 21 m",
                              "-x,-y,z; y,-x,-z; -x+1/2,y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(114, "P -4 21 c",
                              "-x,-y,z; y,-x,-z; -x+1/2,y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(115, "P -4 m 2", "-x,-y,z; y,-x,-z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(116, "P -4 c 2", "-x,-y,z; y,-x,-z; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(117, "P -4 b 2",
                              "-x,-y,z; y,-x,-z; x+1/2,-y+1/2,z")
DECLARE_GENERATED_SPACE_GROUP(118, "P -4 n 2",
                              "-x,-y,z; y,-x,-z; x+1/2,-y+1/2,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(119, "I -4 m 2", "-x,-y,z; y,-x,-z; x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(120, "I -4 c 2", "-x,-y,z; y,-x,-z; x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(121, "I -4 2 m", "-x,-y,z; y,-x,-z; -x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(122, "I -4 2 d",
                              "-x,-y,z; y,-x,-z; -x+1/2,y,-z+3/4")
DECLARE_GENERATED_SPACE_GROUP(123, "P 4/m m m",
                              "-x,-y,z; -y,x,z; -x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(124, "P 4/m c c",
                              "-x,-y,z; -y,x,z; -x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(125, "P 4/n b m",
                              "-x,-y,z; -y,x,z; -x,y,-z; -x+1/2,-y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(126, "P 4/n n c",
                              "-x,-y,z; -y,x,z; -x,y,-z; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(127, "P 4/m b m",
                              "-x,-y,z; -y,x,z; -x+1/2,y+1/2,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(128, "P 4/m n c",
                              "-x,-y,z; -y,x,z; -x+1/2,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    129, "P 4/n m m",
    "-x,-y,z; -y+1/2,x+1/2,z; -x+1/2,y+1/2,-z; -x+1/2,-y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(
    130, "P 4/n c c",
    "-x,-y,z; -y+1/2,x+1/2,z; -x+1/2,y+1/2,-z+1/2; -x+1/2,-y+1/2,-z")
DECLARE_GENERATED_SPACE_GROUP(131, "P 42/m m c",
                              "-x,-y,z; -y,x,z+1/2; -x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(132, "P 42/m c m",
                              "-x,-y,z; -y,x,z+1/2; -x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    133, "P 42/n b c",
    "-x,-y,z; -y+1/2,x+1/2,z+1/2; -x,y,-z+1/2; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(
    134, "P 42/n n m",
    "-x,-y,z; -y+1/2,x+1/2,z+1/2; -x,y,-z; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(135, "P 42/m b c",
                              "-x,-y,z; -y,x,z+1/2; -x+1/2,y+1/2,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    136, "P 42/m n m",
    "-x,-y,z; -y+1/2,x+1/2,z+1/2; -x+1/2,y+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    137, "P 42/n m c",
    "-x,-y,z; -y+1/2,x+1/2,z+1/2; -x+1/2,y+1/2,-z+1/2; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(
    138, "P 42/n c m",
    "-x,-y,z; -y+1/2,x+1/2,z+1/2; -x+1/2,y+1/2,-z; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(139, "I 4/m m m",
                              "-x,-y,z; -y,x,z; -x,y,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(140, "I 4/m c m",
                              "-x,-y,z; -y,x,z; -x,y,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    141, "I 41/a m d",
    "-x+1/2,-y+1/2,z+1/2; -y,x+1/2,z+1/4; -x+1/2,y,-z+3/4; -x,-y+1/2,-z+1/4")
DECLARE_GENERATED_SPACE_GROUP(
    142, "I 41/a c d",
    "-x+1/2,-y+1/2,z+1/2; -y,x+1/2,z+1/4; -x+1/2,y,-z+1/4; -x,-y+1/2,-z+1/4")

// Trigonal
DECLARE_GENERATED_SPACE_GROUP(143, "P 3", "-y,x-y,z")
DECLARE_GENERATED_SPACE_GROUP(144, "P 31", "-y,x-y,z+1/3")
DECLARE_GENERATED_SPACE_GROUP(145, "P 32", "-y,x-y,z+2/3")
DECLARE_GENERATED_SPACE_GROUP(146, "R 3", "-y,x-y,z")
DECLARE_GENERATED_SPACE_GROUP(147, "P -3", "-y,x-y,z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(148, "R -3", "-y,x-y,z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(149, "P 3 1 2", "-y,x-y,z; -y,-x,-z")
DECLARE_GENERATED_SPACE_GROUP(150, "P 3 2 1", "-y,x-y,z; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(151, "P 31 1 2", "-y,x-y,z+1/3; -y,-x,-z+2/3")
DECLARE_GENERATED_SPACE_GROUP(152, "P 31 2 1", "-y,x-y,z+1/3; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(153, "P 32 1 2", "-y,x-y,z+2/3; -y,-x,-z+1/3")
DECLARE_GENERATED_SPACE_GROUP(154, "P 32 2 1", "-y,x-y,z+2/3; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(155, "R 32", "-y,x-y,z; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(156, "P 3 m 1", "-y,x-y,z; -y,-x,z")
DECLARE_GENERATED_SPACE_GROUP(157, "P 3 1 m", "-y,x-y,z; y,x,z")
DECLARE_GENERATED_SPACE_GROUP(158, "P 3 c 1", "-y,x-y,z; -y,-x,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(159, "P 3 1 c", "-y,x-y,z; y,x,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(160, "R 3 m", "-y,x-y,z; -y,-x,z")
DECLARE_GENERATED_SPACE_GROUP(161, "R 3 c", "-y,x-y,z; -y,-x,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(162, "P -3 1 m", "-y,x-y,z; -y,-x,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(163, "P -3 1 c",
                              "-y,x-y,z; -y,-x,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(164, "P -3 m 1", "-y,x-y,z; y,x,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(165, "P -3 c 1", "-y,x-y,z; y,x,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(166, "R -3 m", "-y,x-y,z; y,x,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(167, "R -3 c", "-y,x-y,z; y,x,-z+1/2; -x,-y,-z")

// Hexagonal
DECLARE_GENERATED_SPACE_GROUP(168, "P 6", "-y,x-y,z; -x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(169, "P 61", "-y,x-y,z+1/3; -x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(170, "P 65", "-y,x-y,z+2/3; -x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(171, "P 62", "-y,x-y,z+2/3; -x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(172, "P 64", "-y,x-y,z+1/3; -x,-y,z")
DECLARE_GENERATED_SPACE_GROUP(173, "P 63", "-y,x-y,z; -x,-y,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(174, "P -6", "-y,x-y,z; x,y,-z")
DECLARE_GENERATED_SPACE_GROUP(175, "P 6/m", "-y,x-y,z; -x,-y,z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(176, "P 63/m", "-y,x-y,z; -x,-y,z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(177, "P 6 2 2", "-y,x-y,z; -x,-y,z; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(178, "P 61 2 2",
                              "-y,x-y,z+1/3; -x,-y,z+1/2; y,x,-z+1/3")
DECLARE_GENERATED_SPACE_GROUP(179, "P 65 2 2",
                              "-y,x-y,z+2/3; -x,-y,z+1/2; y,x,-z+2/3")
DECLARE_GENERATED_SPACE_GROUP(180, "P 62 2 2",
                              "-y,x-y,z+2/3; -x,-y,z; y,x,-z+2/3")
DECLARE_GENERATED_SPACE_GROUP(181, "P 64 2 2",
                              "-y,x-y,z+1/3; -x,-y,z; y,x,-z+1/3")
DECLARE_GENERATED_SPACE_GROUP(182, "P 63 2 2", "-y,x-y,z; -x,-y,z+1/2; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(183, "P 6 m m", "-y,x-y,z; -x,-y,z; -y,-x,z")
DECLARE_GENERATED_SPACE_GROUP(184, "P 6 c c", "-y,x-y,z; -x,-y,z; -y,-x,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(185, "P 63 c m",
                              "-y,x-y,z; -x,-y,z+1/2; -y,-x,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(186, "P 63 m c", "-y,x-y,z; -x,-y,z+1/2; -y,-x,z")
DECLARE_GENERATED_SPACE_GROUP(187, "P -6 m 2", "-y,x-y,z; x,y,-z; -y,-x,z")
DECLARE_GENERATED_SPACE_GROUP(188, "P -6 c 2",
                              "-y,x-y,z; x,y,-z+1/2; -y,-x,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(189, "P -6 2 m", "-y,x-y,z; x,y,-z; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(190, "P -6 2 c", "-y,x-y,z; x,y,-z+1/2; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(191, "P 6/m m m",
                              "-y,x-y,z; x,y,-z; y,x,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(192, "P 6/m c c",
                              "-y,x-y,z; -x,-y,z; y,x,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(193, "P 63/m c m",
                              "-y,x-y,z; -x,-y,z+1/2; y,x,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(194, "P 63/m m c",
                              "-y,x-y,z; -x,-y,z+1/2; y,x,-z; -x,-y,-z")

// Cubic
DECLARE_GENERATED_SPACE_GROUP(195, "P 2 3", "-x,-y,z; -x,y,-z; z,x,y")
DECLARE_GENERATED_SPACE_GROUP(196, "F 2 3", "-x,-y,z; -x,y,-z; z,x,y")
DECLARE_GENERATED_SPACE_GROUP(197, "I 2 3", "-x,-y,z; -x,y,-z; z,x,y")
DECLARE_GENERATED_SPACE_GROUP(198, "P 21 3",
                              "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y")
DECLARE_GENERATED_SPACE_GROUP(199, "I 21 3",
                              "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y")
DECLARE_GENERATED_SPACE_GROUP(200, "P m -3",
                              "-x,-y,z; -x,y,-z; z,x,y; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(201, "P n -3",
                              "-x,-y,z; -x,y,-z; z,x,y; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(202, "F m -3",
                              "-x,-y,z; -x,y,-z; z,x,y; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(203, "F d -3",
                              "-x,-y,z; -x,y,-z; z,x,y; -x+1/4,-y+1/4,-z+1/4")
DECLARE_GENERATED_SPACE_GROUP(204, "I m -3",
                              "-x,-y,z; -x,y,-z; z,x,y; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    205, "P a -3", "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    206, "I a -3", "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(207, "P 4 3 2", "-x,-y,z; -x,y,-z; z,x,y; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(208, "P 42 3 2",
                              "-x,-y,z; -x,y,-z; z,x,y; y+1/2,x+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(209, "F 4 3 2", "-x,-y,z; -x,y,-z; z,x,y; y,x,-z")

DECLARE_GENERATED_SPACE_GROUP(
    210, "F 41 3 2",
    "-x,-y+1/2,z+1/2; -x+1/2,y+1/2,-z; z,x,y; y+3/4,x+1/4,-z+3/4")
DECLARE_GENERATED_SPACE_GROUP(211, "I 4 3 2", "-x,-y,z; -x,y,-z; z,x,y; y,x,-z")
DECLARE_GENERATED_SPACE_GROUP(
    212, "P 43 3 2",
    "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y; y+1/4,x+3/4,-z+3/4")
DECLARE_GENERATED_SPACE_GROUP(
    213, "P 41 3 2",
    "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y; y+3/4,x+1/4,-z+1/4")
DECLARE_GENERATED_SPACE_GROUP(
    214, "I 41 3 2",
    "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y; y+3/4,x+1/4,-z+1/4")
DECLARE_GENERATED_SPACE_GROUP(215, "P -4 3 m", "-x,-y,z; -x,y,-z; z,x,y; y,x,z")
DECLARE_GENERATED_SPACE_GROUP(216, "F -4 3 m", "-x,-y,z; -x,y,-z; z,x,y; y,x,z")
DECLARE_GENERATED_SPACE_GROUP(217, "I -4 3 m", "-x,-y,z; -x,y,-z; z,x,y; y,x,z")
DECLARE_GENERATED_SPACE_GROUP(218, "P -4 3 n",
                              "-x,-y,z; -x,y,-z; z,x,y; y+1/2,x+1/2,z+1/2")
DECLARE_GENERATED_SPACE_GROUP(219, "F -4 3 c",
                              "-x,-y,z; -x,y,-z; z,x,y; y+1/2,x+1/2,z+1/2")

DECLARE_GENERATED_SPACE_GROUP(
    220, "I -4 3 d",
    "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y; y+1/4,x+1/4,z+1/4")
DECLARE_GENERATED_SPACE_GROUP(221, "P m -3 m",
                              "-x,-y,z; -x,y,-z; z,x,y; y,x,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    222, "P n -3 n", "-x,-y,z; -x,y,-z; z,x,y; y,x,-z; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(
    223, "P m -3 n", "-x,-y,z; -x,y,-z; z,x,y; y+1/2,x+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    224, "P n -3 m",
    "-x,-y,z; -x,y,-z; z,x,y; y+1/2,x+1/2,-z+1/2; -x+1/2,-y+1/2,-z+1/2")
DECLARE_GENERATED_SPACE_GROUP(225, "F m -3 m",
                              "-x,-y,z; -x,y,-z; z,x,y; y,x,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    226, "F m -3 c", "-x,-y,z; -x,y,-z; z,x,y; y+1/2,x+1/2,-z+1/2; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(227, "F d -3 m",
                              "-x,-y+1/2,z+1/2; -x+1/2,y+1/2,-z; z,x,y; "
                              "y+3/4,x+1/4,-z+3/4; -x+1/4,-y+1/4,-z+1/4")
DECLARE_GENERATED_SPACE_GROUP(228, "F d -3 c",
                              "-x,-y+1/2,z+1/2; -x+1/2,y+1/2,-z; z,x,y; "
                              "y+3/4,x+1/4,-z+3/4; -x+3/4,-y+3/4,-z+3/4")
DECLARE_GENERATED_SPACE_GROUP(229, "I m -3 m",
                              "-x,-y,z; -x,y,-z; z,x,y; y,x,-z; -x,-y,-z")
DECLARE_GENERATED_SPACE_GROUP(
    230, "I a -3 d",
    "-x+1/2,-y,z+1/2; -x,y+1/2,-z+1/2; z,x,y; y+3/4,x+1/4,-z+1/4; -x,-y,-z")

} // namespace Geometry
} // namespace Mantid