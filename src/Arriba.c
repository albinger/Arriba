#include "pebble.h"


static Window *s_main_window;
static Layer *s_face_layer, *s_hands_layer;
static TextLayer *s_time_layer;
static int seconds, colon;
#define SETTINGS 0

// Possible messages received from the config page
enum {
  CONFIG_KEY_SECONDS = 0x0,
  CONFIG_KEY_COLON = 0x1,
};

//some useful constants
#define CENTERX 72
#define CENTERY 70
#define DIALRADIUS 64
#define SECONDRADIUS 64
#define DOTRADIUS 5
#define HOURLENGTH 40
#define MINUTELENGTH 69
#define COLORSSECOND 300
  
GPoint dialCenter, hourEndpoint, minuteEndpoint, secondCenter;

// The Watchface
static void face_layer_update(Layer *layer, GContext *ctx) {
  
  // draw outline uses the stroke color
  #ifdef PBL_COLOR
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
  #else
    graphics_context_set_stroke_color(ctx, GColorWhite);   
  #endif
  //graphics_context_set_stroke_width(ctx, 3);
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_antialiased(ctx, true);
    
  #else
    //graphics_draw_circle(ctx, dialCenter, DIALRADIUS+1);
    graphics_draw_circle(ctx, dialCenter, DIALRADIUS-1);
  #endif
  graphics_draw_circle(ctx, dialCenter, DIALRADIUS);
  graphics_context_set_fill_color(ctx, GColorWhite);   
  graphics_fill_circle(ctx, dialCenter, DOTRADIUS);
}

