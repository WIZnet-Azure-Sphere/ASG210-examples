#ifndef __AZ_M4_PROCESS_H
#define __AZ_M4_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#if 1

// 0
#define SOCK_CONFIG_TCP     1
#define SOCK_DATA_TCP       3
#define SOCK_DHCP           4
#define SOCK_SNTP           5
#define SOCK_FWUPDATE       6
#define SOCK_CONFIG_UDP     7

#else

#if 1 //becky 200803
#define SOCK_DATA_TCP          3					   
#endif
#define SOCK_CONFIG_UDP        7
#define SOCK_CONFIG_TCP        1


#define SOCK_DHCP       4
#define SOCK_SNTP       5
#define SOCK_FWUPDATE   6
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif