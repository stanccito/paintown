#ifndef _paintown_global_things_h
#define _paintown_global_things_h

#include <ostream>
#include <string>

class Bitmap;
class MessageQueue;

/* put this in an enum */
extern const int ALLIANCE_NONE;
extern const int ALLIANCE_PLAYER;
extern const int ALLIANCE_ENEMY;
extern const int ALLIANCE_FREE_FOR_ALL;

/* who uses this? it probably doesnt belong here */
extern const int MIN_RELATIVE_DISTANCE;

// extern const char * NAME_FONT;

namespace Global{
	
extern const unsigned int MagicId;

extern const char * DEFAULT_FONT;

int getVersion();
std::string getVersionString();

bool shutdown();

void setDebug( int i );
int getDebug();
std::ostream & debug(int i, const std::string & context = "paintown");

void showTitleScreen();
const std::string titleScreen();

void registerInfo(MessageQueue *);
void unregisterInfo(MessageQueue *);
void info(const std::string & str);

}

#endif
