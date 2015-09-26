//Matthew Del Signore
//Pebble Pomodoro Timer App
#include <pebble.h>

Window *main_window;
TextLayer *timer_text;
int work = 0; //0 if it's a break, 1 if it's a work timer
int pomodoros = 0; //the number of pomdoros completed


//load and start the timer
void main_window_load(Window* window){
  timer_text = text_layer_create(GRect(0, 0, 144, 20));

  //make the text pretty
  text_layer_set_background_color(timer_text, GColorClear);
  text_layer_set_text_color(timer_text, GColorBlack);
  text_layer_set_text(timer_text, "00:00");
  text_layer_set_text_alignment(timer_text, GTextAlignmentCenter);
  text_layer_set_font(timer_text, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));


  layer_add_child(window_get_root_layer(window), text_layer_get_layer(timer_text));
}

static void main_window_unload(Window *window) {
    // Destroy timer text layer
    text_layer_destroy(timer_text);
}

void init(void) {
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
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
