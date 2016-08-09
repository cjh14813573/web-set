/*==========================================================================*
 *    Copyright(c) 2008-2011, China soft Co., Ltd.
 *                     ALL RIGHTS RESERVED
 *
 *  PRODUCT  : RTU2008
 *
 *  FILENAME : inifile.c
 *  CREATOR  : RTU Team                 DATE: 2008-10-29 10:00
 *  VERSION  : V1.00
 *  PURPOSE  : 
 *
 *
 *  HISTORY  :
 *
 *==========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "inifile.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_FILE_SIZE 1024*512
#define LEFT_BRACE '['
#define RIGHT_BRACE ']'

int load_ini_file(const char *file, char *buf,int *file_size)
{
	FILE *in = NULL;
	int i=0;
	*file_size =0;

	assert(file !=NULL);
	assert(buf !=NULL);

	in = fopen(file,"r");
	if( NULL == in) { 
		return 0;
	}

	buf[i]=fgetc(in);
	
	//load initialization file
	while( buf[i]!= (char)EOF) {
		i++;
		assert( i < MAX_FILE_SIZE ); //file too big, you can redefine MAX_FILE_SIZE to fit the big file 
		buf[i]=fgetc(in);
	}
	
	buf[i]='\0';
	*file_size = i;

	fclose(in);
	return 1;
}

static int newline(char c)
{
	return ('\n' == c ||  '\r' == c )? 1 : 0;
}
static int end_of_string(char c)
{
	return '\0'==c? 1 : 0;
}
static int left_barce(char c)
{
	return LEFT_BRACE == c? 1 : 0;
}
static int isright_brace(char c )
{
	return RIGHT_BRACE == c? 1 : 0;
}
int parse_file(const char *section, const char *key, const char *buf,int *sec_s,int *sec_e,
					  int *key_s,int *key_e, int *value_s, int *value_e)
{
	const char *p = buf;
	int i=0;

	assert(buf!=NULL);
	assert(section != NULL && strlen(section));
	assert(key != NULL && strlen(key));

	*sec_e = *sec_s = *key_e = *key_s = *value_s = *value_e = -1;

	while( !end_of_string(p[i]) ) 
		{
		//find the section
		if( ( 0==i ||  newline(p[i-1]) ) && left_barce(p[i]) )
		{
			int section_start=i+1;

			//find the ']'
			do{
				i++;
			} while( !isright_brace(p[i]) && !end_of_string(p[i]));

			if(i-section_start==strlen(section)&&0 == strncmp(p+section_start,section, i-section_start)) {
				int newline_start=0;

				i++;

				//Skip over space char after ']'
				while(isspace(p[i])) {
					i++;
				}

				//find the section
				*sec_s = section_start;
				*sec_e = i;

				while( ! (newline(p[i-1]) && left_barce(p[i])) 
				&& !end_of_string(p[i]) ) {
					int j=0;
					//get a new line
					newline_start = i;

					while( !newline(p[i]) &&  !end_of_string(p[i]) ) {
						i++;
					}
					
					//now i  is equal to end of the line
					j = newline_start;

					if(';' != p[j]) //skip over comment
					{
						while(j < i && p[j]!='=') {
							j++;
							if('=' == p[j]) {
								if(j-newline_start==strlen(key)&&strncmp(key,p+newline_start,j-newline_start)==0)
								{
									//find the key ok
									*key_s = newline_start;
									*key_e = j-1;

									*value_s = j+1;
									*value_e = i;

									return 1;
								}
							}
						}
					}

					i++;
				}
			}
		}
		else
		{
			i++;
		}
	}
	return 0;
}


int read_profile_string( const char *section, const char *key,char *value, 
		 int size, const char *default_value, const char *file)
{
	char buf[MAX_FILE_SIZE]={0};
	int file_size;
	int sec_s,sec_e,key_s,key_e, value_s, value_e;

	//check parameters
	assert(section != NULL && strlen(section));
	assert(key != NULL && strlen(key));
	assert(value != NULL);
	assert(size > 0);
	assert(file !=NULL &&strlen(key));

	if(!load_ini_file(file,buf,&file_size))
	{
		if(default_value!=NULL)
		{
			strncpy(value,default_value, size);
		}
		return 0;
	}

	if(!parse_file(section,key,buf,&sec_s,&sec_e,&key_s,&key_e,&value_s,&value_e))
	{
		if(default_value!=NULL)
		{
			strncpy(value,default_value, size);
		}
		return 0; //not find the key
	}
	else
	{
		int cpcount = value_e -value_s;

		if( size-1 < cpcount)
		{
			cpcount =  size-1;
		}
	
		memset(value, 0, size);
		memcpy(value,buf+value_s, cpcount );
		value[cpcount] = '\0';

		return 1;
	}
}


int read_profile_int( const char *section, const char *key,int default_value, 
				const char *file)
{
	char value[32] = {0};
	if(!read_profile_string(section,key,value, sizeof(value),NULL,file))
	{
		return default_value;
	}
	else
	{
		return atoi(value);
	}
}

int write_profile_string(const char *section, const char *key,
					const char *value, const char *file)
{
	char buf[MAX_FILE_SIZE]={0};
	char w_buf[MAX_FILE_SIZE]={0};
	int sec_s,sec_e,key_s,key_e, value_s, value_e;
	int value_len = (int)strlen(value);
	int file_size;
	FILE *out;

	//check parameters
	assert(section != NULL && strlen(section));
	assert(key != NULL && strlen(key));
	assert(value != NULL);
	assert(file !=NULL &&strlen(key));

	if(!load_ini_file(file,buf,&file_size))
	{
		sec_s = -1;
	}
	else
	{
		parse_file(section,key,buf,&sec_s,&sec_e,&key_s,&key_e,&value_s,&value_e);
	}

	if( -1 == sec_s)
	{
		if(0==file_size)
		{
			sprintf(w_buf+file_size,"[%s]\n%s=%s\n",section,key,value);
		}
		else
		{
			//not find the section, then add the new section at end of the file
			memcpy(w_buf,buf,file_size);
			sprintf(w_buf+file_size,"\n[%s]\n%s=%s\n",section,key,value);
		}
	}
	else if(-1 == key_s)
	{
		//not find the key, then add the new key=value at end of the section
		memcpy(w_buf,buf,sec_e);
		sprintf(w_buf+sec_e,"%s=%s\n",key,value);
		sprintf(w_buf+sec_e+strlen(key)+strlen(value)+2,buf+sec_e, file_size - sec_e);
	}
	else
	{
		//update value with new value
		memcpy(w_buf,buf,value_s);
		memcpy(w_buf+value_s,value, value_len);
		memcpy(w_buf+value_s+value_len, buf+value_e, file_size - value_e);
	}
	
	out = fopen(file,"w");
	if(NULL == out)                                                                   
	{
		return 0;
	}
	
	if(-1 == fputs(w_buf,out) )
	{
		fclose(out);
		return 0;
	}

	fclose(out);
	return 1;
}
int writesysfile(char *file,char *key,char *value)
{
char tempbuff[MAX_FILE_SIZE]={0};
       char buf[MAX_FILE_SIZE]={0};
	char w_buf[MAX_FILE_SIZE]={0};
	int  value_s, value_e;
	int value_len = (int)strlen(value);
	int file_size;
	char *q;
	FILE *out;
    load_ini_file(file,buf,&file_size);
   //   int newline_start=0,key_flag=0;
    const char *p=buf;
/*for(ii=0;ii<strlen(buf);ii++)
	{
		if(p[ii]=='#')
		{
			while(p[i]!='\n'&&p[i]!='\r')
			{
				ii++;
			}
			i=ii;
		}
	}*/
