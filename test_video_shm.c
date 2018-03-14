/*******************************************************************************
Copyright(R) 2014-2024, MPR Tech. Co., Ltd
File Name: test_video.c
Author: Karsn           Date: 2017-12-18             Version: 1.0
Discription:

Function:

History:
1. Version: 1.0        Date: 2017-12-18         Author: Karsn
    Modification:
      Creation.
*******************************************************************************/
#include <assert.h>

#include <stdio.h>                                                               
#include <string.h>                                                              
#include <unistd.h>                                                              
#include <stdlib.h>     
#include <signal.h>                                                         
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>  
#include <sys/sem.h>                                                      
#include <sys/un.h>    

#include <png.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

#include "decoder_srv.h"

/****************************** Private typedef *******************************/


/****************************** Private define ********************************/

/****************************** Private macro *********************************/
/***************************** Private variables ******************************/


/************************ Private function prototypes *************************/


/*******************************************************************************
Function: 
Description:
 use libpng to read an image file.
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
Others:
*******************************************************************************/
static int get_data (const char *name, int *width, int *height, void *raw)
{
    /* Check Input */
    if((name==NULL)||(width==NULL)||(height==NULL)||(raw==NULL))
    {
    	printf("%s(): input failed\r\n",__FUNCTION__);
    	return -1;
    }
    
    FILE *file = fopen(name, "rb");
    if(!file) 
    {
    	printf("%s(): Open file failed-%s\r\n", __FUNCTION__, name);
    	return -1;
    }
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) 
    {
    	printf("%s(): Create png struct failed\r\n", __FUNCTION__);
    	
    	fclose(file);
    	return -1;
    }
    if(setjmp(png_jmpbuf(png))) 
    {
    	printf("%s(): set png error proc failed\r\n", __FUNCTION__);
    	png_destroy_read_struct(&png, NULL, NULL);
    	fclose(file);
    	return -1;
    }
    png_infop info = png_create_info_struct(png);
    if(!info) 
    {
    	printf("%s(): Create png info failed\r\n", __FUNCTION__);
    	png_destroy_read_struct(&png, NULL, NULL);
    	fclose(file);
    	return -1;
    }
    png_init_io(png, file);
    png_read_info(png, info);
    /* configure for 8bpp grayscale input */
    int color = png_get_color_type(png, info);
    int bits = png_get_bit_depth(png, info);
    if(color & PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    if(color == PNG_COLOR_TYPE_GRAY && bits < 8)
        png_set_gray_1_2_4_to_8(png);
    if(bits == 16)
        png_set_strip_16(png);
    if(color & PNG_COLOR_MASK_ALPHA)
        png_set_strip_alpha(png);
    if(color & PNG_COLOR_MASK_COLOR)
        png_set_rgb_to_gray_fixed(png, 1, -1, -1);
    /* allocate image */
    *width = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    png_bytep rows[*height];
    int i;
    for(i = 0; i < *height; i++)
        rows[i] = raw + (*width * i);
    png_read_image(png, rows);

	png_destroy_read_struct(&png, &info, NULL);
    fclose(file);

    return 0;
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
Function: get_imgs_from_dir
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
 1.[pppImgs]
    Png img files
Return:
 1.[int]
   >0 Nums of the files
   <0 failed
Others:
*******************************************************************************/
static int get_imgs_from_dir(char const *pDirPath, char ***pppImgs)
{
	int ls32v_Cnt = 0;
	struct dirent *lptrv_FileHandle = NULL;//readdir 的返回类型  
	DIR *lptrv_DirHandle = NULL;

	/* Check Input */
	assert(pDirPath != NULL);
	
	/*Search */
    lptrv_DirHandle = opendir(pDirPath);  
    if(lptrv_DirHandle == NULL)  
    {  
        printf("%s(): open dir %s error!\n",__FUNCTION__, pDirPath);  
        return -1;  
    }  
      
    while((lptrv_FileHandle = readdir(lptrv_DirHandle)) != NULL)  
    {  
        //目录结构下面问什么会有两个.和..的目录？ 跳过着两个目录  
        if(!strcmp(lptrv_FileHandle->d_name,".")||!strcmp(lptrv_FileHandle->d_name,".."))  
            continue;  
              
        //非常好用的一个函数，比什么字符串拼接什么的来的快的多  
        char lu8v_FilePath[512] = {0};  
        sprintf(lu8v_FilePath,"%s/%s", pDirPath,lptrv_FileHandle->d_name);  
          
        struct stat s;  
        lstat(lu8v_FilePath,&s);  
          
        if((!S_ISDIR(s.st_mode)) && (strlen(lptrv_FileHandle->d_name) > 4))
        {  
        	if(strcmp(&(lptrv_FileHandle->d_name[strlen(lptrv_FileHandle->d_name)-4]), ".png") == 0)
        	{
        		printf("%s(): find file - %s\r\n",__FUNCTION__, lptrv_FileHandle->d_name); 
        		
        		if(*pppImgs == NULL)
        		{
        			ls32v_Cnt = 0; //Clear ls32v_Cnt
        			
        			*pppImgs = (char **)malloc(512*sizeof(char *));
        			if(*pppImgs == NULL)
        			{
        				printf("%s(): malloc for *ppImgs failed..\r\n",__FUNCTION__);
        			}
        		}
        		
        		if(ls32v_Cnt < 512)
        		{
	        		(*pppImgs)[ls32v_Cnt] = malloc(strlen(lu8v_FilePath)+2);
	        		if((*pppImgs)[ls32v_Cnt] != NULL)
	        		{
	        			strcpy((*pppImgs)[ls32v_Cnt++], lu8v_FilePath);
	        		}
	        		else 
	        		{
	        			printf("%s(): save file failed\r\n", __FUNCTION__);
	        		}
	        	}
            }
        }  
    }  
    closedir(lptrv_DirHandle);  
	
	return ls32v_Cnt;
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
static int rls_img_path(char **ppImgs, int nNums)
{
	if(ppImgs != NULL)
	{
		int ls32v_Cnt = 0;
		
		for(ls32v_Cnt=0; ls32v_Cnt < nNums; ls32v_Cnt++)
		{
			if(ppImgs[ls32v_Cnt] != NULL)
			{
				free(ppImgs[ls32v_Cnt]);
			}
		}
		
		free(ppImgs);
	}
	
	return 0;
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
void sigroutine(int nSignal)
{
	printf("nSignal is %d\r\n",nSignal);
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
	int ls32v_Ret = 0;
	int ls32v_ShmID = -1;
	int ls32v_SemID = -1;
	TruImgFrame *lptrv_ImgFrame = NULL;
	
	int ls32v_NumsOfImgs = 0;
	char **lptrv_ImgPath = NULL;

    /* Check Arg */    
    if(argc < 2)
    {
    	fprintf(stderr, "Usage: %s imgfile \r\n", argv[0]);
    	exit(-1);
    }
    
    printf("%s(): pid is %d ...\r\n", argv[0], getpid());
	//signal(SIGINT, sigroutine);
    
    /* Get Img File */
    ls32v_NumsOfImgs = get_imgs_from_dir(argv[1], &lptrv_ImgPath);
    
    
    /* Create shm */
    ls32v_ShmID = shmget(ftok(VIDEO_SHM_PATH, 0), sizeof(TruImgFrame), 0666|IPC_CREAT); //create shm
    if(ls32v_ShmID < 0)
    {
    	perror("shmget failed!\r\n");
    	rls_img_path(lptrv_ImgPath, ls32v_NumsOfImgs);
    	exit(1);
    }
    
    lptrv_ImgFrame = (TruImgFrame *)shmat(ls32v_ShmID, NULL, 0); //Get shm addr
    if(lptrv_ImgFrame < 0)
    {
    	perror("shmat failed\r\n");
    	rls_img_path(lptrv_ImgPath, ls32v_NumsOfImgs);
    	exit(1);
    }
    
    /* Create Sem */
    ls32v_SemID = semget(ftok(VIDEO_SEM_PATH, 0), 1, 0666|IPC_CREAT); //create sem
    if(ls32v_SemID < 0)
    {
    	perror("semget failed!\r\n");
    	rls_img_path(lptrv_ImgPath, ls32v_NumsOfImgs);
    	exit(1);
    }
    
    ls32v_Ret = semctl(ls32v_SemID,0,SETVAL,1); //初始化信号量，赋值为1
    if(ls32v_Ret < 0)
    {
    	perror("semctl init failed\r\n");
    	rls_img_path(lptrv_ImgPath, ls32v_NumsOfImgs);
    	exit(1);
    }
    
	while(1)
	{
		semop_p(ls32v_SemID);

	    /* obtain image data */
	    if(lptrv_ImgFrame < 0)
	    {
	    	fprintf(stderr,"%s(): lptrv_ImgFrame is null\r\n", argv[0]);
	    	break;
	    }
	    
	    ls32v_Ret = get_data(lptrv_ImgPath[lptrv_ImgFrame->nCnt%ls32v_NumsOfImgs], 
	                         &(lptrv_ImgFrame->nWidth), 
	                         &(lptrv_ImgFrame->nHeight), 
	                         &(lptrv_ImgFrame->aPixels));
	    
	    printf("%s(): nCnt-%d,Img-%s nWidth-%d, nHeight-%d\r\n",
	    	   argv[0],
	           lptrv_ImgFrame->nCnt,
	           lptrv_ImgPath[lptrv_ImgFrame->nCnt%ls32v_NumsOfImgs],
	           lptrv_ImgFrame->nWidth, 
	           lptrv_ImgFrame->nHeight);
	           
	    if(ls32v_Ret >= 0)
	    {
	    	lptrv_ImgFrame->nCnt++;
	    }
	    
	    semop_v(ls32v_SemID);

	    //usleep(100*1000);
	    sleep(1);
	}
    
    printf("%s(): end ... \r\n", argv[0]);
    
    /* Release */
    if(shmdt(lptrv_ImgFrame) < 0)
    {
    	perror("shmdt failed\r\n");
    }
    
    if(semctl(ls32v_SemID,0,IPC_RMID) < 0)
    {
    	perror("semctl del failed\r\n");
    }
    
    rls_img_path(lptrv_ImgPath, ls32v_NumsOfImgs);

    exit(0);
}
