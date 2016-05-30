#include "uip-ds6-route.h"
/*struct for address list*/
struct  address_elem {
	struct address_elem* next;
	uip_ipaddr_t address;
};

