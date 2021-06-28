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
#include <set>
#include <map>

#include <iostream>
#include <fstream>

using namespace std;


static int Abc_CommandRunEco_test(Abc_Frame_t* pAbc, int argc, char** argv);
void insertTarget(string inFName, string oFName);


void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "Synthesis", "eco_modify_verilog", Abc_CommandRunEco_test, 0);
}

void destroy(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer = {init, destroy};


struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;


int Abc_CommandRunEco_test(Abc_Frame_t* pAbc, int argc, char** argv) {
  char * pFileNames[4] = {NULL};
  int c;
  map<string, string> wiretype;
  std::vector<string> gatelist;

  Extra_UtilGetoptReset();
  //printf(argc);
  ofstream nFile("wirename");
  ofstream gFile("gatename");
  ofstream r1File("F.v");
  ofstream r2File("G.v");
  if ( argc - 1 < 2 || argc - 1 > 3 )
  {
      Abc_Print( 1, "Expecting three file names on the command line.\n" );
  }
  for ( c = 0; c < argc - 1; c++ )
  {
      ifstream pFile(argv[c+1], ios::in);

      if ( !pFile )
      {
          printf( "Cannot open input file \"%s\".\n", argv[c+1] );
          return 0;
      }
      else
      {
          //parse wire
          string temp;
          std::set<string>    wireset_tmp;
          while(getline(pFile, temp))
          {
              //cout  << temp.substr(0, 4) << endl;
              if (temp.substr(0, 5) == "input")
              {
                string wirename = "";
                for (int i = 0; i < temp.size(); ++i)
                {
                    if (temp[i] == ' ') wirename = "";
                    else if (temp[i] == ';' or temp[i] == ',') {
                      wireset_tmp.insert(wirename);
                      wiretype[wirename] = "input";
                    }
                    else wirename = wirename + temp[i];
                }
              }

              else if (temp.substr(0, 6) == "output")
              {
                string wirename = "";
                for (int i = 0; i < temp.size(); ++i)
                {
                    if (temp[i] == ' ') wirename = "";
                    else if (temp[i] == ';' or temp[i] == ','){
                      wireset_tmp.insert(wirename);
                      wiretype[wirename] = "output";
                    }
                    else wirename = wirename + temp[i];
                }
              }

              else if (temp.substr(0, 4) == "wire")
              {
                  string wirename = "";
                  for (int i = 0; i < temp.size(); ++i)
                  {
                      if (temp[i] == ' ') wirename = "";
                      else if (temp[i] == ';' or temp[i] == ',') {
                        wireset_tmp.insert(wirename);
                        wiretype[wirename] = "wire";
                      }
                      else wirename = wirename + temp[i];
                  }
              }
              else if (temp.substr(0, 6) != "module" and temp.substr(0, 9) != "endmodule")
              {
                  int gatename = 0;
                  string tempname = "";
                  for (int i = 0; i < temp.size(); ++i)
                  {
                      if (temp[i] == ' ') {
                        gatename = i;
                        tempname = "";
                      }
                      else if (temp[i] == '(') {
                        gatelist.push_back(temp.substr(0, gatename));
                        gatelist.push_back(tempname);
                        tempname = "";
                      }
                      else if (temp[i] == ')' or temp[i] == ','){
                        wireset_tmp.insert(tempname);
                        if (wiretype[tempname] == "")
                        {
                          wiretype[tempname] = "wire";
                        }
                        tempname = "";
                      }
                      else tempname = tempname + temp[i];
                  }
              }
          }
          pFile.close();

          ifstream pFile(argv[c+1], ios::in);
          if (c == 0)
          {
            //cout << "c==0" << endl;
            while(getline(pFile, temp))
            {
              if (temp.substr(0, 6) == "module") {
                //cout << temp << endl;
                r1File << temp << endl;
                for (const auto &s : wireset_tmp) {
                  nFile  << s << endl;
                  r1File << wiretype[s] << ' ' << s << ';' << endl;
                }
              }
              else if (temp.substr(0, 4) != "wire" and temp.substr(0, 5) != "input" and temp.substr(0, 6) != "output")
              {
                string tempname = "";
                for (int i = 0; i < temp.size(); ++i)
                {
                    tempname = tempname + temp[i];
                    if (temp[i] == ' '){
                      r1File << tempname;
                      tempname = "";
                    }
                    else if (temp[i] == ';'){
                      r1File << tempname << "\n";
                      tempname = "";
                    }
                    else if (temp[i] == '('){ 
                      tempname = "";
                      r1File << '(' << ' ';
                    }
                }
              }
            }
            r1File << "endmodule" << endl;
          }
          else if (c == 1)
          {
            //cout << "c==0" << endl;
            while(getline(pFile, temp))
            {
              if (temp.substr(0, 6) == "module") {
                //cout << temp << endl;
                r2File << temp << endl;
                for (const auto &s : wireset_tmp) {
                  //nFile  << s << endl;
                  r2File << wiretype[s] << ' ' << s << ';' << endl;
                }
              }
              else if (temp.substr(0, 4) != "wire" and temp.substr(0, 5) != "input" and temp.substr(0, 6) != "output")
              {
                string tempname = "";
                for (int i = 0; i < temp.size(); ++i)
                {
                    tempname = tempname + temp[i];
                    if (temp[i] == ' '){
                      r2File << tempname;
                      tempname = "";
                    }
                    else if (temp[i] == ';'){
                      r2File << tempname << "\n";
                      tempname = "";
                    }
                    else if (temp[i] == '('){ 
                      tempname = "";
                      r2File << '(' << ' ';
                    }
                }
              }
            }
            r2File << "endmodule" << endl;
          }
          pFile.close();
      }
  }
  //nFile << "wirelist" << endl;
  //cout << wirelist.size() << endl;
  r1File.close();
  r2File.close();
  nFile.close();

  for (int i = 0; i < gatelist.size()/2; ++i)
  {
    gFile << gatelist[2*i] << ' ' << gatelist[2*i+1] << endl;
  }
  gFile.close();
  pFileNames[c] = argv[c+1];
  //to do

  //fwrite( wire1 /n wire2 /n ..... ,nFile)
  //fclose( nFile );
  //fclose( gFile );


  //////////// insert target ////////////
  cout << "starts to insert target" << endl;
  insertTarget("F.v","F_t.v");



  return 0;
}

vector<string>  split(const string& str,const string& delim) {
    vector<string> res;
    if("" == str) return  res;
    
    string strs = str + delim;
    size_t pos;
    size_t size = strs.size();
 
    for (int i = 0; i < size; ++i) {
        pos = strs.find(delim, i);
        if( pos < size) {
            string s = strs.substr(i, pos - i);
            res.push_back(s);
            i = pos + delim.size() - 1;
        }
        
    }
    return res; 
}


string& trim(string &s, string end=" ") 
{
    if (s.empty()) 
    {
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(end) + 1);
    return s;
}

void insertTarget(string inFName, string oFName="F.v")
{
    fstream fin, fout;
    string buf;
    
    vector<string> allWire;
    fin.open("wirename",ios::in);
    while(!fin.eof())
    {
        fin >> buf;
        allWire.push_back(buf);
    }
    fin.close();

    set<string> allTarget;
    fin.open("wirename",ios::in);
    while(!fin.eof())
    {
        fin >> buf;
        allTarget.insert(buf);
    }
    fin.close();

    // cout << "allWire contains:\n";
    // for (auto& s : allWire)
    //     cout << s << ',';
    // cout << endl << allWire.size() << endl;

    fin.open(inFName,ios::in);
    fout.open(oFName,ios::out);
    string line;
    vector<string>  splited;
    set<string> oNames, iNames;
    set<string> declaredWire;
    while(getline(fin,line))
    {
        cout << line << endl;
        if(line.find("module")==0 || line.find("endmodule")==0)
        {
            fout << line << endl;
            continue;
        }
        if(line.find("wire")==0)
            continue;
        if(line.find("output")==0)
        {
            splited = split(line," ");
            for(int i=1;i<splited.size();i++)
            {
                trim(splited[i],";\n");
                trim(splited[i],",");
                oNames.insert(splited[i]);
            }
            fout << line << endl;
        }
        else if(line.find("input")==0)
        {
            splited = split(line," ");
            for(int i=1;i<splited.size();i++)
            {
                trim(splited[i],";\n");
                trim(splited[i],",");
                iNames.insert(splited[i]);
            }
            fout << line << endl;
        }
        else
        {
            string gate = line.substr(0,line.find(" "));
            //cout << "gate: " << gate << endl;
            size_t st=line.find("("), end=line.find(")");
            string wireStr = line.substr(st+1,end-st-1);
            trim(wireStr);
            cout << "wireStr: " << wireStr << endl;
            splited = split(wireStr," ");
            for(int i=0;i<splited.size();i++)//for each pin of a gate
            {
                trim(splited[i],",");
                if(splited[i]=="")
                    continue;
                cout << i << ": " << splited[i] << endl;
                if(i==0)//gate output
                {
                    //the pin is PO and is target
                    if(oNames.find(splited[i])!=oNames.end() && allTarget.find(splited[i])!=allTarget.end())
                    {
                        cout << "outputs: " << splited[i] << endl;
                        fout << "wire t_" << splited[i] << ", u_" << splited[i] << ";\n";
                        fout << "buf (" << splited[i] << ",t_" << splited[i] << ");\n";
                        splited[i] = "u_"+splited[i];
                    }
                    //the pin is internal wire and is target
                    else if(allTarget.find(splited[i])!=allTarget.end())
                    {
                        cout << "wires: " << splited[i] << endl;
                        fout << "wire u_" << splited[i] << ";\n";
                        splited[i] = "u_"+splited[i];
                    }
                    //the pin isn't a target
                    else
                    {
                        continue;
                    }
                }
                else//gate input
                {
                    if(allTarget.find(splited[i])!=allTarget.end())
                    {
                        cout << "inputs: " << splited[i] << endl;
                        if(declaredWire.find(splited[i])==declaredWire.end())
                        {
                            fout << "wire t_" << splited[i] << ";" << endl;
                            declaredWire.insert(splited[i]);    
                        }
                        splited[i] = "t_"+splited[i];
                    }
                    //the pin isn't a target
                    else
                    {
                        continue;
                    }
                }
                
            }
            fout << gate << " (";
            for(int i=0;i<splited.size();i++)
            {
                if(splited[i]!="")
                {
                    fout << splited[i];
                    if(i<splited.size()-1)
                        fout << ", ";
                    else
                        fout << ");\n";
                }
                
            }

        }
    }
    fin.close();
    fout.close();
}