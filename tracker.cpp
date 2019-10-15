#include <unistd.h> 
#include <stdio.h> 
#include<iostream>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>   
#include <pthread.h>
#include <bits/stdc++.h> 
using namespace std;
map <string,string> details;//uid password
map <string,pair <string,string> > identification;// (uid,<ip,port>)
map<string,string>ownership; // gid|uid
map<string,vector<string> > groupinfo;

void init()
{
	details.insert({"1","t"});
	details.insert({"2","g"});
	pair<string,string> p;
	p.first="1";
	p.second="8081";
	identification.insert({"1",p});
	p.first="2";
	p.second="8082";
	identification.insert({"1",p});

}

void registration(vector<string> &vec,int server_fd)
{
	string uid=vec[1];
    string password=vec[2];
    string IP= vec[3];
    string port=vec[4];
    //cout<<uid<<" "<<password<<" "<<IP<<" "<<port<<" "<<endl<<flush;
   	details.insert({uid,password});
   	pair<string,string> p;
   	p.first=IP;
   	p.second=port;
    identification.insert({uid,{IP,port}});
	char* buffer="user created";
	send(server_fd,buffer,1024,0);

}

void login(vector<string>&vec,int server_fd)
{
	//cout<<"inside login"<<endl;
	string uid=vec[1];
	string password=vec[2];
	char* buffer="uid not found";
	char buffer1[1024];
	strcpy(buffer1,uid.c_str());
	char*buffer2="invlaid credentials";
	auto itr=details.find(uid);
	if (itr==details.end())// uid not present
	{
		send(server_fd,buffer2,1024,0);
	}
	else if (itr!=details.end())//record found in database
	{
		if (itr->second==password) // if correct password
		{
			send(server_fd,buffer1,1024,0);//sending back uid
		}
		else //invalid credentials
		{
			send(server_fd,buffer2,1024,0);
		}

	}
}
void create_group(vector<string>&vec,int server_fd)
{
	string uid=vec[2];
	string gid=vec[1];
	ownership.insert({gid,uid});//updating ownertable
	vector<string> temp;
	temp.push_back(uid);
	groupinfo[gid]=temp;//updating groupinfo
	char buff[1024];
	for (int i = 0; gid[i]!='\0'; ++i)
	{
		buff[i]=gid[i];
	}
	send(server_fd,buff,1024,0);
}

/************************************************/
void list_grp(vector<string>&vec,int server_fd)
{
	string temp="";
	auto itr=ownership.begin();
	for (; itr!=ownership.end(); ++itr)
	{
		string str=itr->first;
		for (int i = 0; str[i]!='\0'; ++i)
		{
			temp=temp+str[i];
		}
		temp=temp+"/";
	}
	temp=temp+"#";
	char buff[1024];
	for (int i = 0; temp[i]!='\0'; ++i)
	{
		buff[i]=temp[i];
	}
	send(server_fd,buff,1024,0);

}
void joingroup(vector<string>&vec,int server_fd)
{
	string gid=vec[1];
	string uid;
	string port;
	auto itr=ownership.find(gid);//finding group owner
	uid=itr->second;//getting uid of owner gid|uid
	auto itr1=identification.find(uid);
	port=itr1->second.second;//getting port of owner
	char buff[1024];
	for (int i = 0; port[i]!='\0' ; ++i)
	{
		buff[i]=port[i];
	}
	send(server_fd,buff,1024,0);// port sent to peer
}

void updategroupinfo(vector<string>&vec,int server_fd)
{
	string uid=vec[1];
	string gid=vec[2];
	vector<string> temp;
	temp.push_back(uid);
	groupinfo[gid]=temp;

}
void sendfile(vector<string>&vec,int server_fd)
{
	FILE *fp = fopen ( "/home/tushar/Desktop/Tushar.pdf"  , "rb" );
	fseek (fp,0,SEEK_END);
  	int fsize = ftell (fp);
  	rewind (fp);
  	send (server_fd ,&fsize,sizeof(fsize),0);
	char Buffer [1024]; 
	int n;
	while ((n=fread(Buffer,sizeof(char),1024,fp) ) > 0  && fsize>0 ){
		send (server_fd,Buffer,n,0);
		fsize = fsize - n ;
}

fclose ( fp );
}
void *myThreadFun(void *vargp) 
{
    char buffer[1024];
    int *temp = (int *)vargp;
    int server_fd = *temp;
    recv( server_fd , buffer, 1024,0); //reading string sent to tracker
    //cout<<buffer<<endl;
    string str="";
    for(int i=0;buffer[i]!='#';i++)
    {
    	str=str+buffer[i];
    }
   // cout<<"string in controller: "<<str<<endl;
    string temp1;
    stringstream s(str);
    vector <string> vec;
    while(getline(s,temp1,'/'))
    {
    	vec.push_back(temp1);	//
    }
    if(vec[0]=="create_user")
    {
    	registration(vec,server_fd);
    }
    else if (vec[0]=="login")
    {
    	//cout<<"call invoked"<<endl;
    	login(vec,server_fd);
    }
    else if(vec[0]=="create_group")
    {
    	create_group(vec,server_fd);
    }
    else if (vec[0]=="list_groups")
    {
    	list_grp(vec,server_fd);
    }
    else if(vec[0]=="join_group")
    {
    	joingroup(vec,server_fd);
    }
    else if (vec[0]=="pr")
    {
    	updategroupinfo(vec,server_fd);
    }
    else if (vec[0]=="download")
    {
    	sendfile(vec,server_fd);
    }
    close(server_fd);
    pthread_exit(NULL);
    *temp=-1;
    return NULL;
}
int convert(string str)
{
	int ans=0;
	int i=0;
	while(str[i]!='\0')
	{
		ans=ans*10+str[i]-'0';
		i++;
	}
	return ans;
}
int main(int argc, char *argv[]) 
{ 
	init();
	cout<<"-> ";
	int server_fd, new_socket;// serverFD and acceptFD 
	struct sockaddr_in address;
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char *hello = "Hello from server"; 
	pthread_t thread;
	int port=convert(argv[1]);
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) //SOCKET
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(port); 
	memset(address.sin_zero, '\0', sizeof address.sin_zero);
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 100) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	int i=0;
	while(1)
	{
		//cout<<"waiting"<<endl<<flush;
		addrlen= sizeof(address);
		//cout<<"before socket"<<endl<<flush;
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0) 
	{ 
		
		perror("accept"); 
		exit(EXIT_FAILURE); 
	} 
	cout<<strerror(errno)<<endl;
	//cout<<"after socket"<<endl<<flush;
	//cout<<new_socket<<endl<<flush;
	int pt = pthread_create(&thread, NULL, myThreadFun, (void *)&new_socket);   
	if(pt!=0) {
		cout << "Thread could not be created";
	} 
	 
	}//while loop
          
	return 0; 
} 


