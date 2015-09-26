//Matthew Del Signore
//Pebble Pomodoro Timer App
#include <pebble.h>

static Window *main_window;
static TextLayer *timer_text;
static BitmapLayer *tomato_layer; //used to display the tomato images
static int work = 1; //0 if it's a break, 1 if it's a work timer
static int pomodoros = 0; //the number of pomdoros completed
static int running = 0; //0 if the timer isn't running, 1 if it is

//timer lengths (in minutes)
static int s_work_time = 1;
static int s_rest_time = 1;
static int s_timer=0; // the timer used for the pomodoro timer
//keeps track of the location of the pomodoros
static int x = 0;
static int y = 55;

//load the image of the tomato
static GBitmap *s_tomato;

//load and start the timer
static void main_window_load(Window* window){
  timer_text = text_layer_create(GRect(0, 0, 144, 50)); //make the window the size of the pebbles screen

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


//method to handle when a pomodoro is finished
static void pomodoro_finished(){
  //reset the timer
  s_timer = 0;
  
  //check if it was a work or rest period
  if(work == 1){
    //update the pomodoro timer
    pomodoros++;
    //set it to a rest period
    work = 0;
    
    y = 50+((pomodoros / 5))*30; 
    //add a tomato to the screen
    tomato_layer = bitmap_layer_create(GRect(x, y, 20, 20));
    bitmap_layer_set_bitmap(tomato_layer, s_tomato);
    layer_add_child(window_get_root_layer(main_window), bitmap_layer_get_layer(tomato_layer));
    x += 30;
  }else{
    if(work == 0){
      //set it to a work period
      work = 1; 
    }
  }
}

//function called to update the timer
static void update_time() {
  // Create a long-lived buffer
  static char buffer[] = "00m00s";
  
  int seconds = s_timer % 60;
  int minutes = (s_timer % 3600) / 60;
  
  //increment the timer
  s_timer++;
  
  // display the timer
  snprintf(buffer, sizeof("00m00s"), "%dm%ds", minutes,seconds);
  
  // Display this time on the TextLayer
  text_layer_set_text(timer_text, buffer);
  
  //check if the timer is done
  int timer_end = (work == 0) ? s_rest_time : s_work_time; //the timer end value
  if (minutes == timer_end){
    pomodoro_finished();
  }
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
  
  s_tomato = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TOMATO);
  
  //update the timer value then the application starts 
  update_time();
}

void handle_deinit(void) {
  gbitmap_destroy(s_tomato);
  bitmap_layer_destroy(tomato_layer);
  window_destroy(main_window);
  
}

int main(void) {
  init();
  app_event_loop();
  handle_deinit();
}
