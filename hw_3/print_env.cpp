#include <iostream>
#include <fstream>
#include <sstream>
#include <map>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
using namespace std;
void chkenv(string q)
{
	char *t = getenv(q.c_str());
	if (t == NULL)
		cout << q << " dosent exist!" << endl;
	else
		cout << q << " : " << t << endl;
	cout << "<br/>" << endl;
}
int main()
{
	cout << "Content-Type:text/html;\n\n" << endl;
	chkenv("PATH");
	chkenv("QUERY_STRING");
	chkenv("CONTENT_LENGTH");
	chkenv("REQUEST_METHOD");
	chkenv("SCRIPT_NAME");
	chkenv("REMOTE_HOST");
	chkenv("REMOTE_ADDR");
	chkenv("ANTH_TYPE");
	chkenv("REMOTE_USER");
	chkenv("REMOTE_IDENT");

	
}
