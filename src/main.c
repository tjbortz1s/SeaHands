#include <pebble.h>
#include "game.h"
#include "types.h"
  
//define a number of constants
const uint8_t PEBBLEHEIGHT = 168;
const uint8_t PEBBLEWIDTH = 144;
const uint8_t NUMOFSHIPS = 0;
const uint8_t NUMOFISLANDS = 1;
//used for island AI to sell off materials at certain prices
const uint8_t TARGETVALUEMETAL = 100;
const uint8_t TARGETVALUEWOOD = 100;
const uint8_t TARGETVALUESTONE = 100;
const uint8_t TARGETVALUEFOOD = 100;
//values for the player stats
const uint8_t MINSHIPSPEED = 1;
const uint8_t MAXSHIPSPEED = 10;
const uint8_t SHIPMAXCARGO = 10; //per resource, not as a whole
const uint8_t NUMBEROFMENUITEMS = 5;

//create the window and canvas layer
static Window *window;
static Layer *graphics_canvas_layer;

GameData gamedata;

static void canvas_update_proc(Layer *this_layer, GContext *ctx){
  //GRect bounds = layer_get_bounds(this_layer);
  //get the bounds of the layer
  //get center of screen
  //GPoint center = GPoint((bounds.size.w / 2), (bounds.size.h / 2));
  
  //draw the player
  GPoint player = GPoint(72, 82);
  graphics_context_set_fill_color(ctx, GColorBlack);
  //draw the player
  graphics_fill_circle(ctx, player, 5);
  
  //draw the direction, if it is not zero
  if((gamedata.playerxvelocity != 0) || (gamedata.playeryvelocity !=0)){
    if( (abs(gamedata.playerxvelocity) <= 5) && (abs(gamedata.playeryvelocity) <= 5 ))
      graphics_context_set_fill_color(ctx, GColorWhite);
    //72 and 82 are the center of pebble's screen.
    GPoint playervector = GPoint((72 + gamedata.playerxvelocity), (82 + gamedata.playeryvelocity));
    graphics_fill_circle(ctx, playervector, 2);
  }
  
  //draw any islands that are currently on screen.
  for(int i = 0; i < 10; i++){
    if(gamedata.islandsx[i] == -1 && gamedata.islandsy[i] == -1){
      //do nothing, island does not exist
    }
    else{ //if(abs(gamedata.islandsx[i]-gamedata.playerx) < 146 && abs(gamedata.islandsy[i] - gamedata.playery) < 165){
      graphics_context_set_fill_color(ctx, GColorBlack);
      GPoint islandpoint = GPoint(gamedata.islandsx[i] - gamedata.playerx + 72, gamedata.islandsy[i] - gamedata.playery + 84);
      graphics_fill_circle(ctx, islandpoint, 25);
    }
  }
  
  //draw any ships that exist, and are on screen
  if(gamedata.totalships >= 0){
    for(int i = 0; i <= gamedata.totalships; i++){
      if(abs(gamedata.shipsx[i]-gamedata.playerx) < 146 && abs(gamedata.shipsy[i] - gamedata.playery) < 165)
      graphics_context_set_fill_color(ctx, GColorBlack);
      GPoint shippoint = GPoint(gamedata.shipsx[i] - gamedata.playerx + 72, gamedata.shipsy[i] - gamedata.playery + 84);
      graphics_fill_circle(ctx, shippoint, 3);
    }
  }
  
  if(gamedata.gamemode == 'm'){ //draw menus from the info in gamedata
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorBlack);
    //will extract whatever number is selected.  Then draw a box at some X Y coords depending on that, the box shows what menu icon is selected
    //menu is otherwise drawn statically, with no difference between boxes.
    //may have to run a function to display the menu, will draw the black box as part of the canvas layer.
    
    graphics_fill_rect(ctx, GRect(0,0,54,76), 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_rect(ctx, GRect(0, gamedata.currentmenu*15, 54, 15 ));
    GRect textbox = GRect(0, 0, 54, 73);
    char totalmenu[9 * NUMBEROFMENUITEMS];
    //create the char array for the menu    
    snprintf(totalmenu, sizeof(totalmenu), "Metal:%d\nWood:%d\nStone:%d\nFood:%d\n-Exit-\n",
           gamedata.islandscargo[gamedata.playerisland][0],
           gamedata.islandscargo[gamedata.playerisland][1],
           gamedata.islandscargo[gamedata.playerisland][2],
           gamedata.islandscargo[gamedata.playerisland][3]);
    graphics_draw_text(ctx, totalmenu, fonts_get_system_font(FONT_KEY_FONT_FALLBACK), textbox, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  }
  
  //draw loop
  //graphics_context_set_fill_color(ctx, GColorBlack);
  //graphics_fill_circle(ctx, center, 40);
  //graphics_context_set_fill_color(ctx, GColorWhite);
  //graphics_fill_circle(ctx, center, 35);
  //draw the "stalk"
  //graphics_context_set_fill_color(ctx, GColorBlack);
  //graphics_fill_rect(ctx, GRect(32, 50, 5, 100), 0, GCornerNone);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  gamedata.downhit = 1;
  gamedata.uphit = 1;
  
  //text_layer_set_text(text_layer, "Select");
}

static void select_release_handler(ClickRecognizerRef recognizer, void *context) {
  gamedata.downhit = 0;
  gamedata.uphit = 0;
  
  //text_layer_set_text(text_layer, "XSelect");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  gamedata.uphit = 1;
  //gamedata.buttonrelease = 0;
  //text_layer_set_text(text_layer, "Up");
}

static void up_release_handler(ClickRecognizerRef recognizer, void *context) {
  gamedata.uphit = 0;
  gamedata.buttonrelease = 1;
  //text_layer_set_text(text_layer, "XUp");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  gamedata.downhit = 1;
  //gamedata.buttonrelease = 0;
  //text_layer_set_text(text_layer, "Down");
}

static void down_release_handler(ClickRecognizerRef recognizer, void *context) {
  gamedata.downhit = 0;
  gamedata.buttonrelease = 1;
  //text_layer_set_text(text_layer, "XDown");
}

static void click_config_provider(void *context) {
  window_raw_click_subscribe(BUTTON_ID_UP, up_click_handler, up_release_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_DOWN, down_click_handler, down_release_handler, NULL);
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_click_handler, select_release_handler, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //create and set the update protocol for the canvas to draw on
  graphics_canvas_layer = layer_create(GRect(0,0,bounds.size.w, bounds.size.h));
  layer_add_child(window_layer, graphics_canvas_layer);
  layer_set_update_proc(graphics_canvas_layer, canvas_update_proc);

  //remove tutorial things later, when I understand them
  //text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  //text_layer_set_text(text_layer, "Press a button");
  //text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  //layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  //text_layer_destroy(text_layer);
  layer_destroy(graphics_canvas_layer);
}

void handleTimer(){
  //tell the watch to run this again and again and again, X amount of time apart
  app_timer_register(GAMELOOP_TIMER_INTERVALL, handleTimer, NULL);
  //function that is called to update the values in gamedata
  update_game(&gamedata);
  //make sure the layer gets redrawn
  layer_mark_dirty(graphics_canvas_layer);
}

static void init(void) {
  //do required window things
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	.load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
  
  //begin initializing game elements
  gamedata.gamemode = 'm';
  gamedata.playerisland = 1;
  initialize_islands(&gamedata);
  initialize_player(&gamedata);
  initialize_ships(&gamedata);
  

  //start the game loop
  app_timer_register(GAMELOOP_TIMER_INTERVALL, handleTimer, NULL);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}