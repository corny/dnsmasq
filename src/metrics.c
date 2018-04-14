#include "dnsmasq.h"
#include "metrics.h"

#ifdef HAVE_METRICS
#include <assert.h>
#include <pthread.h>

pthread_t thread;
int sock = -1;
char buffer[1024];
char *socketPath;

// forward declarations
void stopMetrics();
static void sendMetrics(int sock);
static void * responder(void *arg);


void startMetrics(const char *path) {
  socketPath = path;

  // build socket address
  struct sockaddr_un addr = {
    .sun_family = AF_UNIX
  };
  strlcpy(addr.sun_path, socketPath, sizeof(addr.sun_path));

  // open socket
  unlink(path);
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket() failed");
    return;
  }

  // bind
  if (!bind(sock, (struct sockaddr *)&addr, SUN_LEN(&addr))) {
    perror("bind() failed");
    goto abort;
  }

  // listen
  if (!listen(sock, 1)) {
    perror("listen() failed");
    goto abort;
  }

  // create responder thread
  int result_code = pthread_create(&thread, NULL, &responder, NULL);
  assert(!result_code);
  return;

abort:
  stopMetrics();
}

static void
stopMetrics() {
  if (sock >= 0) {
    close(sock);
    unlink(socketPath);
  }
}

// responds to socket connections
static void *
responder(void *arg){
  while (1) {
    int client = accept(sock, NULL, NULL);
    if (client < 0) {
      perror("accept() failed");
      break;
    }
    sendMetrics(client);
    close(client);
  }
  return arg;
}


// writes metrics to the given socket
static void
sendMetrics(int sock) {
  int pos = 0;

#define writeField(name, value) snprintf(buffer+pos, sizeof(buffer)-pos, name ": %d\n", metrics[i]); break;

  for(int i=0; i < COUNTER_MAX; i++) {
    switch (i) {
      case COUNTER_BOOTP:        writeField("BOOTP", i);
      case COUNTER_PXE:          writeField("PXE", i);
      case COUNTER_DHCPACK:      writeField("DHCPACK", i);
      case COUNTER_DHCPDECLINE:  writeField("DHCPDECLINE", i);
      case COUNTER_DHCPDISCOVER: writeField("DHCPDISCOVER", i);
      case COUNTER_DHCPINFORM:   writeField("DHCPINFORM", i);
      case COUNTER_DHCPNAK:      writeField("DHCPNAK", i);
      case COUNTER_DHCPOFFER:    writeField("DHCPOFFER", i);
      case COUNTER_DHCPRELEASE:  writeField("DHCPRELEASE", i);
      case COUNTER_DHCPREQUEST:  writeField("DHCPREQUEST", i);
      case COUNTER_NOANSWER:     writeField("NOANSWER", i);
    }
  }
#undef writeField

  if (send(sock, buffer, sizeof(buffer), 0) < 0)
      perror("send() failed");
}


#endif /* HAVE_METRICS */
