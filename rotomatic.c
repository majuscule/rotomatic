#include <linux/input.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "findpowermate.h"
#include <libnotify/notify.h>
#include <stdlib.h>
#include <sys/time.h>

int abs_offset = 0;
time_t button_down;
time_t time_pressed;
int muted = 0;

void process_event(struct input_event *ev)
{
#ifdef VERBOSE
  fprintf(stderr, "type=0x%04x, code=0x%04x, value=%d\n",
	  ev->type, ev->code, (int)ev->value);
#endif

  switch(ev->type){
  case EV_MSC:
    printf("The LED pulse settings were changed; code=0x%04x, value=0x%08x\n", ev->code, ev->value);
    break;
  case EV_REL:
    if(ev->code != REL_DIAL)
      fprintf(stderr, "Warning: unexpected rotation event; ev->code = 0x%04x\n", ev->code);
    else {
      int difference = (int)ev->value;
      abs_offset += difference;
      if (button_down) break;
      if (difference > 0) {
          NotifyNotification * ico =
              notify_notification_new(" ", " ", "/usr/share/icons/powermate/volume-up.png");
          notify_notification_show (ico, NULL);
          FILE *output = popen("amixer set Master 1+", "r");
          pclose(output);
      } else {
          NotifyNotification * ico =
              notify_notification_new(" ", " ", "/usr/share/icons/powermate/volume-down.png");
          notify_notification_show (ico, NULL);
          FILE *output = popen("amixer set Master 1-", "r");
          pclose(output);
      }
    }
    break;
  case EV_KEY:
    if(ev->code != BTN_0)
      fprintf(stderr, "Warning: unexpected key event; ev->code = 0x%04x\n", ev->code);
    else {
      muted = !muted;
      if (ev->value) {
          button_down = time(NULL);
          abs_offset = 0;
      } else {
        if (abs_offset > 2) {
          system("/usr/bin/next");
        } else if (abs_offset < -2) {
          system("/usr/bin/back");
        } else if (time(NULL) - button_down > 1) {
          if (muted) {
            NotifyNotification * ico =
                notify_notification_new(" ", " ", "/usr/share/icons/powermate/volume-mute.png");
            notify_notification_show (ico, NULL);
          } else {
            NotifyNotification * ico =
                notify_notification_new(" ", " ", "/usr/share/icons/powermate/volume-on.png");
            notify_notification_show (ico, NULL);
          }
          FILE *output = popen("amixer set Master toggle", "r");
          pclose(output);
        } else { system("/usr/bin/music-toggle"); }
        button_down = 0;
      }
    }
    break;
  }

  fflush(stdout);
}

#define BUFFER_SIZE 32
void watch_powermate(int fd)
{
  struct input_event ibuffer[BUFFER_SIZE];
  int r, events, i;

  while(1){
    r = read(fd, ibuffer, sizeof(struct input_event) * BUFFER_SIZE);
    if( r > 0 ){
      events = r / sizeof(struct input_event);
      for(i=0; i<events; i++)
	process_event(&ibuffer[i]);
    }else{
      fprintf(stderr, "read() failed: %s\n", strerror(errno));
      return;
    }
  }
}

int main(int argc, char *argv[])
{
  int powermate = -1;

  if(argc == 1)
    powermate = find_powermate(O_RDONLY);
  else
    powermate = open_powermate(argv[1], O_RDONLY);

  if(powermate < 0){
    fprintf(stderr, "Unable to locate powermate\n");
    fprintf(stderr, "Try: %s [device]\n", argv[0]);
    return 1;
  }

  notify_init("powermate");
  watch_powermate(powermate);

  close(powermate);

  return 0;
}
