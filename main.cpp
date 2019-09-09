#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sys/wait.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <fcntl.h>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
using namespace std;
extern char **environ;
void redirCall(char **);
void redirCallReverse(char **);
void builtIn(char **);
void pipesCalling(int,char **);
void callCd(char **);
void callEcho(char **);
void spawningChild(char**,int);
void redirCallAppend(char **);
int status;
bool scriptFlag = false;
int scriptFd;
map<string,string> environmentVar;
int main()
{
	set<string> builtInCommands = {"echo","cd","history"};
	ifstream configFile("/home/aman/Desktop/CPPprograms/LinuxShellImplementation/config.txt");
	string variable;
	while(getline(configFile,variable))
	{
		int pos = variable.find("=");
		string lval = variable.substr(0,pos);
		string rval = variable.substr(pos+1,variable.size()-1);
		environmentVar.insert(make_pair(lval,rval));
		char *temp1 = (char *)(malloc(sizeof(char) * lval.size()+1));
		unsigned int i = 0;
		for(i = 0;i < lval.size();i++)
		{
			temp1[i] = lval[i];
		}
		temp1[i] = '\0';
		char *temp2 = (char *)(malloc(sizeof(char) * rval.size()+1));
		i = 0;
		for(i = 0;i < rval.size();i++)
		{
			temp2[i] = rval[i];
		}
		temp2[i] = '\0';
		setenv(temp1,temp2,1);
		//cout<<getenv(temp1)<<endl;
	}
	string command;
	cout<<environmentVar["PWD"]<<environmentVar["PS1"];
	while (getline(cin,command))
	{
		
		if(command == "")
		{
			cout<<environmentVar["PWD"]<<environmentVar["PS1"];
			continue;
		}
		if(command.find("=") != string::npos)
		{
			if(command.find("alias") != string::npos)
			{
				command.erase(0,6);
			}
			int pos = command.find("=");
			string lval = command.substr(0,pos);
			string rval = command.substr(pos+2,command.size()-1);
			rval.erase(rval.size()-1,1);
			environmentVar[lval] = rval;
			cout<<environmentVar["PWD"]<<environmentVar["PS1"];
			continue;
		}
		stringstream stream(command);
		string arguments;
		char *argv[100];
		int j = 0;
		while(stream >> arguments)
		{
			if(environmentVar.find(arguments) != environmentVar.end())
			{
				string aliasValue = environmentVar[arguments];
				stringstream argsExtract(aliasValue);
				string extract;
				while(argsExtract >> extract)
				{
					argv[j]= (char *)(malloc(sizeof(char) * extract.size() + 1));
					for(unsigned int i = 0;i<extract.size();i++)
					{
						argv[j][i] = extract[i];
					}
					argv[j][extract.size()] = '\0';
					j++;
				}
				continue;
			}
			argv[j]= (char *)(malloc(sizeof(char) * arguments.size() + 1));
			for(unsigned int i = 0;i<arguments.size();i++)
			{
				argv[j][i] = arguments[i];
			}
			argv[j][arguments.size()] = '\0';
			j++;
		}
		argv[j] = NULL;	
		if((strcmp(argv[0],"record") == 0)&& (strcmp(argv[1],"start") == 0))
		{
			scriptFlag = true;
			if(argv[2] == NULL)
				scriptFd = open("typescript",O_CREAT | O_RDWR | O_TRUNC ,0666);
			
			else
				scriptFd = open(argv[2],O_CREAT | O_RDWR | O_APPEND,0666);
			//cout<<scriptFd<<endl;
			cout<<environmentVar["PWD"]<<environmentVar["PS1"];
			continue;
		}
		if((strcmp(argv[0],"record") == 0) && (strcmp(argv[1],"stop") == 0))
		{
			scriptFlag = false;
			cout<<environmentVar["PWD"]<<environmentVar["PS1"];
			continue;
		}
		if(builtInCommands.find(argv[0]) != builtInCommands.end())
		{
			builtIn(argv);	
			cout<<environmentVar["PWD"]<<environmentVar["PS1"];
			continue;
		}
		if(strcmp(argv[0],"$?")==0)
		{
			cout<<status<<endl;
			cout<<environmentVar["PWD"]<<environmentVar["PS1"];
			continue;
		}
		if(strcmp(argv[0],"$$") == 0)
		{
			cout<<getpid()<<endl;
			cout<<environmentVar["PWD"]<<environmentVar["PS1"];
			continue;
		}
		bool flagPipe = false;
		bool flagRedir = false;
		bool flagRedirAppend = false;
		bool flagRedirReverse = false;
		for(int i = 0;argv[i] != NULL;i++)
		{
			if((strcmp(argv[i],">")) == 0)
			{
				flagRedir = true;
			}
		}
		for(int i = 0;argv[i] != NULL;i++)
		{
			if((strcmp(argv[i],">>")) == 0)
			{
				flagRedirAppend = true;
			}
		}
		for(int i = 0;argv[i] != NULL;i++)
		{
			if((strcmp(argv[i],"<")) == 0)
			{
				flagRedirReverse = true;
			}
		}
		int count = 0;
		for(int i = 0;argv[i] != NULL;i++)
		{
			if((strcmp(argv[i],"|")) == 0)
			{
				count++;
				flagPipe = true;
			}
		}
		if(flagPipe && flagRedirAppend)
		{
			redirCallAppend(argv);
			//cout<<environmentVar["PS1"]<<endl;
		}
		if(flagPipe && flagRedir)
		{
			redirCall(argv);
			//cout<<environmentVar["PS1"]<<endl;
		}
		else if(flagPipe && flagRedirReverse)
		{
			redirCallReverse(argv);
		}
		else if(flagPipe)
		{
			pipesCalling(count,argv);
		}
		else if(flagRedir)
		{
			redirCall(argv);
		}
		else if(flagRedirAppend)
		{
			//cout<<"call"<<endl;
			redirCallAppend(argv);
		}
		else if(flagRedirReverse)
		{

			redirCallReverse(argv);
		}
		else
		{
			spawningChild(argv,0);
		}
		cout<<environmentVar["PWD"]<<environmentVar["PS1"];
	}
}
void copyToTerminal()
{
	fstream fs("helper");
	fstream fs2("typescript",fstream::app);
	string line;
	while(getline(fs,line))
	{
		string str2  =line + "\n";
		stringstream stream(str2);
		string str;
		while(stream >> str)
		{
			fs2<<str;
			fs2<<" ";
		}
		fs2<<endl;
		char buff[line.size()];
		for(unsigned int i = 0;i<line.size();i++)
		{
			buff[i] = line[i];
		}
			
			write(STDOUT_FILENO,&buff,line.size());			
			cout<<endl;
	}
	return;
}
void spawningChild(char **argv,int type)
{
	//cout<<i<<endl;
	int pid;
	if(scriptFlag && type != 225)
	{
		int scriptFd2 = open("helper",O_CREAT | O_RDWR | O_TRUNC ,0666);
		int tmpout;
		tmpout = dup(1);
		if ((pid = fork()) < 0)
		{
			cout<<"fork error"<<endl;
		} 
		else if (pid == 0)
		{			
			dup2(scriptFd2,1);
			execvp(argv[0],argv);
			cout<<"couldn’t execute:"<<endl;
			exit(127);
		}
		if ((pid = waitpid(pid, &status, 0)) < 0)
			cout<<"waitpid error"<<endl;
		close(scriptFd2);
		dup2(tmpout,1);
		copyToTerminal();
		return;
	}
	if ((pid = fork()) < 0)
	{
		cout<<"fork error"<<endl;
	} 
	else if (pid == 0)
	{
		//cerr<<"came once"<<endl;
		execvp(argv[0],argv);
		cout<<"couldn’t execute:"<<endl;
		exit(127);
	}
	if ((pid = waitpid(pid, &status, 0)) < 0)
		cout<<"waitpid error"<<endl;
	return;
}
void pipesCalling(int times,char **argv)
{
	int tmpin,tmpout;
	tmpin = dup(0);
	tmpout = dup(1);
	int fdin = dup(tmpin);	
	int count = 0;
	int fdout;
	int z = 0,n=0;
	while(argv[z] != NULL)
	{
		n++;
		z++;
	}
	/*int j = 0;
		while(argv[j])
		{
			cout<<argv[j]<<endl;
			j++;
		}*/
	for(int i = 0;i<n;i++)
	{
		if((strcmp(argv[i],"|")) == 0)
			continue;
		char *argv2[100];
		int argsINCommand = 0;
		
		while((i < n)  && ((strcmp(argv[i],"|")) != 0))
		{
			argv2[argsINCommand]= (char *)(malloc(sizeof(char) * (strlen(argv[i]) + 1)));
			for(unsigned int k = 0;k < strlen(argv[i]);k++)
			{
				argv2[argsINCommand][k] = argv[i][k];
			}
			argv2[argsINCommand][strlen(argv[i])+1] = '\0';
			//cout<<argv2[argsINCommand]<<endl;
			i++;
			argsINCommand++;
		}
		i--;
		
		argv2[argsINCommand] = NULL;
		//int j = 0;
		/*while(argv2[j])
		{
			cout<<argv2[j]<<endl;
			j++;
		}*/
		dup2(fdin,0);
		close(fdin);
		int fd[2];
		if(count == times)
		{
			fdout = dup(tmpout);
			//cerr<<"inside"<<endl;
		}
		
		else
		{
			pipe(fd);
			fdin = fd[0];
			fdout = fd[1];
		}
		dup2(fdout,1);
		close(fdout);
		//cerr<<"Inside pip"<<fdout<<endl;
		if(count == times)
		{
			//cerr<<count<<endl;
			//int i = 0;
			/*while(argv2[i])
			{
				cout<<argv2[i]<<endl;
				i++;
			}*/
			spawningChild(argv2,0);
		}
		else
		{
			spawningChild(argv2,225);
		}
		count++;
	}
	dup2(tmpout,1);
	dup2(tmpin,0);
	close(tmpin);
	close(tmpout);	
}

