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

static struct ubus_object_type ubus_object_type = {
	.name = "dnsmasq",
};

static struct ubus_object ubus_object = {
	.name = "dnsmasq",
	.type = &ubus_object_type,
};

void set_ubus_listeners()
{
	if (!ubus)
		return;

	poll_listen(ubus->sock.fd, POLLIN);
	poll_listen(ubus->sock.fd, POLLERR);
	poll_listen(ubus->sock.fd, POLLHUP);
}

void check_ubus_listeners()
{
	if (!ubus) {
		ubus = ubus_connect(NULL);
		if (!ubus)
			return;
		ubus_add_object(ubus, &ubus_object);
	}

	if (poll_check(ubus->sock.fd, POLLIN))
		ubus_handle_event(ubus);

	if (poll_check(ubus->sock.fd, POLLHUP)) {
		ubus_free(ubus);
		ubus = NULL;
	}
}

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


#endif /* HAVE_UBUS */
