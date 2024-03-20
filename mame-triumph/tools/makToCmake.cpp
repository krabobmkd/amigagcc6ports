#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;
int main(int argc,char **argv)
{
    ifstream ifs("mame.mak","rb")
    if(!ifs.good()) {
        cout << "need mame.mak" << endl;
        return  1;
    }


	return  0;
}
