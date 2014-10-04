#include <pebble.h>
  
static Window *s_main_window;
static GFont s_time_font;
static TextLayer *s_time_layer;

#define DECSEC_OFFSET 31
#define SEC_OFFSET 36
#define DECMIN_OFFSET 19
#define MIN_OFFSET 24

static void row_update(int value, int offset, char buffer[], int chars) {
 for ( ; chars > 0; chars-- ) {
    if ( value % 2 ) {
      buffer[offset+chars] = '1';
    } else {
      buffer[offset+chars] = '0';
    }
    value = value >> 1;
  }
}

static void update_time(struct tm *tick_time) {
  // Get a tm structure
  int i, tmp, offset;

  // Create a long-lived buffer
  static char buffer[] = "0000\n"
                         "  00\n"
                         "0000\n"
                         "  00\n"
                         "0000\n"
                         " 000\n"
                         "0000";

  // Minutes
  row_update(tick_time->tm_min/10, 25, buffer,3);
  row_update(tick_time->tm_min%10, 29, buffer,4);
  
  // Hours
  row_update(tick_time->tm_hour/10, 16, buffer,2);
  row_update(tick_time->tm_hour%10, 19, buffer,4);
  
  // Days
  row_update(tick_time->tm_mday/10, 6, buffer,2);
  row_update(tick_time->tm_mday%10, 9, buffer,4);

  // Months
  row_update(tick_time->tm_mon+1,-1,buffer,4);
  // Display this time on the TextLayer
  
  text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void main_window_load(Window *window) {
  // Create time TextLayer
  //s_time_layer = text_layer_create(GRect(5, 52, 139, 50));
  s_time_layer = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorClear);
  
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BINARY_24));
  
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  
  // Improve the layout to be more like a watchface
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  update_time(tick_time);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}