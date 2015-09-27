//Matthew Del Signore
//Pebble Pomodoro Timer App
#include <pebble.h>
  
//keys for persistent data store
//TODO: switch these to enums
#define WORK_KEY 1
#define POMODOROS_KEY 2
#define RUNNING_KEY 3
#define S_TIME_KEY 4
#define ARTICLE_TEXT 5
#define ARTICLE_TITLE 6

   
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
static int s_timer=60; // the timer used for the pomodoro timer
//load the image of the tomato
static GBitmap *s_tomato;
static char* s_work_text = "Work!";
static char* s_rest_text = "Rest!";

//stuff for the buzzfeed article window
static Window *s_article_window;
static ScrollLayer *s_article_scroll;
static TextLayer *s_article_title;
static TextLayer *s_article_text;
static char s_article_buffer[80];
static char s_article_title_buffer[80];

//callbacks for the app message
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  //TODO: handle the article text
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case ARTICLE_TEXT:
      snprintf(s_article_buffer, sizeof(s_article_buffer), "%s", t->value->cstring);
      break;
    case ARTICLE_TITLE:
      snprintf(s_article_title_buffer, sizeof(s_article_title_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
    text_layer_set_text(s_article_text, s_article_buffer);
    text_layer_set_text(s_article_title, s_article_title_buffer);
  }
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


//load and unload article view
static void article_window_load(Window* window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect max_text_bounds = GRect(0, 0, bounds.size.w, 2000);

  // Initialize the scroll layer
  s_article_scroll = scroll_layer_create(bounds);

  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(s_article_scroll, window);

  // Initialize the text layer
  s_article_text = text_layer_create(max_text_bounds);
  s_article_title = text_layer_create(max_text_bounds);
  //s_article_title = text_layer_create();
  text_layer_set_text(s_article_text, "loading...");

  // Change the font to a nice readable one
  // This is system font; you can inspect pebble_fonts.h for all system fonts
  // or you can take a look at feature_custom_font to add your own font
  text_layer_set_font(s_article_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  // Trim text layer and scroll content to fit text box
  GSize max_size = text_layer_get_content_size(s_article_text);
  text_layer_set_size(s_article_text, max_size);
  scroll_layer_set_content_size(s_article_scroll, GSize(bounds.size.w, max_size.h + 4));

  // Add the layers for display
  scroll_layer_add_child(s_article_scroll, text_layer_get_layer(s_article_text));
  
  //addi it to the root layer of the window
  layer_add_child(window_layer, scroll_layer_get_layer(s_article_scroll));
}

static void article_window_unload(Window *window) {
  text_layer_destroy(s_article_text);
  text_layer_destroy(s_article_title);
  scroll_layer_destroy(s_article_scroll);
}

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
    window_stack_push(s_article_window, true);
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
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 5 == 0) {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
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
  work = 1; //0 if it's a break, 1 if it's a work timer
  pomodoros = 0; //the number of pomdoros completed
  running = 0; //0 if the timer isn't running, 1 if it is
  s_timer=s_work_time*60; // the timer used for the pomodoro timer
  redraw();//redraw the timer
}

//button to reset the timer
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //display the article window
    window_stack_push(s_article_window, true);

}


static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
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
  
  //initialize the article window
  s_article_window = window_create();
  
  window_set_window_handlers(s_article_window, (WindowHandlers) {
    .load = article_window_load,
    .unload = article_window_unload
  });
  
  // add callbacks for the web communication
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
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
  
}

int main(void) {
  init();
  app_event_loop();
  handle_deinit();
}
