
#ifdef ENABLE_MREMAP_SYSCALL
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include<sys/time.h>

#include "cutil.h"

#define handle_error(msg) do { perror(msg); return -1; } while(0)

#ifndef LOCAL_MAX_ALLOWED_CLIENT
#define LOCAL_MAX_ALLOWED_CLIENT 4
#endif

int make_local_server(const char *servername)
{
    int opt = 1;
    int master_socket;
    struct sockaddr_un address;

    /* create a master socket */
    if( (master_socket = socket(AF_UNIX , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        return -1;
    }

    /* set master socket to allow multiple connections , this is just a good habit, it will work without this */
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        return -1;
    }

    /* type of socket created */
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path + 1, servername, strlen(servername));

    /* bind the socket to unix address */
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        return -1;
    }

    /* try to specify maximum of 1 pending connections for the master socket */
    if (listen(master_socket, LOCAL_MAX_ALLOWED_CLIENT) < 0)
    {
        perror("listen failed");
        return -1;
    }

    return master_socket;
}

int wait_for_client(int sscockfd, int timeout)
{
    int client_socket = -1, sfd = sscockfd;
    int addrlen = 0, activity = -1;
    struct sockaddr_un address;
    /* set of socket descriptors */
    fd_set readfd;
    struct timeval timeval = {0L, 10000L}; // 10ms
    struct timeval *tv = (timeout >= 0) ? &timeval : NULL;

    if (!tv) timeval.tv_usec = 1000*timeout;

    if (sfd < 0) return -1;

    /* clear the socket set */
    FD_ZERO(&readfd);

    /* add master socket to set */
    FD_SET(sfd, &readfd);

    /* wait for an activity on one of the sockets , if timeout is NULL then wait indefinitely */
    activity = select(sfd + 1 , &readfd , NULL , NULL , tv);

    if ((activity < 0) && (errno!=EINTR))
    {
        printf("select error!!\n");
        return -1;
    }

    /* If something happened on the master socket , then its an incoming connection */
    if (FD_ISSET(sfd, &readfd)) {

        if ((client_socket = accept(sfd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            return -1;
        }
    }

    return client_socket;
}

int connect_to_server(const char *servername, int waittime)
{
    int err = 0;
    int usdelay = 900;
    int retry = waittime;
    int sockfd = -1;
    struct sockaddr_un addr;

    /* Create client socket */
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
       return -1;
    }

    /* Assigning socket type and address */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path + 1, servername, strlen(servername));
    while (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        if(err != errno) perror("connect error");
        err = errno;
        if (waittime >= 0) {
            retry--;
            if(retry < 0) {
                close(sockfd);
                sockfd = -1;
                break;
            }
        }
        usleep(usdelay);
    }
    return sockfd;
}

int alloc_shared_mem(unsigned long size, void **smemref)
{
    const char template[] = "/smem-XXXXXX";
    const char *path;
    char *name;
    int fd;
    int ret;
    long flags;

    if(smemref) *smemref = NULL;
    path = get_temp_dir();
    if (!path) return -1;

    name = malloc(strlen(path) + sizeof(template));
    if (!name) return -1;

    strcpy(name, path);
    strcat(name, template);

    fd = mkstemp(name);
    if (fd >= 0) {
        flags = fcntl(fd, F_GETFD);
        fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
        unlink(name);
    }

    free(name);

    if (fd < 0) return -1;

    ret = ftruncate(fd, size);
    if (ret < 0) {
        close(fd);
        return -1;
    }

    if (smemref) {
        *smemref = map_smem(fd,size);
        if (*smemref == NULL) {
            close(fd);
            return -1;
        }
    }
    return fd;
}

void *map_smem(int fd, unsigned long size)
{
    void *ret = NULL;
    if ((fd <= 0) || (size == 0)) return NULL;
    ret = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ((long)MAP_FAILED == (long)ret)  return NULL;
    return ret;
}

void unmap_smem(void *maped_mem, unsigned long size)
{
    if ((maped_mem == NULL) || (size == 0)) return;
    munmap(maped_mem, size);
}

