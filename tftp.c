#include "main.h"

#define TFTP_PORT_ALL  69

typedef struct tPass_
{
     HWND hMain;   
     HWND hStat;   
     HWND hProg;
     UCHAR *buffer;
     struct sockaddr_in *client;
     DWORD id;
}TPASS;

static DWORD up = 20;
void first_loop();
void Go(LPVOID _a_);
void Worker(LPVOID _a_);
SOCKET TFTPSock;
struct sockaddr_in serverTFTP;
unsigned char buffer[1024];

#define TFTP_RRQ    1          /* read request */
#define TFTP_WRQ    2          /* write request */
#define TFTP_DATA   3          /* data packet */
#define TFTP_ACK    4          /* acknowledgement */
#define TFTP_ERROR  5          /* error code */
#define TFTP_OACK   6          /* option acknowledgement */

// ---- TFTP_ERROR CODES ----
#define TFTP_ERROR_NOT_DEFINED		        0 // Not defined, see error message
#define TFTP_ERROR_FILE_NOT_FOUND	    	1 // File not found
#define TFTP_ERROR_ACCESS_VIOLATION			2 // Access violation
#define TFTP_ERROR_DISK_FULL 		        3 // Disk full or allocation exceeded
#define TFTP_ERROR_ILLEGAL_OPERATION		    4 // Illegal TFTP operation
#define TFTP_ERROR_UNKNOWN_ID		        5 // Unknown transfer ID
#define TFTP_ERROR_FILE_EXIST 	            6 // File already exists
#define TFTP_ERROR_UNKNOWN_USER	            7 // No such user
 

void SMS(unsigned char *sms, unsigned char *sp)
{
     DWORD len=GetWindowTextLength(sss);
     if(len>0)  SendMessage(sss,EM_SETSEL,(WPARAM)len,(LPARAM)len);
     SendMessage(sss,EM_REPLACESEL,(WPARAM)0,(LPARAM)(unsigned char*)sms);
     SendMessage(sss,EM_REPLACESEL,(WPARAM)0,(LPARAM)(unsigned char*)sp);
     SendMessage(sss,WM_VSCROLL,(WPARAM)SB_ENDSCROLL,(LPARAM)0);
     return;
}


void CenterOn(HWND hnd)
{
     RECT rcClient, rcDesktop;
     SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
     GetWindowRect(hnd, &rcClient);
     int nX=((rcDesktop.right - rcDesktop.left) / 2) -((rcClient.right - rcClient.left) / 2);
     int nY=((rcDesktop.bottom - rcDesktop.top) / 2) -((rcClient.bottom - rcClient.top) / 2);
     if(up > nY) up = 20;
     SetWindowPos(hnd, NULL, nX, nY-up, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	 SetWindowPos(hnd,HWND_TOPMOST,0,0,0,0,SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);                   
}


BOOL CALLBACK DlgProc(HWND hnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
       case WM_INITDIALOG:
       {
            HWND _s_;
            up += 47;
            CenterOn(hnd);
            ((TPASS*)lParam)->hMain = hnd;
            _s_ = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE ,11,11, 274, 20, hnd,0, ins, NULL);                
            SNDMSG(_s_, WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
            ((TPASS*)lParam)->hStat = _s_;
            _s_ = CreateWindowEx(0,"msctls_progress32",NULL,WS_CHILD | WS_VISIBLE | WS_BORDER,
            0,36, 301, 11, hnd,0, ins, NULL); 
            SendMessage(_s_,PBM_SETBARCOLOR,(WPARAM)0, (LPARAM)0xAE2FF7);//AE2FF7
            ((TPASS*)lParam)->hProg = _s_;   
           CreateThread(0,0,(LPTHREAD_START_ROUTINE)Worker,(LPVOID)lParam,0,0);
         
         }			
       break;
       case WM_CLOSE:
           
           EndDialog(hnd,0); 
       break;
    }
    return FALSE;
}


void Go(LPVOID _a_)
{
    DialogBoxParam(0,MAKEINTRESOURCE(2222),0,DlgProc,(LPARAM)_a_);
}

          
DWORD Nstring(unsigned char *bf)
{
    unsigned char *p = bf;
    DWORD n = 1;
    while(*p++ != 0)  n++;
    return n; 
} 
 
