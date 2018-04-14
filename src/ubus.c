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

#ifdef HAVE_UBUS

#include <libubus.h>

static struct ubus_context *ubus;
static struct blob_buf b;

#ifdef HAVE_METRICS
static int ubus_handle_metrics(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
  blob_buf_init(&b, 0);

  for(int i=0; i < COUNTER_MAX; i++) {
    switch (i) {
      case COUNTER_BOOTP:        blobmsg_add_u32(&b, "bootp", metrics[i]); break;
      case COUNTER_PXE:          blobmsg_add_u32(&b, "pxe", metrics[i]); break;
      case COUNTER_DHCPACK:      blobmsg_add_u32(&b, "dhcpack", metrics[i]); break;
      case COUNTER_DHCPDECLINE:  blobmsg_add_u32(&b, "dhcpdecline", metrics[i]); break;
      case COUNTER_DHCPDISCOVER: blobmsg_add_u32(&b, "dhcpdiscover", metrics[i]); break;
      case COUNTER_DHCPINFORM:   blobmsg_add_u32(&b, "dhcpinform", metrics[i]); break;
      case COUNTER_DHCPNAK:      blobmsg_add_u32(&b, "dhcpnak", metrics[i]); break;
      case COUNTER_DHCPOFFER:    blobmsg_add_u32(&b, "dhcpoffer", metrics[i]); break;
      case COUNTER_DHCPRELEASE:  blobmsg_add_u32(&b, "dhcprelease", metrics[i]); break;
      case COUNTER_DHCPREQUEST:  blobmsg_add_u32(&b, "dhcprequest", metrics[i]); break;
      case COUNTER_NOANSWER:     blobmsg_add_u32(&b, "noanswer", metrics[i]); break;
    }
  }

  ubus_send_reply(ctx, req, b.head);

  return 0;
}
#endif

static struct ubus_method ubus_object_methods[] = {
#ifdef HAVE_METRICS
	{.name = "metrics", .handler = ubus_handle_metrics},
#endif
};

static struct ubus_object_type ubus_object_type = UBUS_OBJECT_TYPE("dnsmasq", ubus_object_methods);

static struct ubus_object ubus_object = {
	.name = "dnsmasq",
	.type = &ubus_object_type,
};

void ubus_event_bcast(const char *type, const char *mac, const char *ip, const char *name, const char *interface)
{
	if (!ubus || !ubus_object.has_subscribers)
		return;

	blob_buf_init(&b, 0);
	if (mac)
		blobmsg_add_string(&b, "mac", mac);
	if (ip)
		blobmsg_add_string(&b, "ip", ip);
	if (name)
		blobmsg_add_string(&b, "name", name);
	if (interface)
		blobmsg_add_string(&b, "interface", interface);
	ubus_notify(ubus, &ubus_object, type, b.head, -1);
}

static void set_ubus_listeners(void)
{
	if (!ubus)
		return;

	poll_listen(ubus->sock.fd, POLLIN);
	poll_listen(ubus->sock.fd, POLLERR);
	poll_listen(ubus->sock.fd, POLLHUP);
}

static void check_ubus_listeners()
{
	if (!ubus) {
		ubus = ubus_connect(NULL);
		if (ubus)
			ubus_add_object(ubus, &ubus_object);
		else
			return;
	}

	if (poll_check(ubus->sock.fd, POLLIN))
		ubus_handle_event(ubus);

	if (poll_check(ubus->sock.fd, POLLHUP)) {
		ubus_free(ubus);
		ubus = NULL;
	}
}

#endif
