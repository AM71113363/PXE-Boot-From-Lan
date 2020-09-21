#include "main.h"

#define DHCP_PORT_RECV    67
#define DHCP_PORT_SEND    68


char yiaddr[16];
char siaddr[16];
unsigned char data[1024];

int type;
SOCKET  DHCPSock;
struct sockaddr_in serverDHCP;
struct sockaddr_in clientDHCP;


#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPACK       5

#define DHO_PAD                         0
#define DHO_SUBNET_MASK                 1
#define DHO_ROUTERS                     3
#define DHO_DOMAIN_NAME_SERVERS         6
#define DHO_HOST_NAME                   12
#define DHO_BOOT_SIZE                   13
#define DHO_NETBIOS_NAME_SERVERS        44
#define DHO_DHCP_LEASE_TIME             51
#define DHO_DHCP_MESSAGE_TYPE           53
#define DHO_DHCP_SERVER_IDENTIFIER      54
#define DHO_DHCP_RENEWAL_TIME           58
#define DHO_DHCP_REBINDING_TIME         59
#define DHO_DHCP_CLIENT_IDENTIFIER      61
#define DHO_END                        255
///                        Ethernet header + IP header + UDP header
#define DHCP_UDP_OVERHEAD   (14 + 20 + 8)
                              
#define DHCP_SNAME_LEN        64
#define DHCP_FILE_LEN        128
#define DHCP_FIXED_NON_UDP   236
#define DHCP_FIXED_LEN      (DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
#define DHCP_MTU_MAX        1500
#define DHCP_OPTION_LEN     (DHCP_MTU_MAX - DHCP_FIXED_LEN)

#define BOOTREQUEST 1
#define BOOTREPLY   2

#define DHCP_OPTIONS_COOKIE    "\143\202\123\143"
#define IsDHCP(x)    ( * (DWORD *) ((x).options) == * (DWORD *) DHCP_OPTIONS_COOKIE )
#define  SizeOfTab(x)   (sizeof (x) / sizeof (x[0]))
#define  MakeMask(x)    ( 1 << (x) )

typedef struct dhcp_packet
{
      unsigned char  op;                 /* 0: Message opcode/type */
      unsigned char  htype;              /* 1: Hardware addr type (net/if_types.h) */
      unsigned char  hlen;               /* 2: Hardware addr length */
      unsigned char  hops;               /* 3: Number of relay agent hops from client */
      unsigned long xid;                /* 4: Transaction ID */
      unsigned short secs;               /* 8: Seconds since client started looking */
      unsigned short flags;              /* 10: Flag bits */
      struct in_addr ciaddr;        /* 12: Client IP address (if already in use) */
      struct in_addr yiaddr;        /* 16: Client IP address */
      struct in_addr siaddr;        /* 18: IP address of next server to talk to */
      struct in_addr giaddr;        /* 20: DHCP relay agent IP address */
      unsigned char chaddr [16];    /* 24: Client hardware address */
      char sname [DHCP_SNAME_LEN];  /* 40: Server name */
      char file [DHCP_FILE_LEN];    /* 104: Boot filename */
      unsigned char options [DHCP_OPTION_LEN];  /* 212: Optional parameters       (actual length dependent on MTU). */
}DHCP;


struct S_DhcpOptions
{
   unsigned nDHCPOpt;
   char     nLen;
};
                   
static struct S_DhcpOptions sDhcpOpt [] =       // 0 for unspecified
{
	{DHO_DHCP_MESSAGE_TYPE,      1},
	{DHO_DHCP_SERVER_IDENTIFIER, 4},
	{DHO_SUBNET_MASK,            4},
	{DHO_ROUTERS,                4},
	{DHO_DOMAIN_NAME_SERVERS,    4},
	{DHO_NETBIOS_NAME_SERVERS,   4},
	{DHO_DHCP_LEASE_TIME,        4},
	{DHO_DHCP_RENEWAL_TIME,      4},
	{DHO_DHCP_REBINDING_TIME,    4},
	{DHO_BOOT_SIZE,              0},
	{DHO_END,                    0}};


