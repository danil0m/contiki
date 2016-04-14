/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "cfs/cfs.h"
#include "dev/eeprom.h"
#include <stdio.h>

struct filestate {
  int flag;
#define FLAG_FILE_CLOSED 0
#define FLAG_FILE_OPEN   1
  eeprom_addr_t fileptr;
  eeprom_addr_t filesize;
};

static struct filestate file={FLAG_FILE_CLOSED,0,0};

#ifdef CFS_EEPROM_CONF_OFFSET
#define CFS_EEPROM_OFFSET CFS_EEPROM_CONF_OFFSET
#else
#define CFS_EEPROM_OFFSET 0
#endif

#define CFS_EEPROM_EOF_ENABLED

#ifdef CFS_EEPROM_EOF_ENABLED
/* value of the page size, in order to put file info in the last page  */
#ifdef CFS_EEPROM_CONF_PAGE_SIZE
#define CFS_EEPROM_PAGE_SIZE CFS_EEPROM_CONF_PAGE_SIZE
#else
#define CFS_EEPROM_PAGE_SIZE 256
#endif /*CFS_EEPROM_CONF_EOF*/


#define VALID 0xDEADBEEF

struct file_eeprom {
	eeprom_addr_t filesize;
	uint32_t valid;
};
#endif /*CFS_EEPROM_EOF_ENABLED*/


/*---------------------------------------------------------------------------*/
int
cfs_open(const char *n, int f)
{
#ifdef CFS_EEPROM_EOF_ENABLED
struct file_eeprom file_info;
#endif /*CFS_EEPROM_EOF_ENABLED*/


  if(file.flag == FLAG_FILE_CLOSED) {
    file.flag = FLAG_FILE_OPEN;
    if(f & CFS_READ) {
      file.fileptr = 0;
#ifdef CFS_EEPROM_EOF_ENABLED
      if(file.filesize==0 ){
    	  /*checks if there is a valid file*/
    	  eeprom_read(EEPROM_SIZE-CFS_EEPROM_PAGE_SIZE, (uint8_t*)&file_info, sizeof(file_info));
    	  printf("fileinfo: filesize:%d\r\n",file_info.filesize);
    	  printf("fileinfo: valid:%x\r\n",file_info.valid);
    	  if(file_info.valid==VALID){
    		  file.filesize=file_info.filesize;
      	  }else {
      	    file.flag = FLAG_FILE_CLOSED;
      		  return -1;
      	  }
      }
#endif /*CFS_EEPROM_EOF_ENABLED*/
    }
    if(f & CFS_WRITE){
      if(f & CFS_APPEND) {
#ifdef CFS_EEPROM_EOF_ENABLED
      if(file.filesize==0 ){
    	  /*checks if there is a valid file*/
    	  eeprom_read(EEPROM_SIZE-CFS_EEPROM_PAGE_SIZE,(uint8_t*) &file_info, sizeof(file_info));
    	  if(file_info.valid==VALID){
    		  file.filesize=file_info.filesize;
      	  }
      }
#endif /*CFS_EEPROM_EOF_ENABLED*/

    	  file.fileptr = file.filesize;
      } else {
	file.fileptr = 0;
	file.filesize = 0;

#ifdef CFS_EEPROM_EOF_ENABLED
	/*sets new info of the file*/
	file_info.filesize=0;
	file_info.valid=VALID;
	eeprom_write(EEPROM_SIZE-CFS_EEPROM_PAGE_SIZE, (uint8_t*)&file_info, sizeof(file_info));
#endif /*CFS_EEPROM_EOF_ENABLED*/

      }
    }
    return 1;
  } else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
void
cfs_close(int f)
{
  file.flag = FLAG_FILE_CLOSED;
}
/*---------------------------------------------------------------------------*/
int
cfs_read(int f, void *buf, unsigned int len)
{
  if(f == 1) {
#ifdef  CFS_EEPROM_EOF_ENABLED
    	  if(file.fileptr+len>file.filesize){
    		    printf("file size: %d\r\n", file.filesize);
    		  return -1;
    	        }
#endif /*CFS_EEPROM_EOF_ENABLED*/
    eeprom_read(CFS_EEPROM_OFFSET + file.fileptr, buf, len);
    file.fileptr += len;
    return len;
  }
 else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
int
cfs_write(int f, const void *buf, unsigned int len)
{

#ifdef CFS_EEPROM_EOF_ENABLED
struct file_eeprom file_info;
/*check if writing in the info page*/
	  if(file.fileptr+len+CFS_EEPROM_PAGE_SIZE>EEPROM_SIZE){
	  	      	return -1;
	  	      }
#endif /*CFS_EEPROM_EOF_ENABLED*/
	  eeprom_write(CFS_EEPROM_OFFSET + file.fileptr, (unsigned char *)buf, len);

    file.fileptr += len;
    file.filesize=file.fileptr;
    printf("file size: %d\r\n", file.filesize);
#ifdef CFS_EEPROM_EOF_ENABLED
	/*sets new info of the file*/
    file_info.filesize=file.filesize;
	file_info.valid=VALID;
	eeprom_write(EEPROM_SIZE-CFS_EEPROM_PAGE_SIZE,(uint8_t*) &file_info, sizeof(file_info));
#endif /*CFS_EEPROM_EOF_ENABLED*/
    return len;

}
/*---------------------------------------------------------------------------*/
cfs_offset_t
cfs_seek(int f, cfs_offset_t o, int w)
{
  if(w == CFS_SEEK_SET && f == 1) {
#ifdef CFS_EEPROM_EOF_ENABLED
	  if(file.filesize-o<0){
    	return -1;
    }
#endif /*CFS_EEPROM_EOF_ENABLED*/
	  file.fileptr = o;
    return o;
  }
#ifdef CFS_EEPROM_EOF_ENABLED
  if(w==CFS_SEEK_END && f == 1){
	  if(file.filesize-o<0){
		  return -1;
	  }
	  file.fileptr=file.filesize-o;
	  return file.fileptr;
  }
#endif /*CFS_EEPROM_EOF_ENABLED*/


  else {
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
int
cfs_remove(const char *name)
{
#ifdef CFS_EEPROM_EOF_ENABLED
struct file_eeprom file_info;
/*sets new info of the file*/
	file.filesize=0;
	file_info.filesize=0;
	file_info.valid=VALID;
	eeprom_write(EEPROM_SIZE-CFS_EEPROM_PAGE_SIZE, (uint8_t*)&file_info, sizeof(file_info));


#endif /*CFS_EEPROM_EOF_ENABLED*/
  return -1;
}
/*---------------------------------------------------------------------------*/
int
cfs_opendir(struct cfs_dir *p, const char *n)
{
  return -1;
}
/*---------------------------------------------------------------------------*/
int
cfs_readdir(struct cfs_dir *p, struct cfs_dirent *e)
{
  return -1;
}
/*---------------------------------------------------------------------------*/
void
cfs_closedir(struct cfs_dir *p)
{
}
/*---------------------------------------------------------------------------*/