void redirCall(char **argv)
{
	//cout<<"got call"<<endl;
	int count = 0,k = 0,count2 = 0;
	
	while(argv[k] != NULL)
	{
		if((strcmp(argv[k],"|")) == 0)
			count++;
		if((strcmp(argv[k],">")) == 0)
			count2++;
		k++;
	}
	//cout<<count<<" "<<count2<<endl;
	if(count2 > 1)
		cout<<"command does not exist"<<endl;
	if(count == 0)
	{
		//cout<<"kjjkd"<<endl;
		int i = 0;
		while(true)
		{
			if((strcmp(argv[i],">")) == 0)
			{
				i++;
				break;
			}
			i++;
		}
		int fd = open(argv[i],O_CREAT | O_RDWR | O_TRUNC,0666);
		char *argv2[10];
		int argsINCommand = 0;
		int j=0;
		while(((strcmp(argv[j],">")) != 0))
		{
			argv2[argsINCommand]= (char *)(malloc(sizeof(char) * (strlen(argv[j]) + 1)));
			for(unsigned int k = 0;k < strlen(argv[j]);k++)
			{
				argv2[argsINCommand][k] = argv[j][k];
			}
			argv2[argsINCommand][strlen(argv[i])+1] = '\0';
			j++;
			argsINCommand++;
		}
		argv2[j] = NULL;
		int tmpout = dup(1);
		dup2(fd,1);
		close(fd);
		spawningChild(argv2,0);		
		dup2(tmpout,1);		
	}
	if(count > 0)
	{
		int i = 0;
		while(true)
		{
			if((strcmp(argv[i],">")) == 0)
			{
				i++;
				break;
			}
			i++;
		}
		int fd = open(argv[i],O_CREAT | O_RDWR | O_TRUNC,0666);
		char *argv2[10];
		int argsINCommand = 0;
		int j=0;
		int pipesCount = 0;
		while(((strcmp(argv[j],">")) != 0))
		{
			if(((strcmp(argv[j],"|")) == 0))
			{
				pipesCount++;
			}
			argv2[argsINCommand]= (char *)(malloc(sizeof(char) * (strlen(argv[j]) + 1)));
			for(unsigned int k = 0;k < strlen(argv[j]);k++)
			{
				argv2[argsINCommand][k] = argv[j][k];
			}
			argv2[argsINCommand][strlen(argv[i])+1] = '\0';
			j++;
			argsINCommand++;
		}
		argv2[j] = NULL;
		int tmpout = dup(1);
		dup2(fd,1);
		close(fd);
		pipesCalling(pipesCount,argv2);
		//cerr<<"did not"<<endl;
		dup2(tmpout,1);
	}
}

