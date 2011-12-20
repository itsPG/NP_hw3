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
class PG_TCP
{
public:
	int c_fd, l_fd, pid;
	string my_ip;
	int my_port;
	int harmonics()
	{
		int pid, stat;
		if (pid = fork())
		{
			waitpid(pid, &stat, 0);
			return pid;
		}
		else
		{
			if (fork())
			{
				exit(0);
			}
			else return 0;
		}
	}
	void go()
	{
		struct sockaddr_in sin;
		struct sockaddr_in cin;
		
		socklen_t len;
		char buf[10000]; 
		char addr_p[INET_ADDRSTRLEN]; 
		int port = 8081;
		int n,r; 
       
		bzero(&sin, sizeof(sin)); 
		sin.sin_family = AF_INET; 
		sin.sin_addr.s_addr = INADDR_ANY; 
		sin.sin_port = htons(port);
    
		l_fd = socket(AF_INET, SOCK_STREAM, 0); 
		while(1)
		{
			r = bind(l_fd, (struct sockaddr *)&sin, sizeof(sin));
			cout << "bind: " << sin.sin_port << endl;
			if(r == 0)break;
			usleep(500000);
		}
		r = listen(l_fd, 10); 
		cout << "listen: " << r << endl;
		printf("waiting ...\n");
		while(1)
		{
			usleep(500000);
			c_fd = accept(l_fd, (struct sockaddr *) &cin, &len); 
			my_port = ntohs(cin.sin_port);
			char addr_p[INET_ADDRSTRLEN];
			my_ip = inet_ntop(AF_INET, &cin.sin_addr, addr_p, sizeof(addr_p));
			cout << "accept: " << c_fd << endl;
			cout << "IP: " << my_ip << endl;
			cout << "port: " << ntohs(cin.sin_port) << endl;
			if (pid = harmonics())
			{
				cout << "parent" << endl;
				close(c_fd);
			}
			else
			{
				close(l_fd);
				dup2(c_fd, 0);
				dup2(c_fd, 1);
				//dup2(c_fd, 2);
				close(c_fd);
				return ;
			}
		}
	}
};
int main()
{
	chdir("/var/www/cgi-bin/");
	PG_TCP Rixia;
	Rixia.go();
	cout << "HTTP/1.1 200 OK" << endl;
	cout << "Cache-Control: no-cache" << endl;
	string tmp;
	string cgi_name,query_string;
	while (getline(cin,tmp))
	{
		cerr << tmp << endl;
		string t;
		istringstream scin(tmp);
		scin >> t;
		if (t == "GET")
		{
			scin >> t;
			t.erase(0,1);
			cgi_name = t.substr(0, t.find("?"));
			if (t.find("?") == string::npos)
				t = "";
			else
				t = t.substr(t.find("?")+1);
			//cerr << "SET " << t << endl;
			//setenv("QUERY_STRING", t.c_str(), 1);
			//t = getenv("QUERY_STRING");
			query_string = "QUERY_STRING="+t;
			//cerr << "QUERY_STRING " << t << endl;
		}
		else if (t == "Connection:")break;
	}
	cerr << "cgi_name is " << cgi_name << endl;
	cerr << "query_string is " << query_string << endl;
	char tmp1[1000]={0}, tmp2[1000]={0};
	strcpy(tmp1, cgi_name.c_str());
	strcpy(tmp2, query_string.c_str()); 
	char *argv[] = {tmp1, 0};
	char *envp[] =
	{
		tmp2,
		"CONTENT_LENGTH=2",
		"REQUEST_METHOD=3",
		"SCRIPT_NAME=4",
		"REMOTE_HOST=5",
		"REMOTE_ADDR=6",
		"ANTH_TYPE=7",
		"REMOTE_USER=8",
		"REMOTE_IDENT=9",
		0
	};
	
	//execlp("./1_hw3", "./1_hw3", NULL);
	execve(cgi_name.c_str(), argv, envp);
	//char c;
	//while(cin.get(c))cout << c;
}
