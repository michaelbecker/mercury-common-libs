#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "Calculator.h"
#include "CalculatorDbg.h"


bool DoubleEqual(double a, double b);
void  PrintResults(double *Data, int NumberSignals, void *UserData);
extern bool Verbose;

#define TEST_ITERATIONS 240


static double CorrectResults[TEST_ITERATIONS][7] = 
{
 {    1.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    2.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    3.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    4.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    5.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    6.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    7.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    8.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {    9.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   10.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   11.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   12.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   13.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   14.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   15.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   16.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   17.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   18.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   19.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   20.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   21.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   22.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   23.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   24.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   25.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   26.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   27.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   28.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   29.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   30.000000,    1.000000,   60.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   31.000000,    2.000000,   61.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   32.000000,    3.000000,   62.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   33.000000,    4.000000,   63.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   34.000000,    5.000000,   64.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   35.000000,    6.000000,   65.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   36.000000,    7.000000,   66.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   37.000000,    8.000000,   67.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   38.000000,    9.000000,   68.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   39.000000,   10.000000,   69.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   40.000000,   11.000000,   70.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   41.000000,   12.000000,   71.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   42.000000,   13.000000,   72.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   43.000000,   14.000000,   73.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   44.000000,   15.000000,   74.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   45.000000,   16.000000,   75.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   46.000000,   17.000000,   76.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   47.000000,   18.000000,   77.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   48.000000,   19.000000,   78.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   49.000000,   20.000000,   79.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   50.000000,   21.000000,   80.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   51.000000,   22.000000,   81.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   52.000000,   23.000000,   82.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   53.000000,   24.000000,   83.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   54.000000,   25.000000,   84.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   55.000000,   26.000000,   85.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   56.000000,   27.000000,   86.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   57.000000,   28.000000,   87.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   58.000000,   29.000000,   88.000000,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   59.000000,   30.000000,   89.000000,    1.000000,   60.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   60.000000,   31.000000,   90.000000,    2.000000,   61.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   61.000000,   32.000000,   91.000000,    3.000000,   62.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   62.000000,   33.000000,   92.000000,    4.000000,   63.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   63.000000,   34.000000,   93.000000,    5.000000,   64.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   64.000000,   35.000000,   94.000000,    6.000000,   65.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   65.000000,   36.000000,   95.000000,    7.000000,   66.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   66.000000,   37.000000,   96.000000,    8.000000,   67.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   67.000000,   38.000000,   97.000000,    9.000000,   68.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   68.000000,   39.000000,   98.000000,   10.000000,   69.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   69.000000,   40.000000,   99.000000,   11.000000,   70.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   70.000000,   41.000000,  100.000000,   12.000000,   71.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   71.000000,   42.000000,  101.000000,   13.000000,   72.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   72.000000,   43.000000,  102.000000,   14.000000,   73.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   73.000000,   44.000000,  103.000000,   15.000000,   74.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   74.000000,   45.000000,  104.000000,   16.000000,   75.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   75.000000,   46.000000,  105.000000,   17.000000,   76.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   76.000000,   47.000000,  106.000000,   18.000000,   77.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   77.000000,   48.000000,  107.000000,   19.000000,   78.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   78.000000,   49.000000,  108.000000,   20.000000,   79.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   79.000000,   50.000000,  109.000000,   21.000000,   80.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   80.000000,   51.000000,  110.000000,   22.000000,   81.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   81.000000,   52.000000,  111.000000,   23.000000,   82.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   82.000000,   53.000000,  112.000000,   24.000000,   83.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   83.000000,   54.000000,  113.000000,   25.000000,   84.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   84.000000,   55.000000,  114.000000,   26.000000,   85.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   85.000000,   56.000000,  115.000000,   27.000000,   86.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   86.000000,   57.000000,  116.000000,   28.000000,   87.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   87.000000,   58.000000,  117.000000,   29.000000,   88.000000,  SM_DBL_DNE,  SM_DBL_DNE,  },
 {   88.000000,   59.000000,  118.000000,   30.000000,   89.000000,    1.000000,   60.000000,  },
 {   89.000000,   60.000000,  119.000000,   31.000000,   90.000000,    2.000000,   61.000000,  },
 {   90.000000,   61.000000,  120.000000,   32.000000,   91.000000,    3.000000,   62.000000,  },
 {   91.000000,   62.000000,  121.000000,   33.000000,   92.000000,    4.000000,   63.000000,  },
 {   92.000000,   63.000000,  122.000000,   34.000000,   93.000000,    5.000000,   64.000000,  },
 {   93.000000,   64.000000,  123.000000,   35.000000,   94.000000,    6.000000,   65.000000,  },
 {   94.000000,   65.000000,  124.000000,   36.000000,   95.000000,    7.000000,   66.000000,  },
 {   95.000000,   66.000000,  125.000000,   37.000000,   96.000000,    8.000000,   67.000000,  },
 {   96.000000,   67.000000,  126.000000,   38.000000,   97.000000,    9.000000,   68.000000,  },
 {   97.000000,   68.000000,  127.000000,   39.000000,   98.000000,   10.000000,   69.000000,  },
 {   98.000000,   69.000000,  128.000000,   40.000000,   99.000000,   11.000000,   70.000000,  },
 {   99.000000,   70.000000,  129.000000,   41.000000,  100.000000,   12.000000,   71.000000,  },
 {  100.000000,   71.000000,  130.000000,   42.000000,  101.000000,   13.000000,   72.000000,  },
 {  101.000000,   72.000000,  131.000000,   43.000000,  102.000000,   14.000000,   73.000000,  },
 {  102.000000,   73.000000,  132.000000,   44.000000,  103.000000,   15.000000,   74.000000,  },
 {  103.000000,   74.000000,  133.000000,   45.000000,  104.000000,   16.000000,   75.000000,  },
 {  104.000000,   75.000000,  134.000000,   46.000000,  105.000000,   17.000000,   76.000000,  },
 {  105.000000,   76.000000,  135.000000,   47.000000,  106.000000,   18.000000,   77.000000,  },
 {  106.000000,   77.000000,  136.000000,   48.000000,  107.000000,   19.000000,   78.000000,  },
 {  107.000000,   78.000000,  137.000000,   49.000000,  108.000000,   20.000000,   79.000000,  },
 {  108.000000,   79.000000,  138.000000,   50.000000,  109.000000,   21.000000,   80.000000,  },
 {  109.000000,   80.000000,  139.000000,   51.000000,  110.000000,   22.000000,   81.000000,  },
 {  110.000000,   81.000000,  140.000000,   52.000000,  111.000000,   23.000000,   82.000000,  },
 {  111.000000,   82.000000,  141.000000,   53.000000,  112.000000,   24.000000,   83.000000,  },
 {  112.000000,   83.000000,  142.000000,   54.000000,  113.000000,   25.000000,   84.000000,  },
 {  113.000000,   84.000000,  143.000000,   55.000000,  114.000000,   26.000000,   85.000000,  },
 {  114.000000,   85.000000,  144.000000,   56.000000,  115.000000,   27.000000,   86.000000,  },
 {  115.000000,   86.000000,  145.000000,   57.000000,  116.000000,   28.000000,   87.000000,  },
 {  116.000000,   87.000000,  146.000000,   58.000000,  117.000000,   29.000000,   88.000000,  },
 {  117.000000,   88.000000,  147.000000,   59.000000,  118.000000,   30.000000,   89.000000,  },
 {  118.000000,   89.000000,  148.000000,   60.000000,  119.000000,   31.000000,   90.000000,  },
 {  119.000000,   90.000000,  149.000000,   61.000000,  120.000000,   32.000000,   91.000000,  },
 {  120.000000,   91.000000,  150.000000,   62.000000,  121.000000,   33.000000,   92.000000,  },
 {  121.000000,   92.000000,  151.000000,   63.000000,  122.000000,   34.000000,   93.000000,  },
 {  122.000000,   93.000000,  152.000000,   64.000000,  123.000000,   35.000000,   94.000000,  },
 {  123.000000,   94.000000,  153.000000,   65.000000,  124.000000,   36.000000,   95.000000,  },
 {  124.000000,   95.000000,  154.000000,   66.000000,  125.000000,   37.000000,   96.000000,  },
 {  125.000000,   96.000000,  155.000000,   67.000000,  126.000000,   38.000000,   97.000000,  },
 {  126.000000,   97.000000,  156.000000,   68.000000,  127.000000,   39.000000,   98.000000,  },
 {  127.000000,   98.000000,  157.000000,   69.000000,  128.000000,   40.000000,   99.000000,  },
 {  128.000000,   99.000000,  158.000000,   70.000000,  129.000000,   41.000000,  100.000000,  },
 {  129.000000,  100.000000,  159.000000,   71.000000,  130.000000,   42.000000,  101.000000,  },
 {  130.000000,  101.000000,  160.000000,   72.000000,  131.000000,   43.000000,  102.000000,  },
 {  131.000000,  102.000000,  161.000000,   73.000000,  132.000000,   44.000000,  103.000000,  },
 {  132.000000,  103.000000,  162.000000,   74.000000,  133.000000,   45.000000,  104.000000,  },
 {  133.000000,  104.000000,  163.000000,   75.000000,  134.000000,   46.000000,  105.000000,  },
 {  134.000000,  105.000000,  164.000000,   76.000000,  135.000000,   47.000000,  106.000000,  },
 {  135.000000,  106.000000,  165.000000,   77.000000,  136.000000,   48.000000,  107.000000,  },
 {  136.000000,  107.000000,  166.000000,   78.000000,  137.000000,   49.000000,  108.000000,  },
 {  137.000000,  108.000000,  167.000000,   79.000000,  138.000000,   50.000000,  109.000000,  },
 {  138.000000,  109.000000,  168.000000,   80.000000,  139.000000,   51.000000,  110.000000,  },
 {  139.000000,  110.000000,  169.000000,   81.000000,  140.000000,   52.000000,  111.000000,  },
 {  140.000000,  111.000000,  170.000000,   82.000000,  141.000000,   53.000000,  112.000000,  },
 {  141.000000,  112.000000,  171.000000,   83.000000,  142.000000,   54.000000,  113.000000,  },
 {  142.000000,  113.000000,  172.000000,   84.000000,  143.000000,   55.000000,  114.000000,  },
 {  143.000000,  114.000000,  173.000000,   85.000000,  144.000000,   56.000000,  115.000000,  },
 {  144.000000,  115.000000,  174.000000,   86.000000,  145.000000,   57.000000,  116.000000,  },
 {  145.000000,  116.000000,  175.000000,   87.000000,  146.000000,   58.000000,  117.000000,  },
 {  146.000000,  117.000000,  176.000000,   88.000000,  147.000000,   59.000000,  118.000000,  },
 {  147.000000,  118.000000,  177.000000,   89.000000,  148.000000,   60.000000,  119.000000,  },
 {  148.000000,  119.000000,  178.000000,   90.000000,  149.000000,   61.000000,  120.000000,  },
 {  149.000000,  120.000000,  179.000000,   91.000000,  150.000000,   62.000000,  121.000000,  },
 {  150.000000,  121.000000,  180.000000,   92.000000,  151.000000,   63.000000,  122.000000,  },

};

static int CurrentResults = 0;
static bool TestPass = true;

static void  
VerifyResults(double *Data, int NumberSignals, void *UserData)
{
    (void)UserData;

    for (int i = 0; i < NumberSignals; i++){
        if (!DoubleEqual(Data[i], CorrectResults[CurrentResults][i])){
            if (Verbose){
                printf("FAILED VERIFICATION!\n");
            }
            TestPass = false;
        }
    }

    CurrentResults++;
}



static double  fcnX(void *UserData)
{
    (void)UserData;

    static double x = 0.0;
    x = x + 1.0;
    return x;
}


const int N = 60;


static double  FcnMinIndex(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    int start = -(N / 2) + 1;
    double sum = 0.0;

    sum = SmGetInputResult(cs, start, 0);

    return sum;
}

static double  FcnMaxIndex(SIGNAL_HANDLE cs, void *UserData)
{
    (void)UserData;

    int end = (N / 2);
    double sum = 0.0;

    sum = SmGetInputResult(cs, end, 0);

    return sum;
}


enum SignalIds {
    IdInvalid,
    IdFcnX,
    IdMinIndex1,
    IdMaxIndex1,
    IdMinIndex2,
    IdMaxIndex2,
    IdMinIndex3,
    IdMaxIndex3,
};


bool UnitTestMdscIndicies(void)
{
    int i;

    printf("Executing UnitTestMdscIndicies\n");

    SmAddRawSignal(IdFcnX, "fcnX", fcnX, NULL, NULL);

    INPUT_SIGNAL MinIndex1Input[] = {
        {IdFcnX, ((N/2) - 1), (N/2)},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL MinIndex2Input[] = {
        {IdMinIndex1, ((N/2) - 1), (N/2)},
        INPUT_SIGNAL_END
    };

    INPUT_SIGNAL MinIndex3Input[] = {
        {IdMinIndex2, ((N/2) - 1), (N/2)},
        INPUT_SIGNAL_END
    };
    
    SmAddCalcSignal(IdMinIndex1, "MinIndex1",  FcnMinIndex,  NULL, MinIndex1Input, NULL);
    SmAddCalcSignal(IdMaxIndex1, "MaxIndex1",  FcnMaxIndex,  NULL, MinIndex1Input, NULL);

    SmAddCalcSignal(IdMinIndex2, "MinIndex2",  FcnMinIndex,  NULL, MinIndex2Input, NULL);
    SmAddCalcSignal(IdMaxIndex2, "MaxIndex2",  FcnMaxIndex,  NULL, MinIndex2Input, NULL);
    
    SmAddCalcSignal(IdMinIndex3, "MinIndex3",  FcnMinIndex,  NULL, MinIndex3Input, NULL);
    SmAddCalcSignal(IdMaxIndex3, "MaxIndex3",  FcnMaxIndex,  NULL, MinIndex3Input, NULL);


    SmInitCalculator();

    if (Verbose)
        SmRegisterDataReadyCallback(PrintResults, NULL);

    SmRegisterDataReadyCallback(VerifyResults, NULL);

    if (Verbose)
        SmPrintHeaders();

    SmInitRunCalculationsAsync(50);

    for (i = 0; i<TEST_ITERATIONS; i++){
        SmRunCalculationsAsync(NULL);
    }

    //
    //  Need to sync with the worker thread.
    //
    if (CurrentResults < 150){
        sleep(1);
    }

    return TestPass;
}


