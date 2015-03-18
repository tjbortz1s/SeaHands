#include <pebble.h>
#include "game.h"
#include "player.h"
#include "island.h"
#include "ship.h"
#include "shipbattle.h"
#include "worldevent.h"

//main character's name is bill
//bill because he looks for money all the time.

static unsigned long long upcounter = 1;

void initialize_game(GameData* gamedata){
  gamedata->playership = -1;
  gamedata->gamemode = 'p';
  gamedata->playerisland = 1;
  gamedata->menulayer = 0;
  for(int i = 0; i < 5; i++)
    gamedata->currentmenu[i] = 0;
  gamedata->maxplayercargo = BASE_PLAYER_CARGO;
  gamedata->currentspeed = BASE_PLAYER_SPEED;
  gamedata->speedlevel = 0;
  gamedata->cargolevel = 0;
  initialize_islands(gamedata);
  initialize_player(gamedata);
  initialize_ships(gamedata);
}

void update_game(GameData* gamedata){
  //set up a timer as to not update everything every frame
  upcounter++;
  //what to do on each timer final
  if(upcounter%200 == 0){
    burn_player_cargo(gamedata);
    update_islands(gamedata);
  }
  if(upcounter%2000 == 0){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    int currenthour = tick_time->tm_hour;
    int currentday = tick_time->tm_yday;
    if(currenthour != gamedata->eventhour || currentday != gamedata->eventday){
      selectrandomevent(gamedata);
      gamedata->eventhour = currenthour;
      gamedata->eventday = currentday;
    }
    //always do this on the largest upcounter
    upcounter = 1;
  }
  updateevent(gamedata);
  update_ships(gamedata);
  update_player(gamedata); 
}


int random(int max){
if(max == 0)
  return 0;
time_t temp = time(NULL);
struct tm *tick_time = localtime(&temp);
int minuite = 0;
int second = 0;
second = tick_time->tm_sec;
minuite = tick_time->tm_min;
  
static long seed = 100;
seed += minuite + second;
seed = (((seed * 214013L + 2531011L) >> 16) & 32767);

return ((seed % (max+1)));
}


int finddistance(int x1, int y1, int x2, int y2){
  int xminus;
  int yminus;
  xminus = x2 - x1;
  yminus = y2 - y1;
  long long int xsquared = xminus * xminus;
  long long int ysquared = yminus * yminus;
  return xsquared + ysquared;
}

//takes the gamedata, and figures out how much of each resource exists
//returns the total value of each resource based on this, based on how much of each mineral
//exists in relation to the others
// 0 - 100 results in a high price for the 0
// 0 - 0 results in average price for both
//currentisland can either be a number 1 - max, or will default to "average" values with -1
//returns values for all 4 inside of int8_t data
ResourceValues getmoneyvalue(GameData* gamedata, int currentisland){
  //APP_LOG(APP_LOG_LEVEL_INFO, "CURRENTISLAND IS: %i", currentisland);
  int totalmetal = 0, totalwood = 0, totalstone = 0, totalfood = 0, average = 0;
  for(int i = 0; i < TOTALISLANDS; i++){
    totalmetal += gamedata->islandscargo[i][0];
    totalwood += gamedata->islandscargo[i][1];
    totalstone  += gamedata->islandscargo[i][2];
    totalfood += gamedata->islandscargo[i][3];
  }
  average = totalmetal + totalwood + totalstone + totalfood;
  average = average / 4;
  
  ResourceValues returnvalue;
  
  //find initial values based on currency
  if(totalmetal > 0)
    returnvalue.metalvalue = ((10 * average)/totalmetal);
  else
    returnvalue.metalvalue = 1;
  if(totalstone > 0)
    returnvalue.stonevalue = ((10 * average)/totalstone);
  else
    returnvalue.stonevalue = 1;
  if(totalwood > 0)
    returnvalue.woodvalue = ((10 * average)/totalwood);
  else
    returnvalue.woodvalue = 1;
  if(totalfood > 0)
    returnvalue.foodvalue = ((10 * average)/totalfood);
  else
    returnvalue.foodvalue = 1;
  
  if(returnvalue.metalvalue > 200)
    returnvalue.metalvalue = 200;
  if(returnvalue.woodvalue > 200)
    returnvalue.woodvalue = 200;
  if(returnvalue.stonevalue > 200)
    returnvalue.stonevalue = 200;
  if(returnvalue.foodvalue > 200)
    returnvalue.foodvalue = 200;
  //adjust values based on island cargo levels if an island was selected
  if(currentisland != -1){
    returnvalue.metalvalue = returnvalue.metalvalue + (returnvalue.metalvalue* (50-gamedata->islandscargo[currentisland][0])/100);
    returnvalue.woodvalue = returnvalue.woodvalue + (returnvalue.woodvalue* (50-gamedata->islandscargo[currentisland][1])/100);
    returnvalue.stonevalue = returnvalue.stonevalue + (returnvalue.stonevalue* (50-gamedata->islandscargo[currentisland][2])/100);
    returnvalue.foodvalue =  returnvalue.foodvalue + (returnvalue.foodvalue* (50-gamedata->islandscargo[currentisland][3])/100);
  }
  
  
  //ten credits is on average worth 1 resource
  //islands value resources they have less than average of
  //islands do not value resources they have more than average of
  //a resource that has more than the average number of units, is less valuable
  //a resources that has less than the average number of units is more valuable
  
  return returnvalue;
}


