#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "proof/fraig/fraig.h"
#include "opt/fxu/fxu.h"
#include "opt/fxch/Fxch.h"
#include "opt/cut/cut.h"
#include "map/fpga/fpga.h"
#include "map/if/if.h"
#include "opt/sim/sim.h"
#include "opt/res/res.h"
#include "opt/lpk/lpk.h"
#include "aig/gia/giaAig.h"
#include "opt/dar/dar.h"
#include "opt/mfs/mfs.h"
#include "proof/fra/fra.h"
#include "aig/saig/saig.h"
#include "proof/int/int.h"
#include "proof/dch/dch.h"
#include "proof/ssw/ssw.h"
#include "opt/cgt/cgt.h"
#include "bool/kit/kit.h"
#include "map/amap/amap.h"
#include "opt/ret/retInt.h"
#include "sat/xsat/xsat.h"
#include "sat/satoko/satoko.h"
#include "sat/cnf/cnf.h"
#include "proof/cec/cec.h"
#include "proof/acec/acec.h"
#include "proof/pdr/pdr.h"
#include "misc/tim/tim.h"
#include "bdd/llb/llb.h"
#include "bdd/bbr/bbr.h"
#include "map/cov/cov.h"
#include "base/cmd/cmd.h"
#include "proof/abs/abs.h"
#include "sat/bmc/bmc.h"
#include "proof/ssc/ssc.h"
#include "opt/sfm/sfm.h"
#include "opt/sbd/sbd.h"
#include "bool/rpo/rpo.h"
#include "map/mpm/mpm.h"
#include "opt/fret/fretime.h"
#include "opt/nwk/nwkMerge.h"
#include "base/acb/acbPar.h"
#include "aig/aig/aig.h"
#include "sat/cnf/cnf.h"
#include "sat/bsat/satSolver.h"
#include <vector>


static int Abc_CommandRunEco_test(Abc_Frame_t* pAbc, int argc, char** argv);


void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "Synthesis", "eco_modify_verilog", Abc_CommandRunEco_test, 0);
}

void destroy(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer = {init, destroy};


struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;

extern "C"
{
    void Acb_NtkRunEco( char * pFileNames[4], int nTimeout, int fCheck, int fRandom, int fInputs, int fVerbose, int fVeryVerbose );
}



int Abc_CommandRunEco_test(Abc_Frame_t* pAbc, int argc, char** argv) {
  //printf("ecoooooo");
  char * pFileNames[4] = {NULL};
  int c, nTimeout = 0, fCheck = 0, fRandom = 0, fInputs = 0, fVerbose = 0, fVeryVerbose = 0;
  Extra_UtilGetoptReset();
  while ( ( c = Extra_UtilGetopt( argc, argv, "Tcrivwh" ) ) != EOF )
  {
      switch ( c )
      {
      case 'T':
          if ( globalUtilOptind >= argc )
          {
              Abc_Print( -1, "Command line switch \"-T\" should be followed by an integer.\n" );
              goto usage;
          }
          nTimeout = atoi(argv[globalUtilOptind]);
          globalUtilOptind++;
          if ( nTimeout < 0 )
              goto usage;
          break;
      case 'c':
          fCheck ^= 1;
          break;
      case 'r':
          fRandom ^= 1;
          break;
      case 'i':
          fInputs ^= 1;
          break;
      case 'v':
          fVerbose ^= 1;
          break;
      case 'w':
          fVeryVerbose ^= 1;
          break;
      case 'h':
          goto usage;
      default:
          goto usage;
      }
  }
  if ( argc - globalUtilOptind < 2 || argc - globalUtilOptind > 3 )
  {
      Abc_Print( 1, "Expecting three file names on the command line.\n" );
      goto usage;
  }
  for ( c = 0; c < argc - globalUtilOptind; c++ )
  {
      FILE * pFile = fopen( argv[globalUtilOptind+c], "rb" );
      if ( pFile == NULL )
      {
          printf( "Cannot open input file \"%s\".\n", argv[globalUtilOptind+c] );
          return 0;
      }
      else
          fclose( pFile );
      pFileNames[c] = argv[globalUtilOptind+c];
  }
  Acb_NtkRunEco( pFileNames, nTimeout, fCheck, fRandom, fInputs, fVerbose, fVeryVerbose );
  return 0;
usage:
  Abc_Print( -2, "usage: runeco [-T num] [-crivwh] <implementation> <specification> <weights>\n" );
  Abc_Print( -2, "\t         performs computation of patch functions during ECO,\n" );
  Abc_Print( -2, "\t         as described in the following paper: A. Q. Dao et al\n" );
  Abc_Print( -2, "\t         \"Efficient computation of ECO patch functions\", Proc. DAC\'18\n" );
  Abc_Print( -2, "\t         https://people.eecs.berkeley.edu/~alanmi/publications/2018/dac18_eco.pdf\n" );
  Abc_Print( -2, "\t         (currently only applicable to benchmarks from 2017 ICCAD CAD competition\n" );
  Abc_Print( -2, "\t         http://cad-contest-2017.el.cycu.edu.tw/Problem_A/default.html as follows:\n" );
  Abc_Print( -2, "\t         \"runeco unit1/F.v unit1/G.v unit1/weight.txt; cec -n out.v unit1/G.v\")\n" );
  Abc_Print( -2, "\t-T num : the timeout in seconds [default = %d]\n", nTimeout );
  Abc_Print( -2, "\t-c     : toggle checking that the problem has a solution [default = %s]\n", fCheck? "yes": "no" );
  Abc_Print( -2, "\t-r     : toggle using random permutation of support variables [default = %s]\n", fRandom? "yes": "no" );
  Abc_Print( -2, "\t-i     : toggle using primary inputs as support variables [default = %s]\n", fInputs? "yes": "no" );
  Abc_Print( -2, "\t-v     : toggle printing verbose information [default = %s]\n", fVerbose? "yes": "no" );
  Abc_Print( -2, "\t-w     : toggle printing more verbose information [default = %s]\n", fVeryVerbose? "yes": "no" );
  Abc_Print( -2, "\t-h     : print the command usage\n");
  return 1;
}