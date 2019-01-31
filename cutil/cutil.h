
#ifndef __CUTIL_H__
#define __CUTIL_H__

#define ALIGN_BY_SIZE(_x, _size) (((_x) & ~((_size)-1)) + ((_size)-1))

#ifdef __cplusplus
extern "C" {
#endif

extern int make_local_server(const char *servername);
extern int wait_for_client(int sscockfd, int timeout);
extern int connect_to_server(const char *servername, int waittime);
extern int alloc_shared_mem(unsigned long size, void **smemref);
extern void *map_smem(int fd, unsigned long size);
extern void unmap_smem(void *maped_mem, unsigned long size);
extern int send_fd(int sockfd, int sfd);
extern int rcv_fd(int sockfd);
extern int watch_fd(int fd, int timeout);
extern int check_connection_termination(int sockfd);
extern int wait_for_response(int fd, int timeout, void *resp, int buflen);
extern unsigned int get_rnd_no(int minlimit, int maxlimit);
extern int is_accessible_dir(const char *dirpath);
extern char *get_temp_dir(void);
extern void clear_rx_buffer(int sockfd);
extern int send_data(int sockfd, void *buffer, unsigned long size);
extern int resize_shared_mem(int fd, unsigned long old_size, unsigned long new_size, void **smemref);
extern long double get_millisecond_time(void);
#ifdef __cplusplus
}
#endif

#endif /* __VMCUTIL_H__ */
