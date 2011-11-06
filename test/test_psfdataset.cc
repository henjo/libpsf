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
    CPPUNIT_TEST(test_get_signal_names);
    CPPUNIT_TEST(test_get_sweep_npoints);
    CPPUNIT_TEST(test_get_sweep_values);
    CPPUNIT_TEST(test_get_sweep_param_names);
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp(void);
    void tearDown(void) {}
    
protected:
    void test_get_signal_names();
    void test_get_sweep_npoints();
    void test_get_sweep_values();
    void test_get_sweep_param_names();

private:	
    std::auto_ptr<PSFDataSet> m_dcop_ds;
};

void TestPSFDataSet::setUp() {
    m_dcop_ds = std::auto_ptr<PSFDataSet>(new PSFDataSet("data/dcOp.dc"));
}

void TestPSFDataSet::test_get_signal_names() {
    // test dcOp
    const char *expected_names[] = { "vin", "vout" };
    stringvector_t names = m_dcop_ds->get_signal_names();
    CPPUNIT_ASSERT(stringvector_set_equal(names, STRINGVECTOR_FROM_CHARARRAYS(expected_names)));
}

void TestPSFDataSet::test_get_sweep_npoints() {
    // test dcOp
    CPPUNIT_ASSERT(m_dcop_ds->get_sweep_npoints() == 0);
}

void TestPSFDataSet::test_get_sweep_values() {
    // test dcOp
    CPPUNIT_ASSERT(m_dcop_ds->get_sweep_values() == NULL);
}

void TestPSFDataSet::test_get_sweep_param_names() {
    // test dcOp
    CPPUNIT_ASSERT(m_dcop_ds->get_sweep_param_names().size() == 0);
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