void redirCallAppend(char **argv)
{
	//cout<<"got call"<<endl;
	int count = 0,k = 0,count2 = 0;
	
	while(argv[k] != NULL)
	{
		if((strcmp(argv[k],"|")) == 0)
			count++;
		if((strcmp(argv[k],">>")) == 0)
			count2++;
		k++;
	}
	//cout<<count<<" "<<count2<<endl;
	if(count2 > 1)
		cout<<"command does not exist"<<endl;
	if(count == 0)
	{
		//cout<<"kjjkd"<<endl;
		int i = 0;
		while(true)
		{
			if((strcmp(argv[i],">>")) == 0)
			{
				i++;
				break;
			}
			i++;
		}
		int fd = open(argv[i],O_CREAT | O_RDWR | O_APPEND,0666);
		char *argv2[10];
		int argsINCommand = 0;
		int j=0;
		while(((strcmp(argv[j],">>")) != 0))
		{
			argv2[argsINCommand]= (char *)(malloc(sizeof(char) * (strlen(argv[j]) + 1)));
			for(unsigned int k = 0;k < strlen(argv[j]);k++)
			{
				argv2[argsINCommand][k] = argv[j][k];
			}
			argv2[argsINCommand][strlen(argv[i])+1] = '\0';
			j++;
			argsINCommand++;
		}
		argv2[j] = NULL;
		int tmpout = dup(1);
		dup2(fd,1);
		close(fd);
		spawningChild(argv2,0);		
		dup2(tmpout,1);		
	}
	if(count > 0)
	{
		int i = 0;
		while(true)
		{
			if((strcmp(argv[i],">>")) == 0)
			{
				i++;
				break;
			}
			i++;
		}
		int fd = open(argv[i],O_CREAT | O_RDWR | O_APPEND,0666);
		char *argv2[10];
		int argsINCommand = 0;
		int j=0;
		int pipesCount = 0;
		while(((strcmp(argv[j],">>")) != 0))
		{
			if(((strcmp(argv[j],"|")) == 0))
			{
				pipesCount++;
			}
			argv2[argsINCommand]= (char *)(malloc(sizeof(char) * (strlen(argv[j]) + 1)));
			for(unsigned int k = 0;k < strlen(argv[j]);k++)
			{
				argv2[argsINCommand][k] = argv[j][k];
			}
			argv2[argsINCommand][strlen(argv[i])+1] = '\0';
			j++;
			argsINCommand++;
		}
		argv2[j] = NULL;
		int tmpout = dup(1);
		dup2(fd,1);
		close(fd);
		pipesCalling(pipesCount,argv2);
		//cerr<<"did not"<<endl;
		dup2(tmpout,1);
	}
}

