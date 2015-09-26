//Matthew Del Signore
//Pebble Pomodoro Timer App
#include <pebble.h>

Window *main_window;
TextLayer *timer_text;
int work = 0; //0 if it's a break, 1 if it's a work timer
int pomodoros = 0; //the number of pomdoros completed

void init(void) {
  main_window = window_create();

  timer_text = text_layer_create(GRect(0, 0, 144, 20));
  window_stack_push(main_window, true);
}

void handle_deinit(void) {
  text_layer_destroy(timer_text);
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  handle_deinit();
}
