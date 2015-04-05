#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <set>
using namespace std;
int main(int argc, char *argv[])
{
    std::fstream bigDataFS;
	bigDataFS.open( fileName , std::fstream::in );
	if( !bigDataFS.is_open() ){
		std::cout << "Open file error!!" << std::endl;
		return;
	}
    int a,b,c,d,e,f,g,h,i,j,k;
    while( bigDataFS.peek() != EOF ){
        bigDataFS >> a >> b >> c >> d >> e >> f >> g >> h >> i >> j >> k ;
        cout << a <<" ";
    }
    return 0;
}
