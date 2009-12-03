#ifndef FILEPROPERTYTEST_H_
#define FILEPROPERTYTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidKernel/FileProperty.h"
#include "MantidKernel/ConfigService.h"

class FilePropertyTest : public CxxTest::TestSuite
{
public:

  void testSearchDirs()
  {
    // It wasn't happy having this in the constructor as I think all of the objects in the test
    //get created first and then all of the tests run
    Mantid::Kernel::ConfigService::Instance().loadConfig("Mantid.properties");
    TS_ASSERT_DIFFERS(Mantid::Kernel::ConfigService::Instance().getDataSearchDirs().size(), 0);
  }
  
  void testLoadPropertyNoExtension()
  {
    Mantid::Kernel::FileProperty *fp = 
      new Mantid::Kernel::FileProperty("Filename","", Mantid::Kernel::FileProperty::Load);

    // Check type
    TS_ASSERT_EQUALS(fp->isLoadProperty(), true)

    ///Test a GEM file in the test directory
    std::string msg = fp->setValue("GEM38370.raw");
    TS_ASSERT_EQUALS(msg, "")    
    delete fp;
  }

  void testLoadPropertyWithExtension()
  {
    std::vector<std::string> exts(1, "raw");
    Mantid::Kernel::FileProperty *fp = 
      new Mantid::Kernel::FileProperty("Filename","", Mantid::Kernel::FileProperty::Load, exts);
    // Check type
    TS_ASSERT_EQUALS(fp->isLoadProperty(), true)
      
    ///Test a GEM file in the test directory
    std::string msg = fp->setValue("GEM38370.raw");
    TS_ASSERT_EQUALS(msg, "")    

    // Test case-insensitivity
    msg = fp->setValue("ALF15739.RAW");
    TS_ASSERT_EQUALS(msg, "")    

    //Check different extension
    msg = fp->setValue("48098.Q");
    TS_ASSERT_EQUALS(msg, ""); 

    delete fp;
  }

  void testNoExistLoadProperty()
  {
    std::vector<std::string> exts(1, "raw");
    Mantid::Kernel::FileProperty *fp = 
      new Mantid::Kernel::FileProperty("Filename","", Mantid::Kernel::FileProperty::NoExistLoad, exts);
    // Check type
    TS_ASSERT_EQUALS(fp->isLoadProperty(), true)

    std::string msg = fp->setValue("GEM38370.raw");
    TS_ASSERT_EQUALS(msg, "")    

    msg = fp->setValue("GEM38371.raw");
    TS_ASSERT_EQUALS(msg, "")    

  }

  void testSaveProperty()
  {
    Mantid::Kernel::FileProperty *fp = 
      new Mantid::Kernel::FileProperty("Filename","", Mantid::Kernel::FileProperty::Save);
    // Check type
    TS_ASSERT_EQUALS(fp->isLoadProperty(), false)

    //Test for some random file name as this doesn't need to exist here
    std::string msg = fp->setValue("filepropertytest.sav");
    TS_ASSERT_EQUALS(msg, "")
  }

};

#endif