// DHCP Management
// Search an option in the DHCP extension
char *DHCPSearchOptionsField (unsigned char *pOpt, int nField, int *pLength)
{
     int Ark;
     unsigned char *p;
     p = pOpt + (sizeof(DHCP_OPTIONS_COOKIE) - 1);
     
     for(Ark=0; Ark<DHCP_OPTION_LEN-3  && p[Ark]!=nField ;Ark += (p[Ark]==DHO_PAD ? 1 : 2+p[Ark+1]));

     if(Ark<DHCP_OPTION_LEN-3  &&  Ark+p[Ark+1] < DHCP_OPTION_LEN && p[Ark] == nField)
     {
        if(pLength!=NULL)  *pLength = p[Ark+1];
        return &p[Ark+2];
     }
return NULL;
} // DHCPSearchField

int DHCPOptionsReply(struct dhcp_packet  *dhcp, int nDhcpType)
{
    unsigned char *pOpt = (unsigned char *)(dhcp->options + (sizeof(DHCP_OPTIONS_COOKIE) - 1));
    HANDLE hFile;

	int Ark;
	DWORD nLease = (2*24*60);

    for (Ark=0 ; Ark<SizeOfTab(sDhcpOpt) ; Ark++)
    {
	  
     if (sDhcpOpt[Ark].nLen!=0) 
     {
        *pOpt++ = (unsigned char) sDhcpOpt[Ark].nDHCPOpt ; 
        *pOpt++ = (unsigned char) sDhcpOpt[Ark].nLen;
     }
     switch (sDhcpOpt[Ark].nDHCPOpt)
     {
		case DHO_DHCP_MESSAGE_TYPE       :{  * pOpt = (unsigned char) nDhcpType ; }break; 
		case DHO_DHCP_SERVER_IDENTIFIER  :{  * (DWORD *) pOpt =  inet_addr(siaddr); }break;

		case DHO_SUBNET_MASK             :{  * (DWORD *) pOpt = inet_addr("255.255.255.0"); }break;
		case DHO_ROUTERS                 :{  * (DWORD *) pOpt = inet_addr("0.0.0.0");  }break;
		case DHO_DOMAIN_NAME_SERVERS     :{  * (DWORD *) pOpt = inet_addr("0.0.0.0");  }break; 
		case DHO_NETBIOS_NAME_SERVERS    :{  * (DWORD *) pOpt = inet_addr("0.0.0.0");  }break;

		case DHO_DHCP_LEASE_TIME         :{  * (DWORD *) pOpt = htonl (nLease * 60);  }break;
		case DHO_DHCP_RENEWAL_TIME       :{  * (DWORD *) pOpt = htonl (nLease/2 * 60); } break;
		case DHO_DHCP_REBINDING_TIME     :{  * (DWORD *) pOpt = htonl ((nLease*80)/100 * 60); } break;
		case DHO_BOOT_SIZE:
	    {
            hFile = CreateFile(Path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                  *pOpt++ = DHO_BOOT_SIZE ;
                  *pOpt++ = sizeof (unsigned short);
                  * (unsigned short *) pOpt = htons ( (unsigned short) (1+GetFileSize (hFile, NULL) / 512) ) ;
                  pOpt += sizeof (unsigned short);
                  CloseHandle( hFile ) ;         // close the file
            }
        }
        break;
        case DHO_END:
        {
             *pOpt++ = DHO_END;
			 *pOpt++ = DHO_PAD;
			 *pOpt++ = DHO_PAD;
		}
         break;
     } // switch option
     pOpt += sDhcpOpt[Ark].nLen ;    // points on next field
   } // for all option

return (int) (pOpt - (unsigned char*) dhcp);
} // DHCPOptionsReply


int ProcessDHCPMessage (DHCP *dhcp, int *pSize)
{
    unsigned char *p=NULL;
    int nDhcpType = 0;
    if(IsDHCP (*dhcp))
    {
       // search DHCP message type
       p = DHCPSearchOptionsField (dhcp->options, DHO_DHCP_MESSAGE_TYPE, NULL);
       if (p!=NULL)        nDhcpType = *p;
    }
    if(dhcp->yiaddr.s_addr!=INADDR_ANY  &&  dhcp->yiaddr.s_addr!=INADDR_NONE )
            return FALSE ; // address already assigned
    
    //Always pack the magic cookie again, just in case it was corrupted
	*(DWORD*)(dhcp->options) = * (DWORD*) DHCP_OPTIONS_COOKIE;
    
    dhcp->yiaddr.s_addr = inet_addr(yiaddr);
    dhcp->siaddr.s_addr = inet_addr(siaddr);

    dhcp->op = BOOTREPLY;
            
    switch (nDhcpType)
    {
        case 0:
        case DHCPDISCOVER:
        {    MAKE_ON(ang1);
             type = 1;
             *pSize = DHCPOptionsReply(dhcp, DHCPOFFER);
        }
        break;
        case DHCPREQUEST :
        {    MAKE_ON(ang3);
             type = 2;
             *pSize = DHCPOptionsReply (dhcp, DHCPACK);
		 
        }break;
     }

// answer only to BootP, Request or discover msg
return  (nDhcpType==0 || nDhcpType==DHCPDISCOVER || nDhcpType==DHCPREQUEST);
}