void Worker(LPVOID _a_)
{
    TPASS *X = (TPASS*)_a_;
    HANDLE hFile = NULL;
    DWORD dwFileSize = 0;
    DWORD BufferSize;
    unsigned char *BufferToSend;
    __int64 step = 1;
    DWORD x = 0;
    DWORD y = 0;
    DWORD dwRead;
    int ret;
    int nSize;
      
    _sleep(500);
    sprintf(buffer,"Thread[%X] : ACTIVE\0", X->id);
    SMS(buffer,"\r\n");
    //new loop starts only for TFTP_RRQ

    unsigned char *FileName = "\0"; //FileName
    unsigned char *Mode     = "\0"; //octed
    unsigned char *BlkSize  = "\0"; //blksize | tsize 
    unsigned char *Count    = "\0"; //number 

    FileName = X->buffer + 2;   ret = Nstring(FileName);
    Mode = FileName + ret; ret = Nstring(Mode);
    BlkSize = Mode + ret;  ret = Nstring(BlkSize);
    Count = BlkSize + ret; 
    //traslate FileName
    for(ret =0; ret <strlen(FileName);ret++)
    {
        if(FileName[ret]=='/') FileName[ret]='\\';        
    }
    if(FileName[0]=='\\') FileName++;

    SMS(FileName," "); SMS(Mode," "); SMS(BlkSize," "); SMS(Count,"\r\n");
    //Mode check
    if( (strncmp(Mode,"octet",5) != 0) && (strncmp(Mode,"mail",4) != 0) && 
        (strncmp(Mode,"ascii",5) != 0) && (strncmp(Mode,"netascii",8) != 0) )
	{
        SMS("Send : ", "Mode not Accepted\r\n");
        memset(buffer, 0, 1024);
        buffer[1] = TFTP_ERROR;
        buffer[3] = TFTP_ERROR_ILLEGAL_OPERATION;       
        nSize = sprintf(&buffer[4],"%s","Illegal TFTP operation");
		do{
	           ret = sendto(TFTPSock, buffer, nSize+4+1,0,(struct sockaddr *)X->client,sizeof(*X->client));
		}while(ret < 0);
		free(X->buffer);
        free(X->client);
        EndDialog(X->hMain,0);
        DestroyWindow(X->hMain);
        free(X);
        SMS("THREAD : ","DEACTIVE\r\n");
        CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
        return;                        
    }
    //open File;
    hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if(hFile == INVALID_HANDLE_VALUE)//if file not found : send 05 code
    {
        SMS("Send : ","File not found\r\n");
        memset(buffer,0,1024);
		buffer[1] = TFTP_ERROR;
        buffer[3] = TFTP_ERROR_FILE_NOT_FOUND;       
        nSize = sprintf(&buffer[4],"%s","File not found");
		do{
		      ret = sendto(TFTPSock, buffer, nSize+4+1,0,(struct sockaddr *)X->client,sizeof(*X->client));
		}while(ret < 0);
		free(X->buffer);
        free(X->client);
        EndDialog(X->hMain,0);
        DestroyWindow(X->hMain);
        free(X);
        SMS("THREAD : ","DEACTIVE\r\n");
        CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
        return;                        
    }
	SetWindowText(X->hMain,FileName);
	dwFileSize = GetFileSize(hFile, NULL);
	sprintf(buffer,"FileSize : %d\0",dwFileSize);
	SetWindowText(X->hStat,buffer);
	
	//file ok
    //1: if just file size is required
    if(strncmp (BlkSize,"tsize",5) == 0)// want the size
    {
        SMS("Send : ", "filesize\r\n");
        memset(buffer, 0, 1024);
        CloseHandle(hFile);
        buffer[1] = TFTP_OACK;
        sprintf(&buffer[2],"%s","tsize");
        nSize = sprintf(&buffer[8],"%d\0",dwFileSize);
        do{
		  ret = sendto(TFTPSock, buffer, nSize+8+1,0,(struct sockaddr *)X->client,sizeof(*X->client));
		}while(ret < 0);
		free(X->buffer);
        free(X->client);
        
        EndDialog(X->hMain,0);
        DestroyWindow(X->hMain);
        free(X);
        SMS("THREAD : ","DEACTIVE\r\n");
        CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
        return;                        
    }
    //2: remains just blksize
    if(strncmp (BlkSize,"blksize",7) == 0)// if the Client want a custom blksize
    {
        DWORD val;
        val = atol(Count);
        if(val > 512)
        {
		    BufferSize = val;
        }
    }
    DWORD cnr = dwFileSize;

    sprintf(buffer,"%d [%d]\r\n\0",dwFileSize,(cnr/BufferSize)); SMS("FileSize : ",buffer);
        
    SendMessage(X->hProg, PBM_SETRANGE,(WPARAM)0,(LPARAM)MAKELPARAM(0,1000));
        
    BufferToSend = (unsigned char *)malloc(BufferSize + 5);
    memset( BufferToSend, 0, BufferSize + 5);
    BufferToSend[1] = TFTP_DATA;
    *((__int64 *)&BufferToSend[2]) = htons ((unsigned short)step) ;
    x = BufferToSend[2];
    y = BufferToSend[3]; 
    //read file
	if(!ReadFile(hFile,&BufferToSend[4],BufferSize, &dwRead, NULL))
    {
        SMS("Error: ", "ReadFile in step 1\r\n");
		CloseHandle(hFile);
		memset(buffer, 0, 1024);
        buffer[1] = TFTP_ERROR;
        buffer[3] = TFTP_ERROR_NOT_DEFINED;       
        nSize = sprintf(&buffer[4],"%s","Not defined, see error message");
		do{
		      ret = sendto(TFTPSock, buffer, nSize+4+1,0,(struct sockaddr *)X->client,sizeof(*X->client));
		}while(ret < 0);
		free(X->buffer);
        free(X->client);
        EndDialog(X->hMain,0);
        DestroyWindow(X->hMain);
        free(X);
        SMS("THREAD : ","DEACTIVE\r\n");
        CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
        return;                        
    }
    // send file
    do{ 
        ret = sendto(TFTPSock, BufferToSend, dwRead+4,0,(struct sockaddr *)X->client,sizeof(*X->client));
    }while(ret < 0);
    //after sending PACKET wait for the receive MSG
    unsigned char *Rdata = (unsigned char*)malloc(520);
    struct sockaddr_in *from;
    from = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    int from_len = sizeof(*from);
    __int64 pupu;    
    //loop until ends THREAD nr;
    while(1)
    {
        memset(Rdata, 0, 520);
        ret = recvfrom(TFTPSock,Rdata, 520,0,(struct sockaddr*)from,&from_len);
   	    if(ret < 1)
   	        continue;
   	    if(X->id != ntohs(from->sin_port))//if a new conn is made by the client
   	    {
            free(X->buffer);
            free(X->client); 
          //  memset(X, 0, sizeof(TPASS));
            CloseHandle(hFile);
            X->buffer = Rdata;
            X->client = from;
            X->id = ntohs(from->sin_port);
            EndDialog(X->hMain,0);
            DestroyWindow(X->hMain);
            SMS("THREAD : ","DEACTIVE\r\n");
            if(Rdata[1] == TFTP_RRQ)//if it's an request call again this
            {
                CreateThread(0,0,(LPTHREAD_START_ROUTINE)Worker,(LPVOID)X,0,0);
                sprintf(buffer,"Passing Go to Go[%X]\0", X->id);
                SMS(buffer,"\r\n");
                break;
            }
		    free(X->buffer);
            free(X->client);
            free(X);
            CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
            break;                        
        }
        if(Rdata[1] != TFTP_ACK) //even it's the same ID but Client doesn't send an ACK
        {
            SMS("RECV : ","Internal Client Error\r\n");
            free(X->buffer);
            free(X->client);
            CloseHandle(hFile);
            EndDialog(X->hMain,0);
            DestroyWindow(X->hMain);
            free(X);
            SMS("THREAD : ","DEACTIVE\r\n");
            CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
            break;                        
        }
        //it ACK PACKET
        if( (Rdata[2] == x) && (Rdata[3] == y) )
        { 
            step++;
            BufferToSend[1] = TFTP_DATA;
            *((__int64 *)&BufferToSend[2]) = htons ((unsigned short)step) ;
            x = BufferToSend[2];
            y = BufferToSend[3]; 
	        if(!ReadFile(hFile,&BufferToSend[4],BufferSize, &dwRead, NULL))
            {
                SMS("Error: ", "ReadFile in step 2\r\n");
		        CloseHandle(hFile);
		        memset(buffer, 0, 1024);
                buffer[1] = TFTP_ERROR;
                buffer[3] = TFTP_ERROR_NOT_DEFINED;       
                nSize = sprintf(&buffer[4],"%s","Not defined, see error message");
		        do{
		        ret = sendto(TFTPSock, buffer, nSize+4+1,0,(struct sockaddr *)X->client,sizeof(*X->client));
		        }while(ret < 0);
		        free(X->buffer);
                free(X->client);
                EndDialog(X->hMain,0);
                DestroyWindow(X->hMain);
                free(X);
                SMS("THREAD : ","DEACTIVE\r\n");
                CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
                break;                        
            }
            dwFileSize -= dwRead ; 
            pupu = (__int64)dwFileSize;
            pupu *= 1000;
            pupu /=cnr;
            SendMessage(X->hProg,PBM_SETPOS,(WPARAM)(1000-pupu), (LPARAM)0);
            if(dwFileSize < 1)
			{
                SMS("Send file : ","OK\r\n\r\n");
                CloseHandle(hFile);
		        free(X->buffer);
                free(X->client);
                EndDialog(X->hMain,0);
                DestroyWindow(X->hMain);
                free(X);
                SMS("THREAD : ","DEACTIVE\r\n");
                CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
                break;                        
            }      
             
        }
        // send file
        do{ 
            ret = sendto(TFTPSock, BufferToSend, dwRead+4,0,(struct sockaddr *)X->client,sizeof(*X->client));
        }while(ret < 0);
        if((dwFileSize-BufferSize) < 1)
		{
             SMS("Send file : ","OK\r\n\r\n");
             CloseHandle(hFile);
		     free(X->buffer);
             free(X->client);
             EndDialog(X->hMain,0);
             DestroyWindow(X->hMain);
             free(X);
             SMS("THREAD : ","DEACTIVE\r\n");
             CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
             break;                        
        }
    }
return;
}

