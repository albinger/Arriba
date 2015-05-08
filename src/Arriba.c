#include "pebble.h"
#include "Arriba.h"
#include <ctype.h>

static Window *s_main_window;
static Layer *s_face_layer, *s_hands_layer;
static TextLayer *s_time_layer;
static int seconds, colon, tap;
static char buffer[] = "00:00";
static GColor Background, Foreground;
static GPoint dialCenter;
  //, hourEndpoint, minuteEndpoint, secondCenter;

#ifdef PBL_COLOR
	char basalt_colors[13];
#endif
    

#ifdef PBL_COLOR

  
  // HexStringToUInt borrowed from PebbleAuth https://github.com/JumpMaster/PebbleAuth.git
unsigned int HexStringToUInt(char const* hexstring){
	unsigned int result = 0;
	char const *c = hexstring;
	unsigned char thisC;

	while( (thisC = *c) != 0 ){
		thisC = toupper(thisC);
		result <<= 4;

		if( isdigit(thisC))
				result += thisC - '0';
		else if(isxdigit(thisC))
				result += thisC - 'A' + 10;
		else{
				APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: Unrecognised hex character \"%c\"", thisC);
				return 0;
		}
		++c;
	}
	return result;  
}

  // set_display_colors borrowed from PebbleAuth https://github.com/JumpMaster/PebbleAuth.git
void set_display_colors() {

	char str_color1[7];
	char str_color2[7];

	if (strlen(basalt_colors) == 12) {
		memcpy(str_color1, &basalt_colors[0], 6);
		memcpy(str_color2, &basalt_colors[6], 6);
		str_color1[6] = '\0';
		str_color2[6] = '\0';
	}else
			return;

	int color1 = HexStringToUInt(str_color1);
	int color2 = HexStringToUInt(str_color2);

	Background = GColorFromHEX(color1);
	Foreground = GColorFromHEX(color2);
  window_set_background_color(s_main_window, Background);
  text_layer_set_text_color(s_time_layer, Foreground);
    
}

#else

	void set_display_colors() { 
    Background = GColorBlack;
		Foreground = GColorWhite;		
}
	
#endif

