#ifndef _DHCPS_H_
#define _DHCPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IS_USE_FIXED_IP	0
#define debug_dhcps 0
#if (debug_dhcps)
#define DEBUG_UNMARK_IP_IN_TABLE
#define DEBUG_DHCPS_CHECK_MSG_AND_HANDLE_OPTIONS
#define DEBUG_SEARCH_MAC
#define DEBUG_MARK_IP_IN_TABLE

#endif

/* dhcp server states */
#define DHCP_SERVER_STATE_OFFER 			(1)
#define DHCP_SERVER_STATE_DECLINE 		(2)
#define DHCP_SERVER_STATE_ACK 				(3)
#define DHCP_SERVER_STATE_NAK 				(4)
#define DHCP_SERVER_STATE_IDLE 				(5)
#define DHCP_SERVER_STATE_RELEASE 		(6)
#define DHCP_SERVER_STATE_RENEWAL 		(7)




#define BOOTP_BROADCAST 				(0x8000)

#define DHCP_MESSAGE_OP_REQUEST        			(1)
#define DHCP_MESSAGE_OP_REPLY          			(2)

#define DHCP_MESSAGE_HTYPE 				(1)
#define DHCP_MESSAGE_HLEN  				(6)

#define DHCP_SERVER_PORT  				(67)
#define DHCP_CLIENT_PORT  				(68)

#define DHCP_MESSAGE_TYPE_DISCOVER  			(1)
#define DHCP_MESSAGE_TYPE_OFFER     			(2)
#define DHCP_MESSAGE_TYPE_REQUEST   			(3)
#define DHCP_MESSAGE_TYPE_DECLINE   			(4)
#define DHCP_MESSAGE_TYPE_ACK       			(5)
#define DHCP_MESSAGE_TYPE_NAK       			(6)
#define DHCP_MESSAGE_TYPE_RELEASE   			(7)

#define DHCP_OPTION_LENGTH_ONE				(1)
#define DHCP_OPTION_LENGTH_TWO				(2)
#define DHCP_OPTION_LENGTH_THREE			(3)
#define DHCP_OPTION_LENGTH_FOUR				(4)

#define DHCP_OPTION_CODE_SUBNET_MASK   			(1)
#define DHCP_OPTION_CODE_ROUTER        			(3)
#define DHCP_OPTION_CODE_DNS_SERVER    			(6)
#define DHCP_OPTION_CODE_INTERFACE_MTU 			(26)
#define DHCP_OPTION_CODE_BROADCAST_ADDRESS 		(28)
#define DHCP_OPTION_CODE_PERFORM_ROUTER_DISCOVERY 	(31)
#define DHCP_OPTION_CODE_REQUEST_IP_ADDRESS   		(50)
#define DHCP_OPTION_CODE_LEASE_TIME   			(51)
#define DHCP_OPTION_CODE_MSG_TYPE     			(53)
#define DHCP_OPTION_CODE_SERVER_ID    			(54)
#define DHCP_OPTION_CODE_REQ_LIST     			(55)
#define DHCP_OPTION_CODE_END         			(255)

#define IP_FREE_TO_USE		                	(1)
#define IP_ALREADY_IN_USE	                	(0)

#define HW_ADDRESS_LENGTH				(6)

typedef struct ip_addr_t
{
  uint32_t addr;
}ip_addr;