void first_loop()
{
    int ret;
    unsigned char *Rdata = (unsigned char*)malloc(520);
    struct sockaddr_in *from;
    from = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    int from_len = sizeof(*from);
    SMS("Waiting for a new Request","\r\n");   
    while(1)
    {
        memset(Rdata, 0, 520);
        ret = recvfrom(TFTPSock,Rdata, 520,0,(struct sockaddr*)from,&from_len);
   	    if( (ret > 0) && (Rdata[1] == TFTP_RRQ) )
            break;
    }

    TPASS *X;
    X = (TPASS*)malloc(sizeof(TPASS));
    memset(X, 0, sizeof(TPASS));
    
    X->buffer = Rdata;
    X->client = from;
    X->id = ntohs(from->sin_port);
    CreateThread(0,0,(LPTHREAD_START_ROUTINE)Go,(LPVOID)X,0,0);
    sprintf(buffer,"Passing SOCK to NewLoop[%X]\0", X->id);
    SMS(buffer,"\r\n");
    return;   
}






void StartTFTP()
{
    TFTPSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(TFTPSock==INVALID_SOCKET)
    { 
        MessageBox(hWnd, "Cant't init TFTP socket","Error",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
        return;
    }
    int ret;
    int True = 1;
    // share bindings for UDP sockets
	ret = setsockopt(TFTPSock, SOL_SOCKET, SO_REUSEADDR,(const char *)&True, sizeof(True));
    if(ret == -1)
    {
        ret = MessageBox(hWnd, "Cant't setsockopt[SO_REUSEADDR] TFTP\r\nExit now?","setsockopt",MB_YESNO | MB_ICONQUESTION	| MB_SYSTEMMODAL);
        if(ret == IDYES)
        {
            shutdown(TFTPSock,2);
            closesocket(TFTPSock);
            return;     
        }
    }    
    //fill server structure for listening socket
    serverTFTP.sin_family=AF_INET;
	serverTFTP.sin_addr.s_addr=INADDR_ANY;  
	serverTFTP.sin_port=htons(TFTP_PORT_ALL); 
	//bind the socket
   	ret = bind(TFTPSock, (struct sockaddr*)&serverTFTP, sizeof(serverTFTP));
	if(ret!=0)
    { 
        MessageBox(hWnd, "Cant't bind TFTP socket","Error",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
        shutdown(TFTPSock,2);
        closesocket(TFTPSock);
        return; 
    }
    SetWindowText(sss,"");
    SMS("Waiting 1st Loop","\r\n");
    
    CreateThread(0,0,(LPTHREAD_START_ROUTINE)first_loop,0,0,0); 
}



