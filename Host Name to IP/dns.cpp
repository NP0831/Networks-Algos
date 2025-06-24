#include<bits/stdc++.h>
#include<string.h>	//strlen
#include<stdlib.h>	//malloc
#include<sys/socket.h>	//you know what this is for
#include<arpa/inet.h>	//inet_addr , inet_ntoa , ntohs etc
#include<netinet/in.h>
#include<unistd.h>	//getpid
using namespace std;

u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
	unsigned char *name;
	unsigned int p=0,jumped=0,offset;
	int i , j;

	*count = 1;
	name = (unsigned char*)malloc(256);

	name[0]='\0';

	//read the names in 3www6google3com format
	while(*reader!=0)
	{
		if(*reader>=192)
		{
			offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
			reader = buffer + offset - 1;
			jumped = 1; //we have jumped to another location so counting wont go up!
		}
		else
		{
			name[p++]=*reader;
		}

		reader = reader+1;

		if(jumped==0)
		{
			*count = *count + 1; //if we havent jumped to another location then we can count up
		}
	}
	name[p]='\0'; //string complete
	if(jumped==1)
	{
		*count = *count + 1; 
	}
	//now convert 3www6google3com0 to www.google.com
	for(i=0;i<(int)strlen((const char*)name);i++) 
	{
		p=name[i];
		for(j=0;j<(int)p;j++) 
		{
			name[i]=name[i+1];
			i=i+1;
		}
		name[i]='.';
	}
	name[i-1]='\0'; //remove the last dot
	return name;
}


void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host) 
{
	int lock = 0 , i;
	strcat((char*)host,".");
	
	for(i = 0 ; i < strlen((char*)host) ; i++) 
	{
		if(host[i]=='.') 
		{
			*dns++ = i-lock;
			for(;lock<i;lock++) 
			{
				*dns++=host[lock];
			}
			lock++; //or lock=i+1;
		}
	}
	*dns++='\0';
}
//DNS header structure
struct DNS_HEADER
{
	unsigned short id; // identification number

	unsigned char rd :1; // recursion desired
	unsigned char tc :1; // truncated message
	unsigned char aa :1; // authoritive answer
	unsigned char opcode :4; // purpose of message
	unsigned char qr :1; // query/response flag

	unsigned char rcode :4; // response code
	unsigned char cd :1; // checking disabled
	unsigned char ad :1; // authenticated data
	unsigned char z :1; // its z! reserved
	unsigned char ra :1; // recursion available