void save_data(GameData* gamedata){
  //Object object;
  //object.field = 2;
  //persist_write_data(OBJECT_KEY, &object, sizeof(object));
  
  //set up the save values object with corresponding values from gamedata
  SaveValues saving;
  for(int i = 0; i < 10; i++){
    saving.shipsx[i] = gamedata->shipsx[i];
    saving.shipsy[i] = gamedata->shipsy[i];
    
    
    
    //these are hardcoded... for now.
    //islandsx[i] = gamedata->islandsx[i];
    //islandsy[i] =  = gamedata->islandsy[i];
    //islandstypes[i];
    for(int j = 0; j < 4; j++)
      saving.islandscargo[i][j] = gamedata->islandscargo[i][j];
    saving.shipsisland[i] = gamedata->shipsisland[i];
    saving.shipstype[i] = gamedata->shipstype[i];
  }
  for(int i = 0; i < 5; i++)
    saving.playercargo[i] = gamedata->playercargo[i];
  saving.playerwallet = gamedata->playerwallet;
  saving.maxplayercargo = gamedata->maxplayercargo;
  saving.currentspeed = gamedata->currentspeed;
  saving.cargolevel = gamedata->cargolevel;
  saving.speedlevel = gamedata->speedlevel;
  saving.metalmultiplier = gamedata->metalmultiplier;
  saving.stonemultiplier = gamedata->stonemultiplier;
  saving.woodmultiplier = gamedata->woodmultiplier;
  saving.foodmultiplier = gamedata->foodmultiplier;
  saving.metaldivisor = gamedata->metaldivisor;
  saving.stonedivisor = gamedata->stonedivisor;
  saving.wooddivisor = gamedata->wooddivisor;
  saving.fooddivisor = gamedata->fooddivisor;
  saving.currentevent = gamedata->currentevent;
  saving.eventhour = gamedata->eventhour;
  saving.eventday = gamedata->eventday;
  
  //save the object into memory
  persist_write_data(DATA_KEY, &saving, sizeof(saving));
}

