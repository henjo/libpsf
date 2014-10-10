// CppUnit unit test for PSFDataSet

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <iterator>

#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

#include "psf.h"

typedef std::vector<std::string> stringvector_t;
typedef std::vector<std::string>::const_iterator stringvector_iter_t;

void print_stringvector(stringvector_t v) {
    copy(v.begin(), v.end(), std::ostream_iterator<std::string>(std::cout, ","));
    std::cout << std::endl;
}

bool stringvector_set_equal(stringvector_t a, stringvector_t b) {
    std::sort (a.begin(), a.end());
    std::sort (b.begin(), b.end());

    return (a.size() == b.size()) && std::equal(a.begin(), a.end(), b.begin());
}
    
#define STRINGVECTOR_FROM_CHARARRAYS(arg) stringvector_t(arg, arg + sizeof(arg) / sizeof(arg[0]))

class TestPSFDataSet : public CPPUNIT_NS::TestCase {
    CPPUNIT_TEST_SUITE(TestPSFDataSet);	
    
    CPPUNIT_TEST(test_dcop_get_signal_names);
    CPPUNIT_TEST(test_dcop_get_nsweeps);
    CPPUNIT_TEST(test_dcop_get_sweep_npoints);
    CPPUNIT_TEST(test_dcop_get_sweep_values);
    CPPUNIT_TEST(test_dcop_get_sweep_param_names);
    CPPUNIT_TEST(test_tran_get_signal_names);
    CPPUNIT_TEST(test_tran_get_nsweeps);
    CPPUNIT_TEST(test_tran_get_sweep_npoints);
    CPPUNIT_TEST(test_tran_get_sweep_values);
    CPPUNIT_TEST(test_tran_get_sweep_param_names);
  
  CPPUNIT_TEST_EXCEPTION(test_open_psfascii, InvalidFileError);

    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp(void);
    void tearDown(void) {}
    
protected:
  // DCOP data set tests
  void test_dcop_get_signal_names();
  void test_dcop_get_nsweeps();
  void test_dcop_get_sweep_npoints();
  void test_dcop_get_sweep_values();
  void test_dcop_get_sweep_param_names();
  
  // Transient data set tests
  void test_tran_get_signal_names();
  void test_tran_get_nsweeps();
  void test_tran_get_sweep_npoints();
  void test_tran_get_sweep_values();
  void test_tran_get_sweep_param_names();
  
  void test_open_psfascii();
private:	
  std::auto_ptr<PSFDataSet> m_dcop_ds, m_tran_ds;
};

void TestPSFDataSet::setUp() {
    m_dcop_ds = std::auto_ptr<PSFDataSet>(new PSFDataSet("data/dcOp.dc"));
    m_tran_ds = std::auto_ptr<PSFDataSet>(new PSFDataSet("data/tran.tran"));
}

// DCOP data set tests
void TestPSFDataSet::test_dcop_get_signal_names() {
    // test dcop
    const char *expected_names[] = { "vin", "vout" };
    stringvector_t names = m_dcop_ds->get_signal_names();
    CPPUNIT_ASSERT(stringvector_set_equal(names, STRINGVECTOR_FROM_CHARARRAYS(expected_names)));
}

void TestPSFDataSet::test_dcop_get_nsweeps() {
    // test dcop
  CPPUNIT_ASSERT_EQUAL(m_dcop_ds->get_nsweeps(), 0);
}

void TestPSFDataSet::test_dcop_get_sweep_npoints() {
    // test dcOp
  CPPUNIT_ASSERT_EQUAL(m_dcop_ds->get_sweep_npoints(), 0);
}

void TestPSFDataSet::test_dcop_get_sweep_values() {
    // test dcOp
  CPPUNIT_ASSERT_EQUAL(m_dcop_ds->get_sweep_values(), (PSFVector *)NULL);
}

void TestPSFDataSet::test_dcop_get_sweep_param_names() {
    // test dcOp
  CPPUNIT_ASSERT_EQUAL(m_dcop_ds->get_sweep_param_names().size(), (long unsigned int)0);
}


// Transient data set tests
void TestPSFDataSet::test_tran_get_signal_names() {
    // test tran
    const char *expected_names[] = { "vin", "vout" };
    stringvector_t names = m_tran_ds->get_signal_names();
    CPPUNIT_ASSERT(stringvector_set_equal(names, STRINGVECTOR_FROM_CHARARRAYS(expected_names)));
}

void TestPSFDataSet::test_tran_get_nsweeps() {
    // test tran
    CPPUNIT_ASSERT(m_tran_ds->get_nsweeps() == 1);
}

void TestPSFDataSet::test_tran_get_sweep_npoints() {
  // test tran
  CPPUNIT_ASSERT_EQUAL(m_tran_ds->get_sweep_npoints(), 24942);
}

void TestPSFDataSet::test_tran_get_sweep_values() {
  // test tran
  stringvector_t names = m_tran_ds->get_signal_names();
  
  // Get signal vectors
  for(stringvector_iter_t name_iter = names.begin(); name_iter != names.end(); name_iter++) {
    const PSFVector* datavector = m_tran_ds->get_signal_vector("in");
    delete(datavector);
  }
}

void TestPSFDataSet::test_tran_get_sweep_param_names() {
  // test tran
  CPPUNIT_ASSERT_EQUAL(m_tran_ds->get_sweep_param_names()[0], std::string("time"));
}



void TestPSFDataSet::test_open_psfascii() {
    // test open unsupported ascii PSF file
    new PSFDataSet("data/designParamVals.info");
}




CPPUNIT_TEST_SUITE_REGISTRATION(TestPSFDataSet);

int main(int argc, char *argv[]) {
    // Create the event manager and test controller
    CPPUNIT_NS::TestResult controller;

    // Add a listener that colllects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener( &result );        

    // Add a listener that print dots as test run.
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener( &progress );      

    // Add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
    runner.run( controller );

    return result.wasSuccessful() ? 0 : 1;
}
