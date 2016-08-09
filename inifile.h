
#ifndef INI_FILE_H_
#define INI_FILE_H_

#ifdef __cplusplus
extern "C"
{

#endif

int read_profile_string( const char *section, const char *key,char *value, int size,const char *default_value, const char *file);
int read_profile_int( const char *section, const char *key,int default_value, const char *file);
int write_profile_string( const char *section, const char *key,const char *value, const char *file);
int readsysfile(char *file,char *key,char *r_buf);
int writesysfile(char *file,char *key,char *value);
int readfile(char *file,char *key,char *r_buf);
int load_ini_file(const char *file, char *buf,int *file_size);

int parse_file(const char *section, const char *key, const char *buf,int *sec_s,int *sec_e,
					  int *key_s,int *key_e, int *value_s, int *value_e);

#ifdef __cplusplus
}; //end of extern "C" {
#endif

#endif //end of INI_FILE_H_