void load_data(GameData* gamedata){
  SaveValues loading;
  //Object read_object;
  //persist_read_data(OBJECT_KEY, &read_object, sizeof(read_object));
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "read_object.field = %d", read_object.field);
  if(persist_exists(DATA_KEY)){
    persist_read_data(DATA_KEY, &loading, sizeof(loading));
    //load the values into gamedata
    for(int i = 0; i < 10; i++){
      gamedata->shipsx[i] = loading.shipsx[i];
      gamedata->shipsy[i] = loading.shipsy[i];
      //these are hardcoded... for now.
      //islandsx[i] = gamedata->islandsx[i];
      //islandsy[i] =  = gamedata->islandsy[i];
      //islandstypes[i];
      for(int j = 0; j < 4; j++)
        gamedata->islandscargo[i][j] = loading.islandscargo[i][j];
      gamedata->shipsisland[i] = loading.shipsisland[i];
      gamedata->shipstype[i] = loading.shipstype[i];
    }
    for(int i = 0; i < 5; i++)
      gamedata->playercargo[i] = loading.playercargo[i];
    gamedata->playerwallet = loading.playerwallet;
    gamedata->maxplayercargo = loading.maxplayercargo;
    gamedata->currentspeed = loading.currentspeed;
    gamedata->playeryvelocity = loading.currentspeed;
    gamedata->cargolevel = loading.cargolevel;
    gamedata->speedlevel = loading.speedlevel;
    gamedata->metalmultiplier = loading.metalmultiplier;
    gamedata->stonemultiplier = loading.stonemultiplier;
    gamedata->woodmultiplier = loading.woodmultiplier;
    gamedata->foodmultiplier = loading.foodmultiplier;
    gamedata->metaldivisor = loading.metaldivisor;
    gamedata->stonedivisor = loading.stonedivisor;
    gamedata->wooddivisor = loading.wooddivisor;
    gamedata->fooddivisor = loading.fooddivisor;
    gamedata->currentevent = loading.currentevent;
    gamedata->eventhour = loading.eventhour;
    gamedata->eventday = loading.eventday;
  }
  else{
    //if no previous data values, set default values
    gamedata->playerwallet = 500;
  }
}

int check_for_player_upgrade(GameData* gamedata, int upgradetype){
  return 0;
}

int check_player_upgrade_price(GameData* gamedata, int upgradetype){
  if(upgradetype == 0){
    return (gamedata->cargolevel + 1)*100;
  }
  if(upgradetype == 1){
    return (gamedata->speedlevel + 1)*500;
  }
  return 0;
}

//returns 1 for up 2 for down, and 3 for select
int check_current_button(GameData* gamedata){
  if(gamedata->uphit == 1 && gamedata->downhit == 0)
    return 1;
  if(gamedata->uphit == 0 && gamedata->downhit == 1)
    return 2;
  if(gamedata->uphit == 1 && gamedata->downhit == 1)
    return 3;
  return 0;
}

//will setup the display of notifications, including the changing gamemode, loading the data in gamedata
//it should eventually manage data for variables in notifications
void displaynotification(GameData* gamedata, char notificationtext[100]){
  //here is where I would process and insert variable values into the string
  //however, that is a bit fancy and tough to do, and I don't need it right now
  //so do that later, future me.
  gamedata->gamemode = 'm';
  gamedata->menulayer = 4;
  for(int i = 0; i < 100; i++){
    gamedata->notificationtext[i] = notificationtext[i];
  }
}

//will show a notification for the tutorial
//if forcetutorial is one, it will always show the tutorial.
void attempt_tutorial(GameData* gamedata, int8_t forcetutorial){
  if(!persist_exists(DATA_KEY) || forcetutorial == 1){
    displaynotification(gamedata, "WELCOME TO SEA HANDS!\nTutorial:\nMt = Metal\nWd = Wood\nSt = Stone\nFd = Food\nSu = Supplies");
  }
}

void exitmenus(GameData* gamedata){
  if(gamedata->gamemode == 'm'){
    if((gamedata->menulayer == 0 || gamedata->menulayer == 1 || gamedata->menulayer == 2)){
      exitisland(gamedata);
      gamedata->currentmenu[0] = 0;
      gamedata->currentmenu[1] = 0;
      gamedata->currentmenu[2] = 0;
    }
    if(gamedata->menulayer == 3)
      gamedata->playership = -1;
    gamedata->currentmenu[gamedata->menulayer] = 0;
    gamedata->menulayer = 0;
    gamedata->gamemode = 'p';
  }
}