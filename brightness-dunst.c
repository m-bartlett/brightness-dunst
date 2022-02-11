#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define MAX_BRIGHT 7500
#define BRIGHT_FILE_LITERAL "/sys/class/backlight/intel_backlight/brightness"

const uint16_t ONE_PERCENT = MAX_BRIGHT / 100;
const char *BRIGHT_FILE = BRIGHT_FILE_LITERAL;

int percentage_to_parabolic(int x) { return (uint32_t)(x * x * MAX_BRIGHT) / 10000; }
int parabolic_to_percentage(int y) { return sqrt(y*10000/MAX_BRIGHT); }


int main(int argc, char *argv[]) {

  FILE * file;
  int16_t current_brightness, new_brightness;
  file = fopen(BRIGHT_FILE, "r");

  if (!file) { fprintf( stderr, "Unreadable: %s\n", BRIGHT_FILE); return EXIT_FAILURE; }

  if (!fscanf(file, "%d", &current_brightness)) {
    fprintf( stderr, "Unparsable: %s\n", BRIGHT_FILE);
    return EXIT_FAILURE;
  }

  fclose(file);

  current_brightness = parabolic_to_percentage(current_brightness);

  if (argc > 1)  {
    int percent;
    char *arg = argv[1];
    sscanf(arg, "%d", &percent);
    // int16_t delta = percent * ONE_PERCENT;
    switch (arg[0]) {
      case '+': case '-': new_brightness = current_brightness + percent; break;
      default:            new_brightness = percent;
    }
  }
  else { printf("%d\n", current_brightness); fflush(stdout); return EXIT_SUCCESS; }

  if (new_brightness > 100) new_brightness = 100;
  else if (new_brightness < 2)   new_brightness = 2;

  printf("%d\n", new_brightness);
  new_brightness = percentage_to_parabolic(new_brightness);

  file = fopen(BRIGHT_FILE, "w");
  if (!file) { perror ("Write error"); return EXIT_FAILURE; }
  fprintf(file, "%d", new_brightness);
  fclose(file);

  return EXIT_SUCCESS;
}