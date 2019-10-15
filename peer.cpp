#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <string>
#include <list>
#include <cmath>
#include <vector>
#include <iostream>
#include<pthread.h>
#include<bits/stdc++.h>
using namespace std;

int CURRENTPORT;
int TRACKERPORT;
string uid;//set during login
string grpowner;//set during create group
string join_grp_request;//set during join request
pthread_mutex_t cs;
int server_fd;
struct sockaddr_in address;
struct sockaddr_in serv_addr;
int addrlen = sizeof(address); 
char buffer[1024] = {0};

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
void* functionality(void* ptr)
{
    int socket=*((int *)ptr);
    char buff[1024];
    recv(socket,buff,1024,0);//uid of requesting peer
    string uid=buff;
    cout<<uid<<" wants to join the group(Y/N)";
    char *buff1="yes";
    char *buff2="no";
    int decision;
    cout<<"waiting for input"<<endl;
   // scanf("%d",&decision);
    //cout<<"this is decision" << decision<<endl;
    send(socket,buff1,1024,0);
    //cout<<"status of success:"<<send(socket,buff2,1024,0)<<flush;  
    
    pthread_exit(NULL);
    return NULL;

}
void* listeningmode(void* ptr)
{
	//cout<<"Acting as server as well"<<endl;
	int socket= *((int*)ptr);
	if (listen(socket, 100) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	pthread_t thread;
	int new_socket;
	while(1)
	{
	if ((new_socket = accept(socket, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	} 
	int pt = pthread_create(&thread, NULL, functionality, (void *)&new_socket);   
	if(pt!=0) {
		cout << "Thread could not be created";
	} 
	
	}
}
void registration(int sock,char buffer[1024])
{
		send(sock,buffer,1024,0);//sending string
    	char buffer1[1024];
    	recv(sock,buffer1,1024,0);
    	//cout<<buffer1<<endl<<flush;
        close(sock);
}

void login(int sock,char buffer[1024])
{
		//cout<<"inside login"<<endl;
		//cout<<buffer<<endl;
		send(sock,buffer,1024,0);//sending string
    	char buffer1[1024];
    	recv(sock,buffer1,1024,0);
    	string result=buffer1;
    	if(result=="invlaid credentials" || result=="uid not found" )
    	{
    		exit(0);
    	}
    	else
    	{
    		uid=result;
    	}
    	close(sock);
}
void creategroup(int sock,char buffer[1024])
{
	send(sock,buffer,1024,0);
	char buff[1024];
	recv(sock,buff,1024,0);
	//cout<<buff<<endl;
	string str=buff;//gid received
	if (str!="")
	{
		grpowner=str;
		cout<<"group created successfully";
	}
    close(sock);

}
void joingroup(int sock,char buffer[1024])
{
    cout<<"inside joingroup function"<<endl;
    send(sock,buffer,1024,0);//to tracker
    char buff[1024];
    recv(sock,buff,1024,0);// port of owner received from tracker
    string portno=buff;
    int port=convert(portno); //string to int
    close(sock);// connection with tracker is closed
    /************* NEW CONNECTION WITH CLIENT****************/
    cout<<"trying to connect with client at port:"<<port<<endl;
    int socket1 = 0; 
    //struct sockaddr_in serv_addr;  
    if ((socket1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        
    } 
    
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port);// port of client to which we wanna connect(got from tracker)
     
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        
    } 
    if (connect(socket1, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        cout<<strerror(errno);  
        printf("\nConnection Failed \n"); 
        
    } 
    char uidbuff[1024];
    for (int i = 0; uid[i]!='\0'; ++i)
    {
        uidbuff[i]=uid[i];
    }
    cout<<"1"<<endl;
    send(socket1,uidbuff,1024,0);//sending uid of requesting peer to owner
    cout<<"2"<<endl;
    char responsebuff[4];// Yes or No from peer
    recv(socket1,responsebuff,1024,0);
    cout<<"3"<<endl;
    string response=responsebuff;
    cout<<"response: "<<response<<endl;
    close(socket1);
    /************* NEW CONNECTION WITH TRACKER****************/
    int socket2 = 0; 
    //struct sockaddr_in serv_addr;  
    if ((socket2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
         
    } 
    
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(TRACKERPORT);// port of the Tracker
     
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        
    } 
    if (connect(socket2, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        cout<<strerror(errno);  
        printf("\nConnection Failed \n"); 
         
    }
    if (response=="yes")
     {
        char updatebuffer[1024];
        string str="pr/";
        for (int i=0; uid[i]!='\0'; ++i)
        {
            str=str+uid[i];
        }
        str=str+'/';
        for (int j = 0;join_grp_request[j]!='\0'; ++j)
        {
            str=str+join_grp_request[j];
        }
        str=str+"#";
        for (int i = 0; str[i]!='\0'; ++i)
        {
            updatebuffer[i]=str[i];
        }
        cout<<updatebuffer<<endl;
        send(socket2,updatebuffer,1024,0); //uid and gid sent to tracker for group details
        close(socket2);
     } 
    else
    {
        cout<<"your request has been declined"<<endl;
    }

}
void list_groups(int sock, char buffer[1024])
{
	
	send(sock,buffer,1024,0);
	char buff2[1024];
	recv(sock,buff2,1024,0);
	string str="";
	for (int i = 0; buff2[i]!='#'; ++i)
	{
		str=str+buff2[i];
	}
	stringstream s(str);
	string temp;
	vector<string> vec;
	while(getline(s,temp,'/'))
	{
		vec.push_back(temp);
	}
	auto itr=vec.begin();
	for (; itr!=vec.end(); ++itr)
	{
		cout<<*itr<<" ";
	}
	cout<<endl;
    close(sock);
}

void downloadfun(int sock, char buffer[1024])
{
	send(sock,buffer,1024,0);
	int file_size;
	recv(sock,&file_size,sizeof(file_size),0);
	FILE *fp = fopen ("t.pdf","wb");
	char Buffer [1024];
	int n;
	while ((n = recv(sock,Buffer,1024,0))>0  &&  file_size>0){
	fwrite (Buffer,sizeof(char),n,fp);
	file_size = file_size - n;
} 

}


int main(int argc, char const *argv[]) 
{ 
  	
	CURRENTPORT=convert(argv[1]);
    TRACKERPORT=convert(argv[2]);
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) //SOCKET
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons(CURRENTPORT); //binding to current port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)//BIND 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}
	pthread_t t;
	pthread_create(&t, NULL, listeningmode, (void *)&server_fd); // LISTEN

	/*********************WARNING CONNECT AHEAD**************************/   
    while(1)
	{

	cout<<"trying to connect to tracker or other Clients"<<endl;
    cout<<"-> ";    
    bool flag=false;
	int sock = 0, valread; 
    struct sockaddr_in serv_addr;  
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Sock as a socket to connect
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(TRACKERPORT); //connecting on tracker port    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
    

		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
  		{ 
  			cout<<strerror(errno);	
      	  printf("\nConnection Failed \n"); 
      	  return -1; 
    	} 
		char buffer[1024] = {0}; 
		string query;
    	getline(cin,query);
        cout << "this is query : " << query << endl;
    	stringstream s(query);
    	string temp;
    	vector<string> vec;
    	while(getline(s,temp,' '))
    	{
    		vec.push_back(temp);
    	}
    	string str="";
    	auto itr=vec.begin();
    	for (; itr!=vec.end()-1; ++itr)
    	{
    		str = str+(*itr);
    		str= str + '/';
    	}
    	str=str+(*itr)+"#"; //complete string ending with #
    	//cout << str << endl;
    	int index=0;
    	for (int i = 0;str[i]!='\0'; ++i)
    	{
    		buffer[index]=str[i];
    		index++;
    	}
    	//Tokenizer and string formation ends here
    	if (vec[0]=="create_user")
    	{
    		registration(sock,buffer);
    	}
    	else if (vec[0]=="login")
    	{
    		login(sock,buffer);
            
    	}
    	else if(vec[0]=="create_group")
    	{	
    		if(grpowner=="")
    		{
    		char groupbuffer[1024];
    		string str="";
    		int i=0;
    		while(buffer[i]!='#'){
    			str=str+buffer[i];
    			i++;
    		}
    		str=str+"/";
    		int j=0;
    		while(uid[j]!='\0')
    		{
    			str=str+uid[j];
    			j++;
    		}
    		str=str+"#";
    		for (int i = 0; str[i]!='\0'; ++i)
    		{
    			groupbuffer[i]=str[i];
    		}
    		creategroup(sock,groupbuffer);//appending uid to gid
    	}
    	else
    	{
    		cout<<"you are already an owner"<<endl;
    	}
    	}
    	else if (vec[0]=="list_groups")
    	{
    		list_groups(sock,buffer);
    	}
        else if(vec[0]=="join_group")
        {
            string str=buffer;
            stringstream s(str);
            string temp;
            vector<string> vec;
            while(getline(s,temp,'/'))
            {
                vec.push_back(temp);
            }
            join_grp_request=vec[1];
            joingroup(sock,buffer);
            
		}
		else if (vec[0]=="download")
		{
			downloadfun(sock,buffer);
		}
    	
	} 
    return 0; 
} 