	unsigned short q_count; // number of question entries
	unsigned short ans_count; // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure
struct QUESTION
{
	unsigned short qtype;
	unsigned short qclass;
};

//Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};
#pragma pack(pop)

//Pointers to resource record contents
struct RES_RECORD
{
	unsigned char *name;
	struct R_DATA *resource;
	unsigned char *rdata;
};

//Structure of a Query
typedef struct
{
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;

int main( int argc , char *argv[])
{
	
	map<string,vector<string>> mp;
	//Get the hostname from the terminal
	while(1)
	{
		unsigned char host[100];
		cout<<"Enter the hostname: ";
		cin>>host;
		string x(reinterpret_cast<char*>(host));
        if(x=="exit")
        {
            cout<<"Leaving...\n";
            break;
        }
		if(mp.find(x)!=mp.end())
		{
			vector<string>v = mp[x];
			cout<<"Ip address is present in cache"<<endl;
			cout<<"IP addresses are"<<endl;
			for(auto x:v)
			{
				cout<<x<<endl;
			}
			continue;
		}
		unsigned char buf[65536],*qname,*reader;
		int i , j , stop , s;

		struct sockaddr_in a;
		struct RES_RECORD answers[20],auth[20],addit[20]; //the replies from the DNS server
		struct sockaddr_in dest;
		struct DNS_HEADER *dns = NULL;
		struct QUESTION *qinfo = NULL;

		s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries
		if(s==-1)
		{
			cerr<<"Socket creation failed"<<endl;	//Canonical name for an alias
			continue ;
		}
		cout<<"Socket created sucessfully"<<endl;

		dest.sin_family = AF_INET;
		dest.sin_port = htons(53);
		dest.sin_addr.s_addr = inet_addr("172.17.1.1"); //dns servers

		//Set the DNS structure to standard queries
		dns = (struct DNS_HEADER *)&buf;

		dns->id = (unsigned short) htons(getpid());
		dns->qr = 0; //This is a query
		dns->opcode = 0; //This is a standard query
		dns->aa = 0; //Not Authoritative
		dns->tc = 0; //This message is not truncated
		dns->rd = 1; //Recursion Desired
		dns->ra = 0; //Recursion not available! hey we dont have it (lol)
		dns->z = 0;
		dns->ad = 0;
		dns->cd = 0;
		dns->rcode = 0;
		dns->q_count = htons(1); //we have only 1 question
		dns->ans_count = 0;
		dns->auth_count = 0;
		dns->add_count = 0;

		//point to the query portion
		qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];

		ChangetoDnsNameFormat(qname , host);
		qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //fill it

		qinfo->qtype = htons(1); 
		qinfo->qclass = htons(1);

		int attempts = 3; // Number of retry attempts
		int attempt = 0;
		int received = 0;

		while (attempt < attempts && !received) {
			printf("\nSending Packet (Attempt %d)...", attempt + 1);
			if (sendto(s, (char*)buf, sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1) + sizeof(struct QUESTION), 0, (struct sockaddr*)&dest, sizeof(dest)) < 0) {
				cerr<<"sendto failed";
			}

			struct timeval timeout;
			timeout.tv_sec = 5;  // 5 seconds timeout (adjust as needed)
			timeout.tv_usec = 0;

			fd_set readfds;
			FD_ZERO(&readfds);
			FD_SET(s, &readfds);

			// Wait for a response or timeout
			int ready = select(s + 1, &readfds, NULL, NULL, &timeout);

			if (ready > 0) {
				received = 1; // Response received
			} else if (ready == 0) {
				// Timeout, increment the attempt count
				attempt++;
				cout<<"Timeout (Attempt "<<attempt+1<<") - Retrying..."<<endl;
			} else {
				cout<<"select failed";
				break;
			}
		}

		if (!received) {
			cout<<"No response received after" <<attempts<< "attempts.\n";
			exit(1);
		}
		
		//Receive the answer
		i = sizeof dest;
		cout<<"Receiving answer..."<<endl;
		if(recvfrom (s,(char*)buf , 65536 , 0 , (struct sockaddr*)&dest , (socklen_t*)&i ) < 0)
		{
			cerr<<"recvfrom failed"<<endl;
		}
		cout<<"Recieved sucessfully"<<endl;

		dns = (struct DNS_HEADER*) buf;

		//move ahead of the dns header and the query field
		reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];


		//Start reading answers
		stop=0;

		for(i=0;i<ntohs(dns->ans_count);i++)
		{
			answers[i].name=ReadName(reader,buf,&stop);
			reader = reader + stop;

			answers[i].resource = (struct R_DATA*)(reader);
			reader = reader + sizeof(struct R_DATA);

			if(ntohs(answers[i].resource->type) == 1) //if its an ipv4 address
			{
				answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));
				for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
				{
					answers[i].rdata[j]=reader[j];
				}
				answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';
				reader = reader + ntohs(answers[i].resource->data_len);
			}
			else
			{
				answers[i].rdata = ReadName(reader,buf,&stop);
				reader = reader + stop;
			}
		}

		vector<string> ans;

		printf("\nAnswer Records : %d \n" , ntohs(dns->ans_count) );
		for(i=0 ; i < ntohs(dns->ans_count) ; i++)
		{
			cout<<"Name : "<<answers[i].name<< " ";

			if( ntohs(answers[i].resource->type) == 1) //IPv4 address
			{
				long *p;
				p=(long*)answers[i].rdata;
				a.sin_addr.s_addr=(*p); //working without ntohl
				string st= inet_ntoa(a.sin_addr);
				cout<<st<<endl;
				ans.push_back(st);
			}
			
			if(ntohs(answers[i].resource->type)==5) 
			{				
				cout<<"has alias name : "<<answers[i].rdata<<endl;
			}
		}
		mp[x]=ans;
        cout<<"\n";
	}

	return 0;
}

