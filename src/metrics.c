/* dnsmasq is Copyright (c) 2000-2018 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dnsmasq.h"

#ifdef HAVE_METRICS

volatile u32 metrics[__METRIC_MAX] = {0};

const char * metric_names[] = {
    "bootp",
    "pxe",
    "dhcp_ack",
    "dhcp_decline",
    "dhcp_discover",
    "dhcp_inform",
    "dhcp_nak",
    "dhcp_offer",
    "dhcp_release",
    "dhcp_request",
    "noanswer",
    "leases_allocated",
};

const char* get_metric_name(int i) {
    return metric_names[i];
}

#endif /* HAVE_METRICS */
