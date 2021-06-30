//update again
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
void modifyIO_GFile();
void fitoutformat();


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
      Abc_Print( 1, "Expecting two file names on the command line.\n" );
  }
  for ( c = 0; c < argc - 1; c++ )
  {
      string temp = "";
      std::set<string>    wireset_tmp1;
      std::set<string>    wireset_tmp2;
      bool eol = true;
      ifstream pFile(argv[c+1], ios::in);

      if ( !pFile )
      {
          printf( "Cannot open input file \"%s\".\n", argv[c+1] );
          return 0;
      }
      else
      {
          //parse wire
          string tempname = "";
          string begin, end = "";
          while(getline(pFile, temp))
          {
            string title = "";
            for (int i = 0; i < temp.size(); ++i)
            {
              if (eol)
              {
                if (temp[i] == ' ' or temp[i] == ';') title = "";
                else{
                  title = title + temp[i];
                  if (title == "wire" or title == "input" or title == "output" or title == "and" 
                    or title == "or" or title == "nand" or title == "nor" or title == "not" 
                    or title == "buf" or title == "xor" or title == "xnor" or title == "assign") eol = false;
                }
              }
              else
              {
                if (title == "wire" or title == "input" or title == "output"){
                  if (temp[i] == '[' or temp[i] == ' ') ;
                  else if (temp[i] == ':') {
                    begin = tempname;
                    tempname = "";
                  }
                  else if (temp[i] == ']') {
                    end = tempname;
                    tempname = "";
                  }
                  else if (temp[i] == ';' or temp[i] == ',') {
                    if (begin != "" and end != "") {
                      for (int i = stoi(begin); i < stoi(end)+1; ++i) {
                        if (c == 0) wireset_tmp1.insert(tempname + '[' + to_string(i) + ']');
                        else wireset_tmp2.insert(tempname + '[' + to_string(i) + ']');
                        wiretype[tempname + '[' + to_string(i) + ']'] = title;
                        tempname = "";
                      }
                    }
                    else{
                      if (c == 0) wireset_tmp1.insert(tempname);
                      else wireset_tmp2.insert(tempname);
                      wiretype[tempname] = title;
                      tempname = "";
                    }
                  }
                  else tempname = tempname + temp[i];
                }
                else
                {
                  if (temp[i] == ' ') ;
                  else if (temp[i] == '(') {
                    gatelist.push_back(title);
                    gatelist.push_back(tempname);
                    tempname = "";
                  }
                  else if (temp[i] == ')' or temp[i] == ',' or temp[i] == '=' or temp[i] == ';'){
                    if (tempname != "1'b1" and tempname != "1'b0" and tempname != "")
                    {
                      if (c == 0) wireset_tmp1.insert(tempname);
                      else wireset_tmp2.insert(tempname);
                      wiretype[tempname] = "wire";
                    }
                    tempname = "";
                  }
                  else tempname = tempname + temp[i];
                }

                if (temp[i] == ';') {
                  tempname = "";
                  begin = "";
                  end = "";
                  eol = true;
                }
              }
            }
          }
      }
      pFile.close();

  ///////////////////////////////////////write verilog file///////////////////////////////////////////////////
      ifstream p1File(argv[c+1], ios::in);

      eol = true;
      bool check_wire = true;
      bool check_gate = false;
      if (c == 0)
      {
        string title = "";
        string tempname = "";
        while(getline(p1File, temp))
        {
          for (int i = 0; i < temp.size(); ++i)
          {
            if (eol)
            {
              if (temp[i] == ' ' or temp[i] == ';') title = "";
              else{
                title = title + temp[i];
                check_gate = false;
                if (title == "input" or title == "output" 
                  or title == "module") {
                    r1File << title;
                    eol = false;
                  }
                else if (title == "and" or title == "or" or title == "nand" or title == "nor" 
                  or title == "not" or title == "buf" or title == "xor" or title == "xnor" or title == "assign")
                {
                  if (check_wire) {
                    check_wire = false;
                    for (const auto &s : wireset_tmp1) {
                      nFile  << s << endl;
                      r1File << "wire" << ' ' << s << ';' << endl;
                    }
                  }
                  else;
                  r1File << title;
                  eol = false;
                }
              }
            }
            else
            {
              if (title == "module" or title == "input" 
                or title == "output" or title == "assign")
              { 
                r1File << temp[i];
                if (temp[i] == ';') r1File << endl;
              }
              else
              {
                if (temp[i] == '(') {
                  check_gate = true;
                  r1File << ' ';
                }
                if (check_gate) r1File << temp[i];
                else ;
                if (temp[i] == ';') r1File << endl;
              }
              if (temp[i] == ';') {
                tempname = "";
                title = "";
                eol = true;
              }
            }
          }
        }
        r1File << "endmodule" << endl;
      }
      else if (c == 1)
      {
        string title = "";
        string tempname = "";
        while(getline(p1File, temp))
        {
          for (int i = 0; i < temp.size(); ++i)
          {
            if (eol)
            {
              if (temp[i] == ' ' or temp[i] == ';') title = "";
              else{
                title = title + temp[i];
                check_gate = false;
                if (title == "input" or title == "output" 
                  or title == "module") {
                    r2File << title;
                    eol = false;
                  }
                else if (title == "and" or title == "or" or title == "nand" or title == "nor" 
                  or title == "not" or title == "buf" or title == "xor" or title == "xnor" or title == "assign")
                {
                  if (check_wire) {
                    check_wire = false;
                    for (const auto &s : wireset_tmp2) r2File << "wire" << ' ' << s << ';' << endl;
                  }
                  r2File << title;
                  eol = false;
                }
              }
            }
            else
            {
              if (title == "module" or title == "input" 
                or title == "output" or title == "assign")
              { 
                r2File << temp[i];
                if (temp[i] == ';') r2File << endl;
              }
              else
              {
                if (temp[i] == '(') {
                  check_gate = true;
                  r2File << ' ';
                }
                if (check_gate) r2File << temp[i];
                else ;
                if (temp[i] == ';') r2File << endl;
              }
              if (temp[i] == ';') {
                tempname = "";
                title = "";
                eol = true;
              }
            }
          }
        }
        r2File << "endmodule" << endl;
      }
      p1File.close();
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

  //////////// modify I/O port of G.v ////////////
  modifyIO_GFile();



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

void printSet(set<string> s)
{
    for(auto& i: s)
        cout << i << " , ";
    cout << endl;
}

void insertTarget(string inFName, string oFName="F.v")
{
    fstream fin, fout, wfout;
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
    wfout.open("weight.txt",ios::out);
    string line;
    vector<string>  splited;
    set<string> oNames, iNames;
    set<string> declaredWire;
    while(getline(fin,line))
    {
        //cout << line << endl;
        if(line.find("module")==0 || line.find("endmodule")==0)
        {
            fout << line << endl;
            continue;
        }
        if(line.find("wire")==0)
        {
            //fout << line << endl;
            continue;
        }
        if(line.find("output")==0)
        {
            splited = split(line," ");
            int stInd = line.find("[")==7 ? 2:1;
            for(int i=stInd;i<splited.size();i++)
            {
                trim(splited[i],";\n");
                trim(splited[i],",");
                int ind1 = line.find("[");
                int ind2 = line.find(":");
                if(ind1!=string::npos)
                {
                    //cout << "subStr:" << line.substr(ind1+1, ind2-ind1-1) << endl;
                    
                    int arraySize = stoi(line.substr(ind1+1, ind2-ind1-1))+1;
                    for(int j=0;j<arraySize;j++)
                    {
                      string str2Insert = splited[i]+'['+to_string(j)+']';
                      //cout << "str2Insert: " << str2Insert << endl;
                      oNames.insert(str2Insert);
                      fout << "output " << str2Insert << ";\n";
                    }
                }
                else
                {
                    oNames.insert(splited[i]);
                    //fout << line << endl;
                }
                wfout << splited[i] << " 1" << endl;
            }
            if(line.find("[")==string::npos)
              fout << line << endl;
            
        }
        else if(line.find("input")==0)
        {
            splited = split(line," ");
            int stInd = line.find("[")==6 ? 2:1;
            int ind1 = line.find("[");
            int ind2 = line.find(":");
            
            for(int i=stInd;i<splited.size();i++)
            {
                trim(splited[i],";\n");
                trim(splited[i],",");
                //cout << "splited: " << splited[i] << endl;
                if(ind1!=string::npos)
                {
                    int arraySize = stoi(line.substr(ind1+1, ind2-ind1-1))+1;
                    for(int j=0;j<arraySize;j++)
                    {
                      string str2Insert = splited[i]+'['+to_string(j)+']';
                      iNames.insert(str2Insert);
                      fout << "input " << str2Insert << ";\n";
                    }
                }
                else
                {
                    iNames.insert(splited[i]);
                    //fout << line << endl;
                }
                wfout << splited[i] << " 1" << endl;
            }
            if(line.find("[")==string::npos)
              fout << line << endl;
            
            
        }
        else if(line.find("assign")==0)
        {
            splited = split(line," ");
            assert(splited.size()==4);
            cout << "string to search: " << splited[1] << " and " << splited[3] << endl;
            if(oNames.find(splited[1])!=oNames.end() && allTarget.find(splited[1])!=allTarget.end())
            {
                trim(splited[3],";");
                cout << "found: " << (oNames.find(splited[3])!=oNames.end()) << endl;
                if(oNames.find(splited[3])!=oNames.end())
                    fout << line << endl;
                else
                    fout << "assign " << splited[1] << " = t_" << splited[3] << ";" << endl;
            }
            else if(allTarget.find(splited[1])!=allTarget.end())
            {
                fout << "wire u_" << splited[1] << ";\n";
                trim(splited[3],";");
                if(declaredWire.find(splited[3])==declaredWire.end())
                {
                    fout << "wire t_" << splited[3] << ";" << endl;
                    declaredWire.insert(splited[3]);    
                }
                fout << "assign u_" << splited[1] << " = t_" << splited[3] << ";\n";
            }
        }
        else
        {
            string gate = line.substr(0,line.find(" "));
            //cout << "gate: " << gate << endl;
            size_t st=line.find("("), end=line.find(")");
            string wireStr = line.substr(st+1,end-st-1);
            trim(wireStr);
            //cout << "wireStr: " << wireStr << endl;
            splited = split(wireStr," ");
            for(int i=0;i<splited.size();i++)//for each pin of a gate
            {
                trim(splited[i],",");
                if(splited[i]=="")
                    continue;
                //cout << i << ": " << splited[i] << endl;
                if(i==0)//gate output
                {
                    //the pin is PO and is target
                    if(oNames.find(splited[i])!=oNames.end() && allTarget.find(splited[i])!=allTarget.end())
                    {
                        //cout << "outputs: " << splited[i] << endl;
                        fout << "wire t_" << splited[i] << ", u_" << splited[i] << ";\n";
                        fout << "buf (" << splited[i] << ",t_" << splited[i] << ");\n";
                        splited[i] = "u_"+splited[i];
                        wfout << splited[i] << " 1" << endl;
                    }
                    //the pin is internal wire and is target
                    else if(allTarget.find(splited[i])!=allTarget.end())
                    {
                        //cout << "wires: " << splited[i] << endl;
                        fout << "wire u_" << splited[i] << ";\n";
                        splited[i] = "u_"+splited[i];
                        wfout << splited[i] << " 1" << endl;
                    }
                    //the pin isn't a target
                    else
                    {
                        wfout << splited[i] << " 1" << endl;
                        continue;
                    }
                }
                else//gate input
                {
                    if(allTarget.find(splited[i])!=allTarget.end() && iNames.find(splited[i])==iNames.end())
                    {
                        //cout << "inputs: " << splited[i] << endl;
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
    wfout.close();

    // cout << "inputs: \n";
    // printSet(iNames); 
    // cout << "outputs: \n";
    // printSet(oNames);    
}



void modifyIO_GFile()
{
  fstream fin, fout;
  fin.open("G.v",ios::in);
  fout.open("G_t.v",ios::out);
  string line;
  vector<string> splited;
  int num;
  while(getline(fin,line))
  {
    if(line.find("input")==0)
    {
      if(line.find("[")==6)
      {
        splited = split(line," ");
        int ind = splited[1].find(":");
        num = stoi(splited[1].substr(1,ind-1))+1;
        for(int i=2;i<splited.size();i++)
        {
          trim(splited[i],";");
          trim(splited[i],",");
          for(int j=0;j<num;j++)
          {
            fout << "input " << splited[i] << "[" << to_string(j) << "];\n";
          }
        }
        
      }
      else
      {
        fout << line << endl;
      }
    }
    else if(line.find("output")==0)
    {
      if(line.find("[")==7)
      {
        splited = split(line," ");
        int ind = splited[1].find(":");
        num = stoi(splited[1].substr(1,ind-1))+1;
        for(int i=2;i<splited.size();i++)
        {
          trim(splited[i],";");
          trim(splited[i],",");
          for(int j=0;j<num;j++)
          {
            fout << "output " << splited[i] << "[" << to_string(j) << "];\n";
          }
        }
      }
      else
      {
        fout << line << endl;
      }
    }
    else
    {
      fout << line << endl;
    }
  }
  fin.close();
  fout.close();
}