int send_fd(int sockfd, int sfd)
{
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    char buf[CMSG_SPACE(sizeof(int))], dup[256];
    struct iovec io = { .iov_base = &dup, .iov_len = sizeof(dup) };

    memset(buf, '\0', sizeof(buf));
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

    memcpy ((int *) CMSG_DATA(cmsg), &sfd, sizeof (int));

    if (sendmsg (sockfd, &msg, MSG_NOSIGNAL) < 0) {
        handle_error ("Failed to send message");
    }

    return 0;
}

int rcv_fd(int sockfd)
{
    int fd = -1;
    struct msghdr msg = {0};
    struct cmsghdr *cmsg;
    char buf[CMSG_SPACE(sizeof(int))], dup[256];
    struct iovec io = { .iov_base = &dup, .iov_len = sizeof(dup) };

    memset(buf, '\0', sizeof(buf));
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    if (recvmsg (sockfd, &msg, 0) < 0) {
        handle_error ("Failed to receive message");
    }
    cmsg = CMSG_FIRSTHDR(&msg);

    memcpy (&fd, (int *) CMSG_DATA(cmsg), sizeof(int));

    return fd;
}

int watch_fd(int fd, int timeout)
{
    struct pollfd pfd = { fd, POLLIN, 0 };
    return poll (&pfd, 1, timeout);
}

int check_connection_termination(int sockfd)
{
#ifndef POLLRDHUP
#define POLLRDHUP       0x2000
#endif
    struct pollfd pfd = { sockfd, POLLRDHUP, 0 };
    return poll (&pfd, 1, 0);
}

int wait_for_response(int fd, int timeout, void *resp, int buflen)
{
    int status = watch_fd(fd, timeout);

    if (status > 0) {
        if ( resp && (buflen > 0)) {
            status = read(fd, resp, buflen);
            if (!status) return -1;
        }
        else status = 0;
    }
    return status;
}

unsigned int get_rnd_no(int minlimit, int maxlimit)
{
    return (rand() % (maxlimit - minlimit + 1)) + minlimit;
}


int is_accessible_dir(const char *dirpath)
{
    struct stat sbuf;
    if (stat(dirpath, &sbuf) < 0) return 0;
    else return ((sbuf.st_mode & S_IFMT) == S_IFDIR);
}

char *get_temp_dir(void)
{
    char *path = NULL;
    path = getenv("XDG_RUNTIME_DIR");
    if ((!path) || (path && (!is_accessible_dir(path)))) {
        path = getenv("TMPDIR");
        if ((!path) || (path && (!is_accessible_dir(path)))) {
            path = getenv("TMP");
            if ((!path) || (path && (!is_accessible_dir(path)))) {
                path = getenv("TEMP");
                if ((!path) || (path && (!is_accessible_dir(path)))) {
                    path = "/tmp";
                    if ((!path) || (path && (!is_accessible_dir(path)))) {
                        errno = ENOENT;
                        return NULL;
                    }
                }
            }
        }
    }
    return path;
}

void clear_rx_buffer(int sockfd)
{
    char discard_buf[128];
    while (watch_fd(sockfd, 0) > 0) read(sockfd, discard_buf, sizeof(discard_buf));
}

int send_data(int sockfd, void *buffer, unsigned long size)
{
    return send(sockfd, buffer, size, MSG_NOSIGNAL);
}

int resize_shared_mem(int fd, unsigned long old_size, unsigned long new_size, void **smemref)
{
    if (fd < 0) return -1;
    if (old_size == new_size) return fd;

    if (ftruncate(fd, new_size) < 0) goto ResizeRemapError;

    if (smemref) {
        if (! (*smemref)) {
            *smemref = map_smem(fd, new_size);
            if (! (*smemref)) goto ResizeRemapError;
        }
        else {
#ifdef ENABLE_MREMAP_SYSCALL
            *smemref = mremap(*smemref, old_size, new_size, MREMAP_MAYMOVE);
            if ((long)MAP_FAILED == (long)(*smemref)) goto ResizeRemapError;
#else
            unmap_smem(*smemref, old_size);
            *smemref = map_smem(fd, new_size);
            if (! (*smemref)) goto ResizeRemapError;
#endif
        }
    }
    return fd;
ResizeRemapError:
    if (smemref) unmap_smem(*smemref, old_size);
    close(fd);
    return -1;
}

long double get_millisecond_time(void)
{
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long double)tv.tv_sec)*1000.0f)+(((long double)tv.tv_usec)/1000.0f);
}

