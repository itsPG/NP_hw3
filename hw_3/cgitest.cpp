#include <iostream>
#include <map>
#include <fstream>
#include <sstream>

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
string i2s(int q)
{
	ostringstream sout;
	sout << q;
	return sout.str();
}
int s2i(string q)
{
	int r;
	istringstream ssin(q);
	ssin >> r;
	return r;
}
class PG_http_parse
{
public:
	map<string, string> arg;
	void set(string q)
	{

		int at = 0;
		while (q[at] != '=' && at < q.size()) at++;
		if (at == q.size()-1) return;

		arg[q.substr(0,at)] = q.substr(at+1);

	}
	void parse()
	{
		string q = getenv("QUERY_STRING");
		//cout << "QUERY_STRING " << q << endl;
		q = q + "&";
		int s = 0, e = 0;
		while (s < q.size())
		{
			while (q[e] != '&') e++;
			set(q.substr(s,e-s));
			s = e + 1;
			e += 2;
		}
	}
	bool chk_arg(string q)
	{
		if (arg.find(q) != arg.end())return 1;
		else return 0;
	}
	void list_arg()
	{
		for (map<string, string>::iterator i = arg.begin(); i != arg.end(); ++i)
		{
			cout << i->first << " " << i->second << endl;
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
	string cmd[11][10001];
	int port[11];
	int FSM[11];
	int count[11];
	int cmd_m[11];
	bool done_flag[11];
	const static int F_CONNECTING = 1, F_READING = 2, F_WRITING = 3, F_LAST_READ = 4, F_DONE = 5;
	int max;
	int alive;
	bool pa_detect;
	PG_clients()
	{
		max = 0;
		alive = 0;
		FD_ZERO(&rfds); FD_ZERO(&wfds); FD_ZERO(&rs); FD_ZERO(&ws);
	}
	void init(int q)
	{
		count[q] = 0;
		done_flag[q] = 0;
		FSM[q] = F_CONNECTING;
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
	}
	void conekuto(int q)
	{
		if (connect(fd[q], (struct sockaddr *)&sin[q], sizeof(sin[q])) < 0)
		{
			if (errno != EINPROGRESS){perror("failed in PG_clients -> conekuto"); exit(1);}
			FSM[q] = F_CONNECTING;
		}
		alive++;
	}
	void add_user(string ip, string pt, string fn)
	{
		max++;
		host_name[max] = ip;
		port[max] = s2i(pt);
		data[max] = "";
		ifstream fin(fn.data());
		char c;
		while (fin.get(c)) data[max] += c;
		//cout << "client " << max << " get " << data[max] << endl;
		
		init(max);
		conekuto(max);
	}
	string recv_msg(int q)
	{
		pa_detect = 0;
		char c;
		string msg = "";
		while (read(fd[q], &c, 1) > 0)
		{
			if (c == '\r') continue;
			//cout << "!R!" << endl;
			msg += c;
			if (c == '%')pa_detect = 1;
			if (c == '\n')
			{
				break;
			}
		}
		return msg;
	}
	int send_msg(int q) // return 0 when success, 1 when there's something left.
	{
		string tmp = "";
		while (data[q][count[q]] != '\n' && count[q]<data[q].size())
		{
			tmp += data[q][count[q]];
			count[q]++;
		}
		if (tmp == "exit") done_flag[q] = 1;
		tmp += data[q][count[q]];
		count[q]++;

		int len = write(fd[q], tmp.c_str(), tmp.size());
		if (len < tmp.size())
		{
			count[q] -= tmp.size() - len;
			return 1;
		}
		return 0;
	}
	void print_msg_unit(string q, string at, int i)
	{
		cout << "<script language = \"JavaScript\">document.all[\'";
		cout << at << i;
		cout << "\'].innerHTML += \"" << q << "\";</script>" << endl;
	}
	void print_msg(string q, int i)
	{
		if(q.find('\n') != string::npos)
		{
			q.erase(q.find('\n'),1);
		}
		print_msg_unit(q+"<br/>", "m", i);
	}
	void print_head(int q)
	{
		print_msg_unit(host_name[q], "ip", q);
	}
	int go()
	{
		char buf[100000];
		int error;
		socklen_t n;
		
		while(alive)
		{
			memcpy(&rfds, &rs, sizeof(rfds));
			memcpy(&wfds, &ws, sizeof(wfds));
			if (select(1024, &rfds, &wfds, (fd_set*)0, (struct timeval*)0) < 0){perror("select error"); exit(1);}
			int t;
			usleep(30000);
			for (int i = 1; i <= max; i++)
			{
				switch(FSM[i])
				{
					case F_CONNECTING:
						if (FD_ISSET(fd[i], &rfds) || FD_ISSET(fd[i], &wfds))
						{
							if (getsockopt(fd[i], SOL_SOCKET, SO_ERROR, (void*)&error, &n) <0 || error != 0)
							{
								perror("select error");
								exit(1);
							}
							FD_CLR(fd[i], &ws);
							FSM[i] = F_READING;
							print_head(i);
						}
						break;
					
					case F_WRITING:
						if (FD_ISSET(fd[i], &wfds))
						{
							int flag = send_msg(i);
							if (flag) break;
							FD_CLR(fd[i], &ws);
							FD_SET(fd[i], &rs);
							FSM[i] = F_READING;
							if (done_flag[i])
							{
								FD_CLR(fd[i], &ws);
								FD_CLR(fd[i], &rs);
								shutdown(fd[i],2);
								close(fd[i]);
								alive--;
							}
						}
						break;
						
					case F_READING:
						if (FD_ISSET(fd[i], &rfds))
						{
							string msg = recv_msg(i);
							if (msg == "")
							{
								//cout << "read error!!" << endl;
								FD_CLR(fd[i], &ws);
								FD_CLR(fd[i], &rs);
								shutdown(fd[i],2);
								close(fd[i]);
								alive--;
							}
							print_msg(msg, i);
							if (pa_detect)
							{
								FD_SET(fd[i], &ws);
								FD_CLR(fd[i], &rs);
								FSM[i] = F_WRITING;
							}
						}
						break;
				}
			}
		}
		//cout << "<script >alert(document.all[\'m1\'].innerHTML);</script>" << endl;
		cout << "go end" << endl;
	}
};
class PG_hw3_main
{
public:
	string head;
	void output_head()
	{
		head = "";
		ifstream fin("/home/PG/NP/hw_3/head.html");
		char c;
		while (fin.get(c))head += c;
		cout << head;
	}
	void main()
	{
		output_head();
		PG_clients clients;
		PG_http_parse h;
		h.parse();
		//h.list_arg();
		//return;
		if (h.chk_arg("h1")) clients.add_user(h.arg["h1"], h.arg["p1"], h.arg["f1"]);
		if (h.chk_arg("h2")) clients.add_user(h.arg["h2"], h.arg["p2"], h.arg["f2"]);
		if (h.chk_arg("h3")) clients.add_user(h.arg["h3"], h.arg["p3"], h.arg["f3"]);
		if (h.chk_arg("h4")) clients.add_user(h.arg["h4"], h.arg["p4"], h.arg["f4"]);
		if (h.chk_arg("h5")) clients.add_user(h.arg["h5"], h.arg["p5"], h.arg["f5"]);
		clients.go();
	}
};
int main()
{
	cout << "Content-Type:text/html;\n\n" << endl;

	PG_hw3_main Rixia;
	Rixia.main();

	return 0;

}