void redirCallReverse(char **argv)
{
	int count = 0,k = 0,count2 = 0;
	
	while(argv[k] != NULL)
	{
		if((strcmp(argv[k],"|")) == 0)
			count++;
		if((strcmp(argv[k],"<")) == 0)
			count2++;
		k++;
	}
	//cout<<count<<" "<<count2<<endl;
	if(count2 > 1)
		cout<<"command does not exist"<<endl;
	if(count == 0)
	{
		//cout<<"kjjkd"<<endl;
		int i = 0;
		while(true)
		{
			if((strcmp(argv[i],"<")) == 0)
			{
				i++;
				break;
			}
			i++;
		}
		int fd = open(argv[i],O_RDONLY);
		char *argv2[100];
		int argsINCommand = 0;
		int j=0;
		while(((strcmp(argv[j],"<")) != 0))
		{
			argv2[argsINCommand]= (char *)(malloc(sizeof(char) * (strlen(argv[j]) + 1)));
			for(unsigned int k = 0;k < strlen(argv[j]);k++)
			{
				argv2[argsINCommand][k] = argv[j][k];
			}
			argv2[argsINCommand][strlen(argv[i])+1] = '\0';
			j++;
			argsINCommand++;
		}
		argv2[j] = NULL;
		int tmpin = dup(0);
		dup2(fd,0);
		close(fd);
		spawningChild(argv2,0);
		dup2(tmpin,0);		
	}
	if(count > 0)
	{
		int i = 0;
		while(true)
		{
			if((strcmp(argv[i],"<")) == 0)
			{
				i++;
				break;
			}
			i++;
		}
		int fd = open(argv[i],O_RDONLY);
		char *argv2[100];
		int argsINCommand = 0;
		int j=0;
		int pipesCount = 0;
		while(((strcmp(argv[j],"<")) != 0))
		{
			if(((strcmp(argv[j],"|")) == 0))
			{
				pipesCount++;
			}
			argv2[argsINCommand]= (char *)(malloc(sizeof(char) * (strlen(argv[j]) + 1)));
			for(unsigned int k = 0;k < strlen(argv[j]);k++)
			{
				argv2[argsINCommand][k] = argv[j][k];
			}
			argv2[argsINCommand][strlen(argv[i])+1] = '\0';
			j++;
			argsINCommand++;
		}
		argv2[j] = NULL;
		int tmpin = dup(0);
		dup2(fd,0);
		close(fd);
		pipesCalling(pipesCount,argv2);
		dup2(tmpin,0);
	}
}