// The Watchface
static void face_layer_update(Layer *layer, GContext *ctx) {
  
  // draw outline the stroke color
  graphics_context_set_stroke_color(ctx, Foreground);
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_antialiased(ctx, true);
    
  #else
    graphics_draw_circle(ctx, dialCenter, DIALRADIUS-1);
  #endif
  graphics_draw_circle(ctx, dialCenter, DIALRADIUS);
  graphics_context_set_fill_color(ctx, Foreground);   
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
  
  //calculate outer point of hands
  graphics_context_set_stroke_color(ctx, Foreground);
  graphics_context_set_fill_color(ctx, Foreground);
  
  if(seconds){
     #ifdef PBL_COLOR
       graphics_context_set_fill_color(ctx, Background);
       //graphics_context_set_fill_color(ctx, GColorWhite); 
       tempY = CENTERY + SECONDRADIUS * sin_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
       graphics_fill_circle(ctx,GPoint(
         CENTERX + SECONDRADIUS * cos_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 122)?122:tempY),DOTRADIUS);
       graphics_draw_circle(ctx,GPoint(
         CENTERX + SECONDRADIUS * cos_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 122)?122:tempY),DOTRADIUS);
     #else
       graphics_context_set_fill_color(ctx, Foreground); 
       tempY = CENTERY + SECONDRADIUS * sin_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
       graphics_fill_circle(ctx,GPoint(
         CENTERX + SECONDRADIUS * cos_lookup((secondAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 122)?122:tempY),DOTRADIUS);
     #endif
   }
  
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 5);
  #endif
  graphics_draw_line(ctx, dialCenter,
                     GPoint(CENTERX + HOURLENGTH * cos_lookup(((hourAngle+91*now->tm_min) + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO,
                            CENTERY + HOURLENGTH * sin_lookup(((hourAngle+91*now->tm_min) + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO));
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 3);
  #endif 
  tempY = CENTERY + MINUTELENGTH * sin_lookup((minuteAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO;
  graphics_draw_line(ctx, dialCenter,
                      GPoint(CENTERX + MINUTELENGTH * cos_lookup((minuteAngle + 3*TRIG_MAX_ANGLE/4)%TRIG_MAX_ANGLE) / TRIG_MAX_RATIO, (tempY > 130)?130:tempY));   
}

void handle_tick(struct tm *now, TimeUnits units_changed) {
  // Create a long-lived buffer
  
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
    case 4:
     if(tap){
       strftime(buffer, 6, "%d.%m", now);
     }else{
       if(clock_is_24h_style() == true) {
         strftime(buffer, 6, "%H:%M",now);
       }else{
         strftime(buffer, 6, "%I:%M", now); 
       } 
     }
    break;
    case 5:
     if(tap){
       strftime(buffer, 6, "%m.%d", now);
     }else{
       if(clock_is_24h_style() == true) {
         strftime(buffer, 6, "%H:%M",now);
       }else{
         strftime(buffer, 6, "%I:%M", now); 
       } 
     }
    break; 
  }
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  layer_mark_dirty(s_hands_layer);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  struct tm *now;
  time_t t = time(NULL);
  now = localtime(&t);
  APP_LOG(APP_LOG_LEVEL_INFO,"tap handler");
  
  if(tap){
    tap =0;
    if(clock_is_24h_style() == true) {
       strftime(buffer, 6, "%H:%M",now);
     }else{
       strftime(buffer, 6, "%I:%M", now); 
     }
  }else{
    tap = 1;
    switch(colon){
      case 4:
          strftime(buffer, 6, "%d.%m", now);
      break;
      case 5:
        strftime(buffer, 6, "%m.%d", now);
      break; 
    }
  }
  text_layer_set_text(s_time_layer, buffer);
}


// Settings
void in_recv_handler(DictionaryIterator *received, void *context) {
  time_t t = time(NULL);
  int setstring = 0;
  APP_LOG(APP_LOG_LEVEL_INFO,"In Settings Handler");
  Tuple *seconds_tuple = dict_find(received, CONFIG_KEY_SECONDS);
  Tuple *colon_tuple = dict_find(received, CONFIG_KEY_COLON);
  #ifdef PBL_COLOR
		Tuple *basalt_colors_tuple = dict_find(received,CONFIG_KEY_BASALT_COLORS);
	#endif

  if (seconds_tuple) {
    //seconds = atoi(seconds_tuple->value->cstring);
    seconds = seconds_tuple->value->int8;
    APP_LOG(APP_LOG_LEVEL_INFO,"seconds:%i",seconds);
    setstring = setstring + (1 * seconds);
  }
  if (colon_tuple) {
    colon = colon_tuple->value->int8;
    APP_LOG(APP_LOG_LEVEL_INFO,"colon: %i",colon);
    setstring = setstring + (10 * colon);
  }
  
  APP_LOG(APP_LOG_LEVEL_INFO,"setstring %i", setstring);
  // save the settings in persistent memory
  persist_write_int(SETTINGS, setstring);
  // reset the tick_timer every time settings are updated
  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  if(seconds ==1){
	  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  }else{
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  }
  if(colon == 4 || colon == 5){
    accel_tap_service_subscribe(tap_handler);
  }
  handle_tick(localtime(&t), SECOND_UNIT|MINUTE_UNIT|HOUR_UNIT);
  
  #ifdef PBL_COLOR
		
		if (basalt_colors_tuple) {	
			memcpy(basalt_colors, basalt_colors_tuple->value->cstring, basalt_colors_tuple->length);
		  APP_LOG(APP_LOG_LEVEL_DEBUG, "INFO: basalt_colors: %s", basalt_colors);
			persist_write_string(PS_BASALT_COLORS, basalt_colors);
		}		
	#endif

  set_display_colors();
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
  text_layer_set_text_color(s_time_layer, Foreground);
  //this next bit might get really "heavy" when secondes are enabled.  I don't know. 
  //so far it seems to not affect battery usage much.  Yay compilers!
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
    case 4:
      text_layer_set_text(s_time_layer,"DD.MM");
    break;
    case 5:
      text_layer_set_text(s_time_layer,"MM.DD");
    break;
  }
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_insert_below_sibling(text_layer_get_layer(s_time_layer), s_hands_layer);
}

static void main_window_unload(Window *window) {
  
  tick_timer_service_unsubscribe();
  if(colon == 4 || colon == 5) {accel_tap_service_unsubscribe();}
  layer_destroy(s_face_layer);
  layer_destroy(s_hands_layer);
  text_layer_destroy(s_time_layer);
}

static void init() {
  #ifdef PBL_COLOR
    if (persist_exists(PS_BASALT_COLORS)){
			persist_read_string(PS_BASALT_COLORS, basalt_colors, 13);
      APP_LOG(APP_LOG_LEVEL_INFO,"saved color:%s",basalt_colors);
  }else{
			APP_LOG(APP_LOG_LEVEL_INFO,"setting default colors");
      memcpy(basalt_colors, "000055FFFFFF"+'\0', 13);
  }
  
  #else
  Background = GColorBlack;
  Foreground = GColorWhite;
  #endif
  time_t t = time(NULL);
  int my_settings;
	// Read Saved Settings for seconds hand and digital display
  my_settings = persist_exists(SETTINGS) ? persist_read_int(SETTINGS) : 00;
  APP_LOG(APP_LOG_LEVEL_INFO,"read persist string %i",my_settings);
  seconds = my_settings%10;
  colon = my_settings/10;
  if(colon == 4 || colon == 5) {accel_tap_service_subscribe(tap_handler);}
  app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Create Window
  s_main_window = window_create();
  #ifdef PBL_COLOR
    window_set_background_color(s_main_window, Background);
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
  tap=1;
  dialCenter = GPoint(CENTERX, CENTERY);
  set_display_colors();
}

static void deinit() {
  
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