//The Hands and Digital Readout
static void hands_layer_update(Layer *layer, GContext *ctx) {
  struct tm *now;
  time_t t = time(NULL);
  now = localtime(&t);
	int32_t minuteAngle = now->tm_min * TRIG_MAX_ANGLE / 60;
	int32_t hourAngle = (now->tm_hour%12) * TRIG_MAX_ANGLE / 12;
  int32_t secondAngle = now->tm_sec * TRIG_MAX_ANGLE / 60;
  int tempY;
	#ifdef PBL_COLOR
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_width(ctx, 3);
  #endif
  // Create a long-lived buffer
  static char buffer[] = "00:00";
  // Write the current hours and minutes or day and month into the buffer
   switch(colon){
    case 0 :
     if(clock_is_24h_style() == true) {
       strftime(buffer, 5, "%H%M",now);
     }else{
       strftime(buffer, 5, "%I%M", now); 
     }
    break;
    case 1 :
      if(clock_is_24h_style() == true) {
        strftime(buffer, 6, "%H:%M", now);
      }else{
        strftime(buffer, 6, "%I:%M", now);
      }
    break;
    case 2:
      strftime(buffer, 5, "%d%m", now);
    break;
    case 3:
      strftime(buffer, 5, "%m%d", now);
    break;
  }
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  //calculate outer point of hands
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  if(seconds){
     #ifdef PBL_COLOR
       graphics_context_set_fill_color(ctx, GColorRed);
     
       tempY = CENTERY + SECONDRADIUS * sin_lookup((secondAngle + COLORSSECOND + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
       graphics_fill_circle(ctx,GPoint(
         CENTERX + SECONDRADIUS * cos_lookup((secondAngle + COLORSSECOND + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 122)?122:tempY),3);
       tempY = CENTERY + SECONDRADIUS * sin_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
       graphics_fill_circle(ctx,GPoint(
         CENTERX + SECONDRADIUS * cos_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 122)?122:tempY),3);
       tempY = CENTERY + SECONDRADIUS * sin_lookup((secondAngle - COLORSSECOND + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
       graphics_fill_circle(ctx,GPoint(
         CENTERX + SECONDRADIUS * cos_lookup((secondAngle - COLORSSECOND + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 122)?122:tempY),3);
     #else
       graphics_context_set_fill_color(ctx, GColorWhite); 
       tempY = CENTERY + SECONDRADIUS * sin_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
       graphics_fill_circle(ctx,GPoint(
         CENTERX + SECONDRADIUS * cos_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 122)?122:tempY),DOTRADIUS);
     #endif
   }
  
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 5);
  #endif
  graphics_draw_line(ctx, dialCenter,
                     GPoint(CENTERX + HOURLENGTH * cos_lookup((hourAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO,
                            CENTERY + HOURLENGTH * sin_lookup((hourAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO));
 
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 3);
  #endif 
  tempY = CENTERY + MINUTELENGTH * sin_lookup((minuteAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
  graphics_draw_line(ctx, dialCenter,
                      GPoint(CENTERX + MINUTELENGTH * cos_lookup((minuteAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 128)?128:tempY));   
}

void handle_tick(struct tm *now, TimeUnits units_changed) {
  layer_mark_dirty(s_hands_layer);
}

// Settings
void in_recv_handler(DictionaryIterator *received, void *context) {
  time_t t = time(NULL);
  int setstring = 0;
  APP_LOG(APP_LOG_LEVEL_INFO,"In Settings Handler");
  Tuple *seconds_tuple = dict_find(received, CONFIG_KEY_SECONDS);
  Tuple *colon_tuple = dict_find(received, CONFIG_KEY_COLON);
  

  if (seconds_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO,"seconds: %i",atoi(seconds_tuple->value->cstring));
    seconds = atoi(seconds_tuple->value->cstring);
    setstring = setstring + (1 * seconds);
  }
  if (colon_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO,"colon: %i",atoi(colon_tuple->value->cstring));
    colon= atoi(colon_tuple->value->cstring);
    setstring = setstring + (10 * colon);
  }
  
  APP_LOG(APP_LOG_LEVEL_INFO,"setstring %i", setstring);
  // save the settings in persistent memory
  persist_write_int(SETTINGS, setstring);
  // reset the tick_timer every time settings are updated
  tick_timer_service_unsubscribe();
  if(seconds){
	  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  }else{
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  }
  handle_tick(localtime(&t), SECOND_UNIT|MINUTE_UNIT|HOUR_UNIT);
}

static void main_window_load(Window *window) {
  s_face_layer = layer_create(GRect(0,0,144,122));
  s_hands_layer = layer_create(GRect(0,0,144,168));

  layer_set_update_proc(s_face_layer, face_layer_update);
  layer_set_update_proc(s_hands_layer, hands_layer_update);

  layer_add_child(window_get_root_layer(window), s_face_layer);
  layer_add_child(window_get_root_layer(window), s_hands_layer);

    s_time_layer = text_layer_create(GRect(0, 125, 144, 135));
  layer_set_clips((Layer *)s_time_layer, false);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  //this next bit might get really "heavy" when secondes are enabled.  I don't know.
  switch(colon){
    case 0 :
      text_layer_set_text(s_time_layer, "HHMM");
    break;
    case 1 :
      text_layer_set_text(s_time_layer, "HH:MM");
    break;
    case 2:
      text_layer_set_text(s_time_layer,"DDMM");
    break;
    case 3:
      text_layer_set_text(s_time_layer,"MMDD");
    break;
  }
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_insert_below_sibling(text_layer_get_layer(s_time_layer), s_hands_layer);
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  layer_destroy(s_face_layer);
  layer_destroy(s_hands_layer);
  text_layer_destroy(s_time_layer);
}

static void init() {
  time_t t = time(NULL);
  int my_settings;
	// Read Saved Settings
  my_settings = persist_exists(SETTINGS) ? persist_read_int(SETTINGS) : 00;
  APP_LOG(APP_LOG_LEVEL_INFO,"read persist string %i",my_settings);
  seconds = my_settings%10;
  colon = my_settings/10;
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Create Window
  s_main_window = window_create();
  #ifdef PBL_COLOR
    window_set_background_color(s_main_window, GColorOxfordBlue);
  #else
    window_set_background_color(s_main_window, GColorBlack);
  #endif
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });  
  window_stack_push(s_main_window, true);
  handle_tick(localtime(&t), SECOND_UNIT|MINUTE_UNIT|HOUR_UNIT);
  if(seconds){
	  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  }else{
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  }
  dialCenter = GPoint(CENTERX, CENTERY);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

