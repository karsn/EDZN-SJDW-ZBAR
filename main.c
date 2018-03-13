/*******************************************************************************
Copyright(R) 2014-2024, MPR Tech. Co., Ltd
File Name: main.c
Author: Karsn           Date: 2018-03-12             Version: 1.0
Discription:

Function:

History:
1. Version: 1.0        Date: 2018-03-12         Author: Karsn
    Modification:
      Creation.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>  
#include <sys/sem.h>
#include <sys/un.h> 

#include <zbar.h>

#include "decoder_srv.h"

/****************************** Private typedef *******************************/
/****************************** Private define ********************************/
/****************************** Private macro *********************************/
/***************************** Private variables ******************************/
/************************ Private function prototypes *************************/

/*******************************************************************************
Function: 
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
Others:
*******************************************************************************/  
//函数：信号量P操作：对信号量进行减一操作  
static int semop_p(int nSemID)  
{  
    struct sembuf ltruv_SemBuf;  
  
    ltruv_SemBuf.sem_num = 0;//信号量编号  
    ltruv_SemBuf.sem_op = -1;//P操作   
    ltruv_SemBuf.sem_flg = SEM_UNDO;  
  
    return semop(nSemID,&ltruv_SemBuf,1);
}  
/*******************************************************************************
Function: 
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
Others:
*******************************************************************************/  
//函数：信号量V操作：对信号量进行加一操作  
static int semop_v(int nSemID)  
{  
    struct sembuf ltruv_SemBuf={0};  
  
    ltruv_SemBuf.sem_num = 0;//信号量编号  
    ltruv_SemBuf.sem_op = 1;//V操作    
    ltruv_SemBuf.sem_flg = SEM_UNDO;  
  
    return semop(nSemID,&ltruv_SemBuf,1);  
}

