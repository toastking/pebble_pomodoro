//Matthew Del Signore
//Pebble Pomodoro Timer App
#include <pebble.h>
  
//keys for persistent data store
//TODO: switch these to enums
#define WORK_KEY 1
#define POMODOROS_KEY 2
#define RUNNING_KEY 3
#define S_TIME_KEY 4
#define DATE_KEY 5

   
//timer lengths (in minutes)
static int s_work_time = 25;
static int s_rest_time = 5;

//stuff for the timer window
static Window *main_window;
static TextLayer *timer_text;
static TextLayer *s_prompt;
static BitmapLayer *tomato_layer; //used to display the tomato images
static int work = 1; //0 if it's a break, 1 if it's a work timer
static int pomodoros = 0; //the number of pomdoros completed
static int running = 0; //0 if the timer isn't running, 1 if it is
static int s_timer=1*60; // the timer used for the pomodoro timer
//load the image of the tomato
static GBitmap *s_tomato;
static char* s_work_text = "Work!";
static char* s_rest_text = "Rest!";
static char* s_date = "00/00/00";




//method to handle when a pomodoro is finished
static void pomodoro_finished(){
  //reset the timer
  s_timer = 0;
  vibes_short_pulse();
  
  //check if it was a work or rest period
  if(work == 1){
    //set it to a rest period
    work = 0;
    
    s_timer = s_rest_time * 60;
    
    int y = 50+((pomodoros / 5))*30; 
    int x = (pomodoros%5)*30;
    //add a tomato to the screen
    tomato_layer = bitmap_layer_create(GRect(x, y, 20, 20));
    bitmap_layer_set_bitmap(tomato_layer, s_tomato);
    layer_add_child(window_get_root_layer(main_window), bitmap_layer_get_layer(tomato_layer));
    
    //update the pomodoro timer
    pomodoros++;
    //chage the text on the bottom
    text_layer_set_text(s_prompt, s_rest_text);
    
    //display the article window
    //window_stack_push(s_article_window, true);
  }else{
    if(work == 0){
      //set it to a work period
      work = 1; 
      s_timer = s_work_time * 60;
      //chage the text on the bottom
      text_layer_set_text(s_prompt, s_work_text);
    }
  }
}
static void redraw(){
 static char buffer[] = "00m00s";
  
  int seconds = s_timer % 60;
  int minutes = s_timer / 60;
  
  // display the timer
  snprintf(buffer, sizeof("00m00s"), "%dm%ds", minutes,seconds);
  
  // Display this time on the TextLayer
  text_layer_set_text(timer_text, buffer);  
}

//function called to update the timer
static void update_time() {
  // Create a long-lived buffer
  static char buffer[] = "00m00s";
  
  int seconds = s_timer % 60;
  int minutes = s_timer / 60;
  
  //decrement the timer
  s_timer--;
  
  // display the timer
  snprintf(buffer, sizeof("00m00s"), "%dm%ds", minutes,seconds);
  
  // Display this time on the TextLayer
  text_layer_set_text(timer_text, buffer);
  
  //check if the timer is done
  if (s_timer < 0){
    pomodoro_finished();
  }
}

//event handler to update the timer text
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  //check for the time to see if a day has passed
  char* date = "00/00/00";
  
  if(strcmp(s_date, "00/00/00")!=0){
    strftime(date, sizeof("00/00/00"), "%D", tick_time);
  }else{
    //otherwise write to the date string
    strftime(s_date, sizeof("00/00/00"), "%D", tick_time);
  }
    
  //check if it's a new day
  if (strcmp(s_date, date)==0){
    //reset the number of pomodors
    APP_LOG(APP_LOG_LEVEL_DEBUG, "inside conditional");
    pomodoros = 0;
  }
}

//Event Handlers to start and pause the timer
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //start or stop the ticktimerservice to "pause" the timer
  //update the timer every second
  if(running == 0){
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    running = 1;
  }else{
    //update the timer every second
    tick_timer_service_unsubscribe();
    running = 0;
  }
}

