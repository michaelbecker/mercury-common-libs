/**
 *  ONLY INCLUDE THIS FILE IN MAIN.CPP!!!!
 *
 *  THIS CONTAINS THE CHANGING LIST OF UNIT TESTS.
 */

/**
 *  List of unit test functions.
 */
bool UnitTestRaw(void);
bool UnitTestRawRealTime(void);
bool UnitTestFwd(void);
bool UnitTestBkwd(void);
bool UnitTestFwdBkwdIndependent(void);
bool UnitTestFwdBkwdWindowAvg(void);
bool UnitTestMdscIndicies(void);
bool UnitTestSort1(void);
bool UnitTestSort2(void);
bool UnitTestInputManager1(void);
bool UnitTestInputManager2(void);
bool UnitTestSelectDataOut1(void);
bool UnitTestSelectDataOut2(void);
bool UnitTestEarlyReset(void);


/**
 *  Table of unit test functions and corresponding 
 *  command line strings.
 */
struct TEST_CASE TestCases[] = 
{
    { "UnitTestRaw", UnitTestRaw, true },
    { "UnitTestRawRealTime", UnitTestRawRealTime, true },
    { "UnitTestFwd", UnitTestFwd, true  },
    { "UnitTestBkwd", UnitTestBkwd, true },
    { "UnitTestFwdBkwdIndependent", UnitTestFwdBkwdIndependent, true },
    { "UnitTestFwdBkwdWindowAvg", UnitTestFwdBkwdWindowAvg, true },
    { "UnitTestMdscIndicies", UnitTestMdscIndicies, true },
    { "UnitTestSort1", UnitTestSort1, true },
    { "UnitTestSort2", UnitTestSort2, true },
    { "UnitTestSelectDataOut1", UnitTestSelectDataOut1, true },
    { "UnitTestSelectDataOut2", UnitTestSelectDataOut2, true },
    { "UnitTestEarlyReset", UnitTestEarlyReset, true },

//
//  Not porting these classes.  This was a Discovery specific way to drive the 
//  Calculator, and Mercury does not have a Generic Note System.
//
//    { _T("UnitTestInputManager1"), UnitTestInputManager1, false },
//    { _T("UnitTestInputManager2"), UnitTestInputManager2, false },

    {NULL, NULL, false}
};