if(( q = strstr( p, key )) != NULL )
{
value_s=q+strlen(key)+1-&buf[0];
}
else
{
	printf("\nNot Found\n");

	q = strstr( p, "clear" );
	value_s=5+q-&buf[0];
       memcpy(w_buf,buf,value_s);
	sprintf(tempbuff,"\n%s ",key);
       memcpy((w_buf+value_s),&tempbuff[0], (strlen(key)+2));	
       value_s=5+q+strlen(key)+2-&buf[0];


value_e=5+q-&buf[0];
memcpy(w_buf+value_s,value, value_len);
memcpy(w_buf+value_s+value_len, buf+value_e, file_size - value_e);
          
      out = fopen(file,"w");
	if(NULL == out)                                                                   
	{
		return 0;
	}
	
	if(-1 == fputs(w_buf,out) )
	{
		fclose(out);
		printf("writesys wrong");
		return 0;
	}

	fclose(out);
	return 1;
}
q=&buf[0]+value_s;
memcpy(w_buf,buf,value_s);
while(*q!='\r'&&*q!='\n'&&*q!=' '&&*q!='\0')
{
    q++;
}
value_e=q-&buf[0];
memcpy(w_buf+value_s,value, value_len);
memcpy(w_buf+value_s+value_len, buf+value_e, file_size - value_e);
          
      out = fopen(file,"w");
	if(NULL == out)                                                                   
	{
		return 0;
	}
	
	if(-1 == fputs(w_buf,out) )
	{
		fclose(out);
		printf("writesys wrong");
		return 0;
	}

	fclose(out);
	return 1;
}
int readsysfile(char *file,char *key,char *r_buf)
{
       char buf[MAX_FILE_SIZE]={0};
	int  value_s, value_e;
	unsigned int value_len ;
	int file_size;//i=0,j=0;
	char  *q;
	
  if( load_ini_file(file,buf,&file_size)==0)
  	{
          return  0;
	}
    //int newline_start=0,key_flag=0;
    const char *p=buf;
if(( q = strstr( p, key )) != NULL )
{
   value_s=q+strlen(key)+1-&buf[0];
}
else
	{
	return 0;
      }
 q=&buf[0]+value_s;
while(*q!='\r'&&*q!='\n'&&*q!=' ')
{
    q++;
}        
value_e=q-&buf[0];
value_len=value_e-value_s;
             memcpy(r_buf,buf+value_s,value_len);
			 *(r_buf+value_len)='\0';
			 return 1;

}

int readfile(char *file,char *key,char *r_buf)
{
       char buf[MAX_FILE_SIZE]={0};
	int  value_s, value_e;
	unsigned int value_len ;
	int file_size;//i=0,j=0;
	char  *q;
	
  if( load_ini_file(file,buf,&file_size)==0)
  	{
          return  0;
	}
    //int newline_start=0,key_flag=0;
    const char *p=buf;
if(( q = strstr( p, key )) != NULL )
{
   value_s=q+strlen(key)+1-&buf[0];
}
else
	{
	return 0;
      }
 q=&buf[0]+value_s;
while(*q!='\r'&&*q!='\n'&&*q!=' '&&*q!='\0')
{
    q++;
}        
value_e=q-&buf[0];
value_len=value_e-value_s;
             memcpy(r_buf,buf+value_s,value_len);
			 *(r_buf+value_len)='\0';
			 return 1;

}
#ifdef __cplusplus
}; //end of extern "C" {
#endif
