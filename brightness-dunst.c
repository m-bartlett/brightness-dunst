#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <libnotify/notify.h>
#include <stdbool.h>


/*
    TO-DO: Read icons and light/dark theme from Xresources
*/

#define BRIGHTNESS_MAX 7500
#define BRIGHT_FILE_LITERAL "/sys/class/backlight/intel_backlight/brightness"

const char *BRIGHT_FILE = BRIGHT_FILE_LITERAL;


#define ICON_PATH_BRIGHTNESS_LOW \
  "/usr/share/icons/Papirus-Dark/symbolic/status/display-brightness-low-symbolic.svg"
#define ICON_PATH_BRIGHTNESS_MEDIUM \
  "/usr/share/icons/Papirus-Dark/symbolic/status/display-brightness-medium-symbolic.svg"
#define ICON_PATH_BRIGHTNESS_HIGH \
  "/usr/share/icons/Papirus-Dark/symbolic/status/display-brightness-high-symbolic.svg"

#define NOTIFICATION_TIMEOUT_MS 1000
#define NOTIFICATION_CATEGORY_LITERAL    "brightness"
#define NOTIFICATION_BODY_FORMAT_LITERAL "<span>%s</span>"
#define SYNCHRONOUS_LITERAL              "synchronous"
#define NOTIFICATION_HINT_VALUE_LITERAL  "value"

const char *NOTIFICATION_BODY_FORMAT = NOTIFICATION_BODY_FORMAT_LITERAL;
size_t NOTIFICATION_BODY_FORMAT_LENGTH = strlen(NOTIFICATION_BODY_FORMAT);


int percentage_to_parabolic(int x) { return (uint32_t)(x * x * BRIGHTNESS_MAX) / 10000; }
int parabolic_to_percentage(int y) { return sqrt(y*10000/BRIGHTNESS_MAX); }


static void display_brightness(int percentage) {
  char *summary = (char*)NOTIFICATION_CATEGORY_LITERAL;
  char *icon;

  uint8_t brightness = percentage/33;
  switch(brightness) {
    case 0:  icon = (char*)ICON_PATH_BRIGHTNESS_LOW;    break;
    case 1:  icon = (char*)ICON_PATH_BRIGHTNESS_MEDIUM; break;
    case 2:
    default: icon = (char*)ICON_PATH_BRIGHTNESS_HIGH;   break;
  }

  char body[5];
  sprintf(body, "%d%", percentage);
  body[4]='\0';

  notify_init(summary);
  NotifyNotification *notification = notify_notification_new(summary, body, icon);
  notify_notification_set_category(notification, NOTIFICATION_CATEGORY_LITERAL);

  /*
    https://people.gnome.org/~desrt/glib-docs/glib-GVariant.html
    info on GVariants since notify_notification_set_hint_* methods are deprecated
  */

  // Set notification synchronous (overwrite existing notification instead of making a fresh one)
  notify_notification_set_hint( notification,
                                SYNCHRONOUS_LITERAL,
                                g_variant_new_string(NOTIFICATION_CATEGORY_LITERAL) );

  // Add 'value' hint for dunst progress bar to show
  notify_notification_set_hint( notification,
                                NOTIFICATION_HINT_VALUE_LITERAL,
                                g_variant_new_int32(percentage) );

  notify_notification_set_timeout(notification, NOTIFICATION_TIMEOUT_MS);
  notify_notification_show(notification, NULL);
  g_object_unref(notification);
  notify_uninit();
}


int main(int argc, char *argv[]) {
  FILE * file;
  int16_t current_brightness, new_brightness;
  file = fopen(BRIGHT_FILE, "r");

  if (!file) { fprintf(stderr, "Unreadable: %s\n", BRIGHT_FILE); return EXIT_FAILURE; }

  if (!fscanf(file, "%d", &current_brightness)) {
    fprintf(stderr, "Unparsable: %s\n", BRIGHT_FILE);
    return EXIT_FAILURE;
  }

  fclose(file);

  current_brightness = parabolic_to_percentage(current_brightness);

  if (argc > 1)  {
    int percent;
    char *arg = argv[1];
    sscanf(arg, "%d", &percent);
    switch (arg[0]) {
      case '+': case '-': new_brightness = current_brightness + percent; break;
      default:            new_brightness = percent;
    }
  }
  else {
    printf("%d\n", current_brightness);
    fflush(stdout);
    display_brightness(current_brightness);
    return EXIT_SUCCESS;
  }

  if (new_brightness > 100)    new_brightness = 100;
  else if (new_brightness < 2) new_brightness = 2;

  printf("%d\n", new_brightness);
  current_brightness = percentage_to_parabolic(new_brightness);

  file = fopen(BRIGHT_FILE, "w");
  if (!file) { perror ("Write error"); return EXIT_FAILURE; }
  fprintf(file, "%d", current_brightness);
  fclose(file);

  // Drop root privileges for accessing the user's D-Bus interface for libnotify
  if (geteuid() == 0) {
    if (setgid(getgid()) != 0 || setuid(getuid()) != 0) {
      fprintf(stderr, "Root-drop failed\n");

    }
  }

  display_brightness(new_brightness);

  return EXIT_SUCCESS;
}