void builtIn(char **argv)
{
	if(strcmp(argv[0],"cd") == 0)
	{
		callCd(argv);
		return;
	}
	if(strcmp(argv[0],"echo") == 0)
	{
		callEcho(argv);
		return;
	}
}
void callCd(char **argv)
{
	if(argv[1][0] == '~')
	{
		char *temp = argv[1];
		int size = environmentVar["HOME"].size();
		string home = environmentVar["HOME"];
		argv[1] = (char *)(malloc(sizeof(char) * size + 1));
		int i = 0;
		for(i =0;i<size;i++)
			argv[1][i] = home[i];
		argv[1][i] = '\0';
		strcat(argv[1],temp+1);	
	
	}
	int exitstat = chdir(argv[1]);
	if(exitstat == -1)
		cout<<"could notchange cwd"<<endl;
	else
	{
		char newpwd[256];
		getcwd(newpwd,256);
		//cout<<newpwd<<endl;
		int size = strlen(newpwd);
		//cout<<"get"<<endl;
		string str(size,'c');
		for(int i =0;i<size;i++)
			str[i] = newpwd[i];
		environmentVar["PWD"] = str;
	}
}
void callEcho(char **argv)
{
	int i = 1;
	if(strcmp(argv[i],"$?")==0)
	{
		cout<<status<<endl;
		return;
	}
	if(strcmp(argv[i],"$$") == 0)
	{
		cout<<getpid()<<endl;
		return;
	}
	while(argv[i] != NULL)
	{
		if(argv[i][0] == '$')
		{
			string str(20,'c');
			int j = 1;
			while(argv[i][j])
			{
				str[j-1] = argv[i][j];
				j++;
			}
			str.erase(j-1);
			if(environmentVar.find(str) != environmentVar.end())
			{
				cout<<environmentVar[str]<<" ";
			}
			else
			{
				cout<<"Sorry,no such variable"<<" ";
			}
		}
		else
		{
			cout<<argv[i]<<" ";
		}
		i++;
	}
	cout<<endl;
}