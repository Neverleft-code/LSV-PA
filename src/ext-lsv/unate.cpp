#include "base/main/main.h"
#include "base/main/mainInt.h"

#include <iostream>
#include <list>
#include <vector>
using namespace std;
namespace
{
static int Lsv_CommandPrintUnates(Abc_Frame_t* pAbc, int argc, char** argv);

void Lsv_NtkPrintUnates(Abc_Ntk_t* pNtk) {
  Abc_Obj_t* pObj;
  //Abc_Obj_t* pCube;
  int i;
  Abc_NtkForEachNode(pNtk, pObj, i) {
    //printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    Abc_Obj_t* pFanin;
    int j;
    //int k;
    //int value;
    char * pSop = (char *)pObj->pData;
    if(!Abc_SopIsConst1( pSop )&&!Abc_SopIsConst0( pSop ))
      printf("node %s:\n",Abc_ObjName(pObj));
    int varnum = Abc_SopGetVarNum( pSop );
    int cubenum = Abc_SopGetCubeNum( pSop );
    char* recfin[varnum];
    int recfid[varnum];
    int recind = 0;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      //printf("  Fanin-%d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin),
      //       Abc_ObjName(pFanin));
      recfin[recind] = Abc_ObjName(pFanin);
      recfid[recind] = Abc_ObjId(pFanin);
      recind++;
    }
    int m, n;
    int flag0[varnum];
    int flag1[varnum];
    int unateness[varnum];
    for(int k=0;k<varnum;k++){
      flag0[k] = 0;
      flag1[k] = 0;
    }
    for(m=0;m<varnum;m++)
      for(n=m;n<(varnum+3)*cubenum;n=n+varnum+3){
        if(pSop[n]=='0')  flag0[m] = 1;
        else if(pSop[n]=='1') flag1[m] = 1;
      }
    if(Abc_SopGetPhase(pSop)==1){
      for(int k=0;k<varnum;k++){
        if((flag0[k]==1)&&(flag1[k]==0))  unateness[k] = 0;
        else if((flag0[k]==0)&&(flag1[k]==1))  unateness[k] = 1;
        else if((flag0[k]==1)&&(flag1[k]==1))  unateness[k] = 2;
        else  unateness[k] = 3;
      }
    }
    else if(Abc_SopGetPhase(pSop)==0){
      for(int k=0;k<varnum;k++){
        if((flag0[k]==1)&&(flag1[k]==0))  unateness[k] = 1;
        else if((flag0[k]==0)&&(flag1[k]==1))  unateness[k] = 0;
        else if((flag0[k]==1)&&(flag1[k]==1))  unateness[k] = 2;
        else  unateness[k] = 3;
      }
    }
    //for(int k=0;k<15;k++)
    //printf("???????%c\n",pSop[0]);
    vector <int> posid;
    vector <int> negid;
    vector <int> biid;
    vector <char*> poslist;
    vector <char*> neglist;
    vector <char*> bilist;
    for(int k=0;k<varnum;k++){
      if(unateness[k]==0) {neglist.push_back(recfin[k]); negid.push_back(recfid[k]);}
      else if(unateness[k]==1) {poslist.push_back(recfin[k]); posid.push_back(recfid[k]);}
      else if(unateness[k]==2) {bilist.push_back(recfin[k]); biid.push_back(recfid[k]);}
    }
    int key, sortind;
    char* charkey;
    for(int k=1;k<negid.size();k++){
      key = negid.at(k);
      charkey = neglist.at(k);
      sortind = k-1;
      while(sortind>=0&&negid.at(sortind)>key){
        negid.at(sortind+1) = negid.at(sortind);
        neglist.at(sortind+1) = neglist.at(sortind);
        sortind--;
      }
      negid.at(sortind+1) = key;
      neglist.at(sortind+1) = charkey;
    }
    for(int k=1;k<posid.size();k++){
      key = posid.at(k);
      charkey = poslist.at(k);
      sortind = k-1;
      while(sortind>=0&&posid.at(sortind)>key){
        posid.at(sortind+1) = posid.at(sortind);
        poslist.at(sortind+1) = poslist.at(sortind);
        sortind--;
      }
      posid.at(sortind+1) = key;
      poslist.at(sortind+1) = charkey;
    }
    for(int k=1;k<biid.size();k++){
      key = biid.at(k);
      charkey = bilist.at(k);
      sortind = k-1;
      while(sortind>=0&&biid.at(sortind)>key){
        biid.at(sortind+1) = biid.at(sortind);
        bilist.at(sortind+1) = bilist.at(sortind);
        sortind--;
      }
      biid.at(sortind+1) = key;
      bilist.at(sortind+1) = charkey;
    }
    if(!poslist.empty()) cout<<"+unate inputs: "<<poslist.at(0);
    for (int k=1; k<poslist.size(); k++)
      cout << "," << poslist.at(k);
    if(!poslist.empty()) cout<<endl;
    if(!neglist.empty()) cout<<"-unate inputs: "<<neglist.at(0);
    for (int k=1; k<neglist.size(); k++)
      cout << "," << neglist.at(k);
    if(!neglist.empty()) cout<<endl;
    if(!bilist.empty()) cout<<"binate inputs: "<<bilist.at(0);
    for (int k=1; k<bilist.size(); k++)
      cout << "," << bilist.at(k);
    if(!bilist.empty()) cout<<endl;
    //if (Abc_NtkHasSop(pNtk)) {
      //printf("The SOP of this node:\n%s", pSop);
      //printf("Each node cube num is %d\n",Abc_SopGetCubeNum( pSop ));
      //printf("literal num is %d\n",Abc_SopGetLitNum( pSop ));
      //printf("variable num is %d\n",Abc_SopGetVarNum( pSop ));
      //printf("Phase is %d\n",Abc_SopGetPhase(pSop));
    //}
    //printf("cube num = %d\n",Abc_NtkGetCubeNum(pNtk));
    /*Abc_CubeForEachVar( pCube, value, k ){
      printf("%d ",value);
    }*/
  }
  //std::cout<<ABC_FUNC_SOP<<' '<<ABC_NTK_NETLIST<<' '<<ABC_NTK_LOGIC<<' '<<ABC_FUNC_BDD<<' '<<pNtk->ntkFunc<<' '<<pNtk->ntkType<<std::endl;
}

int Lsv_CommandPrintUnates(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Lsv_NtkPrintUnates(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_nodes [-h]\n");
  Abc_Print(-2, "\t        prints the nodes in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}

// Very simple ABC command: prints a greeting and its command line argumentss
/*int Hello_Command( Abc_Frame_t * pAbc, int argc, char ** argv )
{
    std::cout << "Hello world!" << std::endl;

    for(int i=0; i<argc; i++)
    {
        std::cout << "  argv[" << i << "]: " << argv[i] << std::endl;
    }

    return 0;
}*/

// called during ABC startup
void init(Abc_Frame_t* pAbc)
{
    Cmd_CommandAdd( pAbc, "Unate", "lsv_print_sopunate", Lsv_CommandPrintUnates, 0);
}

// called during ABC termination
void destroy(Abc_Frame_t* pAbc)
{
}

// this object should not be modified after the call to Abc_FrameAddInitializer
Abc_FrameInitializer_t frame_initializer = { init, destroy };

// register the initializer a constructor of a global object
// called before main (and ABC startup)
struct registrar
{
    registrar() 
    {
        Abc_FrameAddInitializer(&frame_initializer);
    }
} hello_registrar;

} // unnamed namespace