//button to reset the timer
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //reset all the fields
  //work = 1; //0 if it's a break, 1 if it's a work timer
  //pomodoros = 0; //the number of pomdoros completed
  running = 0; //0 if the timer isn't running, 1 if it is
  if(work == 1){
    s_timer=s_work_time*60; // the timer used for the pomodoro timer
  }else{
      s_timer=s_rest_time*60; // the timer used for the pomodoro timer
  }
  redraw();//redraw the timer
}



static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
}

//draw the pomodoros
static void draw_tomatoes(){
  for(int i =0; i<pomodoros;i++){
    int temp_y = 50+((i / 5))*30; 
    int temp_x = (i%5)*30;
    //add a tomato to the screen
    tomato_layer = bitmap_layer_create(GRect(temp_x, temp_y, 20, 20));
    bitmap_layer_set_bitmap(tomato_layer, s_tomato);
    layer_add_child(window_get_root_layer(main_window), bitmap_layer_get_layer(tomato_layer));
  }
}

//load and start the timer
static void main_window_load(Window* window){
  timer_text = text_layer_create(GRect(0, 0, 144, 50)); //make the window the size of the pebbles screen
  s_prompt = text_layer_create(GRect(0, 110, 144, 28)); //make the window the size of the pebbles screen
  //make the text pretty
  text_layer_set_background_color(timer_text, GColorClear);
  text_layer_set_text_color(timer_text, GColorBlack);
  text_layer_set_text(timer_text, "00m00s");
  text_layer_set_background_color(s_prompt, GColorClear);
  text_layer_set_text_color(s_prompt, GColorBlack);
  text_layer_set_text(s_prompt, s_work_text);
  text_layer_set_text_alignment(timer_text, GTextAlignmentCenter);
  text_layer_set_font(timer_text, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_prompt, GTextAlignmentCenter);
  text_layer_set_font(s_prompt, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));

  //add the text layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(timer_text));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_prompt));
}

static void main_window_unload(Window *window) {
    // Destroy timer text layer
    text_layer_destroy(timer_text);
    text_layer_destroy(s_prompt);
}


void init(void) {
  //load variables from the persistent data store
  if (persist_exists(WORK_KEY)) {
    work = persist_read_int(WORK_KEY); //0 if it's a break, 1 if it's a work timer
  }
  if (persist_exists(POMODOROS_KEY)) {
   pomodoros = persist_read_int(POMODOROS_KEY); //the number of pomdoros completed
  }
  if (persist_exists(RUNNING_KEY)) {
   running = persist_read_int(RUNNING_KEY); //0 if the timer isn't running, 1 if it is
  }
  if (persist_exists(S_TIME_KEY)) {
    s_timer=persist_read_int(S_TIME_KEY); // the timer used for the pomodoro timer
  }
   if (persist_exists(DATE_KEY)) {
    s_timer=persist_read_string(DATE_KEY, s_date, sizeof("00/00/00")); // the string used for the date
  }
  
  //keeps track of the location of the pomodoros
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_stack_push(main_window, true);
  
  s_tomato = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TOMATO);
  
   //add the click event handlers
  window_set_click_config_provider(main_window, click_config_provider);
  
  //draw the tomatoes
  draw_tomatoes();
  
   //redraw the timer view
  redraw();
  
  //redraw the text
  if(work == 0){
    //chage the text on the bottom
    text_layer_set_text(s_prompt, s_rest_text);
  }else{
    //chage the text on the bottom
    text_layer_set_text(s_prompt, s_work_text);
  }

}

void handle_deinit(void) {
  gbitmap_destroy(s_tomato);
  bitmap_layer_destroy(tomato_layer);
  window_destroy(main_window);
  
  persist_write_int(WORK_KEY, work);
  persist_write_int(POMODOROS_KEY, pomodoros);
  persist_write_int(RUNNING_KEY, running);
  persist_write_int(S_TIME_KEY, s_timer);
  persist_write_string(DATE_KEY, s_date);
  
}

int main(void) {
  init();
  app_event_loop();
  handle_deinit();
}