/*******************************************************************************
Function: 
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
Others:
*******************************************************************************/
int main (int argc, char **argv)
{
	//int ls32v_Ret = 0;
	int ls32v_ShmID = -1;
	int ls32v_SemID = -1;
	TruImgFrame *lptrv_ImgFrame = NULL;
	
	int ls32v_FrameCnt = -1;
	zbar_image_scanner_t *lptrv_Scanner = NULL;
	
	/* Get shm */
    ls32v_ShmID = shmget(ftok(VIDEO_SHM_PATH, 0), sizeof(TruImgFrame), 0666|IPC_CREAT|IPC_EXCL); //create shm
    if(ls32v_ShmID > 0)
    {
    	shmdt(lptrv_ImgFrame); //Release shm
		
	    printf("Please run video_srv first!\r\n");
    	exit(-1);
    }
    
    ls32v_ShmID = shmget(ftok(VIDEO_SHM_PATH, 0), sizeof(TruImgFrame), 0666|IPC_CREAT);
    lptrv_ImgFrame = (TruImgFrame *)shmat(ls32v_ShmID, NULL, 0); //Get shm addr
    if(lptrv_ImgFrame < 0)
    {
    	shmdt(lptrv_ImgFrame); //Release shm
    	perror("shmat failed\r\n");
    	exit(-1);
    }
    
    /* get sem */
    ls32v_SemID = semget(ftok(VIDEO_SEM_PATH, 0), 1, 0666|IPC_CREAT|IPC_EXCL); //create sem
    if(ls32v_SemID > 0)
    {
		shmdt(lptrv_ImgFrame); //Release shm
		semctl(ls32v_SemID,0,IPC_RMID); //Release sem
		
	    printf("Please run video_srv first!\r\n");
	    exit(-1);
    }
    ls32v_SemID = semget(ftok(VIDEO_SEM_PATH, 0), 1, 0666|IPC_CREAT);
    
    /* Create Sockete */
    int ls32v_SockFD = 0;                                                                                                                                     
    ls32v_SockFD = socket(AF_UNIX,SOCK_DGRAM,0);                                       
    if(ls32v_SockFD < 0)                                                               
    {                                                                            
        perror("Create socket error");                                                  
        exit(-1);                                                                
    }
    
    struct sockaddr_un ltruv_SockHostAddr;       //set host addr                                               
    memset(&ltruv_SockHostAddr,0,sizeof(ltruv_SockHostAddr));                                                                                                                               
    ltruv_SockHostAddr.sun_family = AF_UNIX;                                                   
    strcpy(ltruv_SockHostAddr.sun_path,OUT_SOCK_PATH);  
	
	
    /* create a reader */
    lptrv_Scanner = zbar_image_scanner_create();

    /* configure the reader */
    zbar_image_scanner_set_config(lptrv_Scanner, 0, ZBAR_CFG_ENABLE, 0);
    zbar_image_scanner_set_config(lptrv_Scanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    //zbar_image_scanner_set_config(lptrv_Scanner, ZBAR_ISBN13, ZBAR_CFG_ENABLE, 1);
    //zbar_image_scanner_set_config(lptrv_Scanner, ZBAR_EAN13, ZBAR_CFG_ENABLE, 1);
    zbar_image_scanner_set_config(lptrv_Scanner, 0, ZBAR_CFG_X_DENSITY, 1);
    zbar_image_scanner_set_config(lptrv_Scanner, 0, ZBAR_CFG_Y_DENSITY, 1);
    
    zbar_image_t *lptrv_Image = zbar_image_create();
    
    while(1)
    {
    	void *lptrv_Raw = NULL;
		/* Get Img */
		semop_p(ls32v_SemID);
		if((ls32v_FrameCnt == lptrv_ImgFrame->nCnt)
		|| (lptrv_ImgFrame->nWidth < 320)
		|| (lptrv_ImgFrame->nHeight < 320))
		{
			semop_v(ls32v_SemID);
		    continue;
		}
		
		/* Copy Img */
		ls32v_FrameCnt = lptrv_ImgFrame->nCnt;
		lptrv_Raw = malloc(lptrv_ImgFrame->nWidth*lptrv_ImgFrame->nHeight);
		if(lptrv_Raw == NULL)
		{
			semop_v(ls32v_SemID);
			printf("%s(): Malloc for raw failed!\r\n",argv[0]);
			continue;
		}
		memcpy(lptrv_Raw, lptrv_ImgFrame->aPixels, lptrv_ImgFrame->nWidth*lptrv_ImgFrame->nHeight);
			
		/* wrap image data */
	    zbar_image_set_format(lptrv_Image, *(int*)"Y800");
	    zbar_image_set_size(lptrv_Image, lptrv_ImgFrame->nWidth, lptrv_ImgFrame->nHeight);
	    zbar_image_set_data(lptrv_Image, 
	                        lptrv_Raw, 
	                        lptrv_ImgFrame->nWidth*lptrv_ImgFrame->nHeight, 
	                        zbar_image_free_data);
		
		semop_v(ls32v_SemID);
		
		/* scan the image for barcodes */
    	int nsyms = zbar_scan_image(lptrv_Scanner, lptrv_Image);
    	if(nsyms < 0)
    	{
    		printf("%s(): scan failed!\r\n", argv[0]);
    		
    		continue;
    	}
    	printf("%s(): FrameCnt-%d..\r\n", argv[0], ls32v_FrameCnt);
    	
    	/* Extract Centers */
    	zbar_point_int_t *lptrv_QRPoint = NULL;
		int ls32v_NumCenters = zbar_image_get_center(lptrv_Image, &lptrv_QRPoint);

    
    	/* extract results */
	    const zbar_symbol_t *symbol = zbar_image_first_symbol(lptrv_Image);
	    for(; symbol; symbol = zbar_symbol_next(symbol)) 
	    {
	        /* do something useful with results */
	        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
	        const char *data = zbar_symbol_get_data(symbol);
	        printf("decoded %s symbol \"%s\" quality=%d\n", zbar_get_symbol_name(typ), 
	                                                        data, 
	                                                        zbar_symbol_get_quality(symbol));
	    	
	    	char lptrv_SockBuf[256] = {0};
	    	strcpy(lptrv_SockBuf, zbar_symbol_get_data(symbol));
	    	strcat(lptrv_SockBuf, "\r\n");
	    	sendto(ls32v_SockFD,
	    	       lptrv_SockBuf,
	    	       strlen(lptrv_SockBuf),
	    	       0,
	    	       (struct sockaddr*)&ltruv_SockHostAddr,
	    	       strlen(ltruv_SockHostAddr.sun_path)+sizeof(ltruv_SockHostAddr.sun_family));
		}
	    
    }

    /* clean up */
    zbar_image_destroy(lptrv_Image);
    zbar_image_scanner_destroy(lptrv_Scanner);

    exit(0);
}
