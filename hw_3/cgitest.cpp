#include <iostream>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
using namespace std;
class Http_Parse
{
public:
	map<string, string> arg;
	void set(string q)
	{
		int at = 0;
		while (q[at] != '=') at++;
		//cout << "set " << q.substr(0,at) << " " << q.substr(at+1) << endl;
		arg[q.substr(0,at)] = q.substr(at+1);
	}
	Http_Parse()
	{
		string q = getenv("QUERY_STRING");
		q = q + "&";
		int s = 0, e = 0;
		while (1)
		{
			while (q[e] != '&') e++;
			set(q.substr(s,e-s));
			s = e + 1;
			e += 2;
		}
	}
};
class PG_clients
{
public:
	fd_set rfds, wfds, rs, ws;
	int fd[11];
	struct sockaddr_in sin[11];
	struct hostent *he;
	string host_name[11];
	string data[11];
	int port[11];
	int FSM[11];
	int count[11];
	int connecting[11];
	const static int F_CONNECTING = 1, F_READING = 2, F_WRITING = 3, F_DONE = 4;
	int max;
	PG_clients()
	{
		max = 0;
		FD_ZERO(&rfds); FD_ZERO(&wfds); FD_ZERO(&rs); FD_ZERO(&ws);
		//FD_SET();
	}
	void init(int q)
	{
		count[q] = 0;
		connecting[q] = 1;
		FSM[q] = F_CONNECTING;
		data[q] = "who\nls \n";
		he = gethostbyname(host_name[q].c_str());
		if (he == NULL)
		{
			cerr << "gethostbyname error" << endl;
			exit(1);
		}
		fd[q] = socket(AF_INET, SOCK_STREAM, 0);
		bzero(&sin[q], sizeof(sin[q]));
		sin[q].sin_family = AF_INET;
		sin[q].sin_addr = *((struct in_addr *)he->h_addr); 
		sin[q].sin_port = htons(port[q]);
		int flags = fcntl(fd[q], F_GETFL, 0);
		fcntl(fd[q], F_SETFL, flags | O_NONBLOCK);
		FD_SET(fd[q], &rs);
		FD_SET(fd[q], &ws);
		//rfds = rs; wfds = ws;
	}
	void conekuto(int q)
	{
		if (connect(fd[q], (struct sockaddr *)&sin[q], sizeof(sin[q])) < 0)
		{
			if (errno != EINPROGRESS)
			{
				perror("failed in PG_clients -> conekuto");
				exit(1);
			}
			//cout << "error" << endl;
			FSM[q] = F_CONNECTING;
		}
	}
	int go()
	{
		char buf[100000];
		int error;
		socklen_t n;
		
		while(1)
		{
			memcpy(&rfds, &rs, sizeof(rfds));
			memcpy(&wfds, &ws, sizeof(wfds));
			if (select(1024, &rfds, &wfds, (fd_set*)0, (struct timeval*)0) < 0){perror("select error"); exit(1);}
			for (int i = 1; i <= max; i++)
			{
				int t;
				if (connecting[i])
				{
					if (FD_ISSET(fd[i], &rfds) || FD_ISSET(fd[i], &wfds))
					{
						if (getsockopt(fd[i], SOL_SOCKET, SO_ERROR, (void*)&error, &n) <0 || error != 0)
						{
							perror("select error");
							exit(1);
						}
						connecting[i] = 0;
					}
				}
				else
				{
					if (FD_ISSET(fd[i], &rfds))
					{
						bzero(buf, sizeof(buf));
						t = read(fd[i], buf, sizeof(buf)-1);
						if (t == 0)
						{
							cout << "read finished" << endl;
							return 0;
						}
						cout << "read " << t << endl;
						cout << buf << endl;
					}
					
					if (FD_ISSET(fd[i], &wfds))
					{
						//cout << "write ready" << endl;
						while (count[i] < data[i].size() && write(fd[i], data[i].c_str()+count[i], 1) )
						{
							//cout << "w " << data[i][count[i]] << endl;
							count[i]++;
							//if (data[i][count[i]-1] == '\n')break;
						}
						if (count[i] >= data[i].size())
						{
								cout << "finished" << endl;
								
								FD_CLR(fd[i], &ws);
								//FD_CLR(fd[i], &rs);
						}
						else
						{
							perror("write error");
						}
					}
					
				}
			}
		}
	}
	
};
int main()
{
	PG_clients t;
	t.max = 1;
	t.host_name[1] = "127.0.0.1";
	t.port[1] = 7000;
	t.init(1);
	t.conekuto(1);
	t.go();
	cout << "Content-Type:text/html;\n\n" << endl;
	//cout << "Hello World" << endl;
	//cout << getenv("QUERY_STRING") << endl;
	//Http_Parse a;
}