/* Reference by RFC 2131 */
typedef struct dhcps_msg_t
{
	uint8_t op; 		/* Message op code/message type. 1 = BOOTREQUEST, 2 = BOOTREPLY */
	uint8_t	htype;		/* Hardware address type */
	uint8_t hlen;		/* Hardware address length */
	uint8_t hops;		/* Client sets to zero, optionally used by relay agents 
				   when booting via a relay agent */
	uint8_t xid[4];		/* Transaction ID, a random number chosen by the client,
				   used by the client and server to associate messages and 
				   responses between a client and a server */
	uint16_t secs;		/* Filled in by client, seconds elapsed since client began address
				   acquisition or renewal process.*/
	uint16_t flags;		/* bit 0: Broadcast flag, bit 1~15:MBZ must 0*/
	uint8_t ciaddr[4];	/* Client IP address; only filled in if client is in BOUND,
				   RENEW or REBINDING state and can respond to ARP requests. */
	uint8_t yiaddr[4];	/* 'your' (client) IP address */
	uint8_t siaddr[4];	/* IP address of next server to use in bootstrap;
				   returned in DHCPOFFER, DHCPACK by server. */
	uint8_t giaddr[4];	/* Relay agent IP address, used in booting via a relay agent.*/
	uint8_t chaddr[16];	/* Client hardware address */
	uint8_t sname[64];	/* Optional server host name, null terminated string.*/
	uint8_t file[128];	/* Boot file name, null terminated string; "generic" name or
			           null in DHCPDISCOVER, fully qualified directory-path name in DHCPOFFER.*/
	uint8_t options[312];   /* Optional parameters field. reference the RFC 2132 */
}dhcps_msg;

/* use this to check whether the message is dhcp related or not */
static const uint8_t dhcp_magic_cookie[4] = {99, 130, 83, 99};
#if 0
// For Test
#if 0
// 30 seconds
static const uint8_t dhcp_option_lease_time_one_day[] = {0x00, 0x00, 0x00, 0x0A}; 
#else
// 1 minutes
static const uint8_t dhcp_option_lease_time_one_day[] = {0x00, 0x00, 0x00, 0x3C}; 
#endif
#else
// 24 hours
static const uint8_t dhcp_option_lease_time_one_day[] = {0x00, 0x01, 0x51, 0x80}; 
#endif
static const uint8_t dhcp_option_interface_mtu_576[] = {0x02, 0x40};

struct table {
	uint32_t ip_range[4];
  uint8_t chaddr[128][16];	/* Client hardware address */
#if 1
	// 20200605
  uint8_t cltime[128][4];	/* Client leased time */
#endif
};

struct address_pool{
	uint32_t start;
	uint32_t end;
};

/* 01~32 */
#define MARK_RANGE1_IP_BIT(table, ip)	((table.ip_range[0]) | (1 << ((ip) - 1)))	 
#define UNMARK_RANGE1_IP_BIT(table, ip)	((table.ip_range[0]) ^ (1 << ((ip) - 1)))	 

/* 33~64 */
#define MARK_RANGE2_IP_BIT(table, ip)	((table.ip_range[1]) | (1 << ((ip) - 1)))
#define UNMARK_RANGE2_IP_BIT(table, ip)	((table.ip_range[1]) ^ (1 << ((ip) - 1)))

/* 65~96 */
#define MARK_RANGE3_IP_BIT(table, ip)	((table.ip_range[2]) | (1 << ((ip) - 1)))
#define UNMARK_RANGE3_IP_BIT(table, ip)	((table.ip_range[2]) ^ (1 << ((ip) - 1)))

/* 97~128 */
#define MARK_RANGE4_IP_BIT(table, ip)	((table.ip_range[3]) | (1 << ((ip) - 1)))
#define UNMARK_RANGE4_IP_BIT(table, ip)	((table.ip_range[3]) ^ (1 << ((ip) - 1)))


/* expose API */
void dhcps_set_addr_pool(int addr_pool_set, ip_addr * addr_pool_start, ip_addr *addr_pool_end);
void dhcps_init(uint8_t s, uint8_t * buf);
static uint8_t search_mac();
uint8_t dhcps_run(void);
#if 1
// 20201028 taylor
void check_leased_time_over();
#endif
/*
 * @brief DHCP 1s Tick Timer handler
 * @note SHOULD BE register to your system 1s Tick timer handler 
 */
void dhcps_time_handler(void);

#ifdef __cplusplus
}
#endif

#endif