void StartDHCP()
{
    sprintf(siaddr,"%d.%d.%d.%d\0", c[0], c[1], c[2], c[3]);
    sprintf(yiaddr,"%d.%d.%d.%d\0", c[0], c[1], c[2], c[3]+1);
        
    //create socket for both server and client, same socket different ports
    DHCPSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(DHCPSock==INVALID_SOCKET)
    { 
        MessageBox(hWnd, "Cant't init DHCP socket","Error",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
        return;
    }
    int ret;
    int True = 1;
    // share bindings for UDP sockets
	ret = setsockopt(DHCPSock, SOL_SOCKET, SO_REUSEADDR,(const char *)&True, sizeof(True));
    if(ret == -1)
    {
        ret = MessageBox(hWnd, "Cant't setsockopt[SO_REUSEADDR] DHCP\r\nExit now?","setsockopt",MB_YESNO | MB_ICONQUESTION	| MB_SYSTEMMODAL);
        if(ret == IDYES)
        {
            shutdown(DHCPSock,2);
            closesocket(DHCPSock);
            return;     
        }
    }
    // add broadcast permission to socket
	ret = setsockopt(DHCPSock, SOL_SOCKET, SO_BROADCAST,(const char *)&True, sizeof(True));
    if(ret == -1)
    {
        ret = MessageBox(hWnd, "Cant't setsockopt[SO_BROADCAST]\r\nExit now?","setsockopt",MB_YESNO | MB_ICONQUESTION	| MB_SYSTEMMODAL);
        if(ret == IDYES)
            return;     
    }
    //fill server structure for listening socket
    serverDHCP.sin_family=AF_INET;
	serverDHCP.sin_addr.s_addr=INADDR_ANY;  
	serverDHCP.sin_port=htons(DHCP_PORT_RECV); 
    //fill client structure for sending packets
    clientDHCP.sin_family = AF_INET;
	clientDHCP.sin_port = htons(DHCP_PORT_SEND);
	clientDHCP.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    //bind the socket
   	ret = bind(DHCPSock, (struct sockaddr*)&serverDHCP, sizeof(serverDHCP));
	if(ret!=0)
    { 
        MessageBox(hWnd, "Cant't bind DHCP socket","Error",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
        shutdown(DHCPSock,2);
        closesocket(DHCPSock);
        return; 
    }
    
    SOCKADDR_STORAGE    from;
    int from_len = sizeof(from);    
    DHCP *dhcp;
    int nSize;
    while(1)
    {//while 
        ret = recvfrom(DHCPSock,data, 1024,0,(struct sockaddr*)&from,&from_len);
   	    // if msg is too short
        if(ret < 236) //offsetof ( struct dhcp_packet, options ))
           continue;
        //do not accept domain server name cuz DNS not avaible
	    dhcp = (DHCP *)data;
        if(dhcp->sname[0]!=0)
           continue;
        // we have only to answer to BOOTREQUEST msg
        if(dhcp->op != BOOTREQUEST) 
           continue;
        //makeup
        //fill filename of bootloader    
        strcpy(dhcp->file,Path);
        nSize = 0;
        type = 0;
        if(ProcessDHCPMessage( dhcp, &nSize))
        {
            if(type == 1) MAKE_ON(ang2);
            if(type == 2) MAKE_ON(ang4);
            do{
               ret = sendto(DHCPSock,data, nSize,0,(struct sockaddr *)&clientDHCP,sizeof(clientDHCP));
            }while(ret < 1);
        }
        if(type == 2)
        {
           _sleep(2000);
           MAKE_OFF(ang1);      
           MAKE_OFF(ang2);      
           MAKE_OFF(ang3);      
           MAKE_OFF(ang4);      
                
        }
         
     
    }//while  
     
     
}





