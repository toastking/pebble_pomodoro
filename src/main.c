//Matthew Del Signore
//Pebble Pomodoro Timer App
#include <pebble.h>

Window *main_window;
TextLayer *timer_text;
int work = 0; //0 if it's a break, 1 if it's a work timer
int pomodoros = 0; //the number of pomdoros completed


//load and start the timer
void main_window_load(Window* window){
  timer_text = text_layer_create(GRect(0, 0, 144, 168)); //make the window the size of the pebbles screen

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

//function called to update the timer
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Use 12 hour format
  strftime(buffer, sizeof("00:00"), "%M:%S", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(timer_text, buffer);
}


//event handler to update the timer text
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


void init(void) {
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  //update the timer every second
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  window_stack_push(main_window, true);
  
  //update the timer value then the application starts 
  update_time();
}


void handle_deinit(void) {
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  handle_deinit();
}
