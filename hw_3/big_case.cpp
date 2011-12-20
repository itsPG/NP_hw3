#include <iostream>
#include <fstream>
using namespace std;
int main()
{
	ofstream fout("/var/www/cgi-bin/big.txt");
	fout << "ls" << endl;
	for (int i = 1; i <= 2000; i++)
		fout << "ls 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 ! ";
	fout << "number | number | number" << endl;
	fout << "ls" << endl;
	fout << "cat /var/www/cgi-bin/big.txt" << endl;
	fout << "exit" << endl;
	
}
