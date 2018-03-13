/*******************************************************************************
Copyright(R) 2014-2024, MPR Tech. Co., Ltd
File Name: decoder_srv.h
Author: Karsn           Date: 2017-12-18             Version: 1.0
Discription:

Function:

History:
1. Version: 1.0        Date: 2017-12-18         Author: Karsn
    Modification:
      Creation.
*******************************************************************************/

#ifndef __DECODER_SRV_H__
#define __DECODER_SRV_H__

#define VIDEO_SHM_PATH "/tmp/isli_video_server.shm"
#define VIDEO_SEM_PATH "/tmp/isli_video_server.sem"

#define OUT_SOCK_PATH "/tmp/isli_scanner_out_server.socket" 

#define SOCK_BUF_SIZE 8*1024*1024

typedef struct
{
	int nCnt;
	int nWidth;
	int nHeight;
	unsigned char aPixels[SOCK_BUF_SIZE];
} TruImgFrame;

#endif
