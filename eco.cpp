#include <fstream>
#include <string>
#include <iostream>

using namespace std;

void fitoutformat(string);

int main(int argc, char * argv[])
{
    string cmd;
    string f1=argv[3];
    string f2=argv[2];
    cmd = "./abc -c \"eco_modify_verilog " + f1 + " " + f2 + "\"";
    cout << cmd << endl;
    system(cmd.c_str());
    cmd = "./abc -c \"runeco F_t.v G_t.v weight.txt\"";
    system(cmd.c_str());

    system("mv patch.v _intermediate_patch.v");

    fitoutformat(argv[4]);
    return 0;
}


void fitoutformat(string foutName){
    fstream inFile, outFile;
    inFile.open("_intermediate_patch.v",ios::in);
    outFile.open(foutName,ios::out);
    string line;
    string addin = "_in";
    int pos = 0, pos2 = 0;
    int pos3 = 0;
    int flag = 0;
    int gateCount=0;
    while(getline(inFile,line)){
        if(line.find("input") != -1) flag = 1;
        while(pos != -1 || pos2 != -1){
            pos = line.find("t_");
            if(pos != -1) line.erase(pos,2);
            pos2 = line.find("u_");
            if(pos2 != -1) {
                line.erase(pos2,2);
                string temp = line.substr(pos2);
                pos3 = temp.find(",");
                if(pos3 == -1){
                    if(flag == 1) pos3 = temp.find(";");
                    else pos3 = temp.find(" ");
                }
                line.insert(pos3+pos2, addin);
            }
        }
        if(line.find("module")!=0)//not the module line
        {
            if(line.find("output")!=string::npos || line.find("input")!=string::npos || line.find("wire")!=string::npos || line.find("endmodule")!=string::npos || line.find("//")!=string::npos || line=="")
            {
                outFile<<line<<endl;
            }
            else
            {
                //cout << "line: " << line << endl;
                int ind = line.find("(");
                string newline = line.substr(0,ind);
                newline+=("eco_g"+to_string(gateCount));
                newline+=(" "+line.substr(ind));
                outFile<<newline<<endl;
                gateCount++;
            }
        }
        else
        {
            //cout << "substring:\n" << line.substr(12) << endl;
            string newline="module top_eco" + line.substr(12);
            outFile<<newline<<endl;
        }
        pos = 0;
        pos2 = 0;
        pos3 = 0;
        flag = 0;
    }
    inFile.close();
    outFile.close();
}
