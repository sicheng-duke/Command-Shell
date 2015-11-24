#include <iostream>
#include <stdio.h>  
#include <stdlib.h>  
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include "split.h"
#include "find.h"
#include "dir.h"

extern char **environ; 
char *convert(const string & s)
{
   char *pc = new char[s.size()+1];
   std::strcpy(pc, s.c_str());
   return pc; 
}

int main()
{
	stack<string> paths;
	string env = getenv("PATH");
	vector<string> env_vec;
	char colon = ':';
	splitBySymbol(env,env_vec,colon);
	while(1)
	{
		//current path
		char cwd[PATH_MAX+1];
		getcwd(cwd,PATH_MAX+1);

		//output the shell
		cout<<"myShell:"<<cwd<<" $ ";
		string input;
		bool findCommand = false;
		bool absolute = false;
		//get the input 
		getline(cin,input);
		vector<string> args_vec;
		splitBySpace(input,args_vec);
		if(cin.eof())
		{
			cout<<endl;
			return EXIT_SUCCESS;
		}
		char *cmd = NULL;
		char *arg = NULL;
		char **args = NULL;
		if(args_vec.size() == 0)
		{
			continue;
		}

		if(changeDir(args_vec[0],args_vec[1],paths))
		{
			continue;
		}
		//set the args
		//if the command is not relative w
		//put the first secton of the command into arg
		if(args_vec[0].find("./") == string::npos)
		{
			args = new char*[args_vec.size()+1];
			for(int i = 1 ; i < args_vec.size();i++)
			{
				char *temp = new char[args_vec[i].length()+1];
				strcpy(temp, args_vec[i].c_str());
				args[i] = temp;
			}
			args[args_vec.size()] = NULL;			
		}
		//if the command is relative
		//else don't need to put the first section into arg
		else
		{
			args = new char*[args_vec.size()];
			for(int i = 1 ; i < args_vec.size();i++)
			{
				char *temp = new char[args_vec[i].length()+1];
				strcpy(temp, args_vec[i].c_str());
				args[i-1] = temp;
			}
			args[args_vec.size()-1] = NULL;
		}
	
		//dealing with command in the PATH environment
		if(args_vec[0].find_first_of('/') == string::npos)
		{
				
				for(int i = 0; i < env_vec.size(); i++)
				{
					if(findFile(args_vec[0],env_vec[i]))
					{
						string command = env_vec[i]+'/'+args_vec[0];
						cmd = new char[command.length() + 1];
						strcpy(cmd, command.c_str());
						arg =  new char[args_vec[0].length() + 1];
						strcpy(arg, args_vec[0].c_str());
           				args[0] = arg;
           				findCommand = true;
           				break;
					
					}
				}
		}
		else
		{
				cmd = new char[args_vec[0].length()+1];
				strcpy(cmd, args_vec[0].c_str());
				//deal command has absolute path
				if(args_vec[0].find("./") == string::npos)
				{
					int last = args_vec[0].find_last_of("/");
					arg =  new char[args_vec[0].length()-last];
					substring(args_vec[0].c_str(),last+1,args_vec[0].length(),arg);
           			args[0] = arg;
           			cout<<arg;
					absolute = true;
				}
		}	

		pid_t child_pid = fork();	
		if(child_pid >= 0)
		{	
			//if it is child process
	 		if(child_pid == 0)
			{
				if(input.find_first_of('/') == string::npos)
				{
					if(findCommand)
					{
						//find command in PATH
						return execve(cmd,args,environ);
					}
					else
					{
						if(args_vec[0] != "exit" && !cin.eof())
						{
							cout<<"Command "<<input<<" not found"<<endl;
						}
						return -1;
					}
				}
				else
				{
					//absolute path?	
					if(absolute)
					{
						return execve(cmd,args,environ);
					}
					//relative path
					else
					{
						return execve(cmd,args,environ);
					}

				}				
			}
			//parent process
			else
			{
				int status;
				waitpid(child_pid,&status,0);
				if(args_vec[0]== "exit")
				{
					delete cmd;
					int i = 0;
					while(args[i] != NULL)
					{
						delete args[i];
						i++;
					}
					delete [] args;	
					break;
				}
				else
				{
					if(WIFSIGNALED(status))
					{
      					cout<<"Program was killed by signal "<<WTERMSIG(status)<<endl;
    				}
    				else
    				{
      					cout<<"Program exited with status "<<WEXITSTATUS(status)<<endl;
    				}

				}	
			}		
	 	}
	 	else
	 	{
	 		perror("fork failed");
	 	}

	}

	return 0;
}
