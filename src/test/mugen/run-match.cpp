#include <string>
#include "util/init.h"
#include "util/debug.h"
#include "util/timedifference.h"
#include "mugen/character.h"
#include "mugen/config.h"
#include "mugen/behavior.h"
#include "mugen/stage.h"
#include "mugen/parse-cache.h"
#include "util/file-system.h"

using namespace std;

void run(string path1 = "mugen/chars/kfm/kfm.def", string path2 = "mugen/chars/kfm/kfm.def"){
    Mugen::ParseCache cache;
    string stagePath = "mugen/stages/kfm.def";
    Mugen::Character kfm1(Filesystem::find(Filesystem::RelativePath(path1)));
    Mugen::Character kfm2(Filesystem::find(Filesystem::RelativePath(path2)));
    kfm1.load();
    kfm2.load();
    Mugen::LearningAIBehavior player1AIBehavior(Mugen::Data::getInstance().getDifficulty());
    Mugen::LearningAIBehavior player2AIBehavior(Mugen::Data::getInstance().getDifficulty());
    kfm1.setBehavior(&player1AIBehavior);
    kfm2.setBehavior(&player2AIBehavior);
    Mugen::Stage stage(Filesystem::find(Filesystem::RelativePath(stagePath)));
    stage.addPlayer1(&kfm1);
    stage.addPlayer2(&kfm2);
    stage.load();
    stage.reset();

    TimeDifference diff;
    diff.startTime();
    while (!stage.isMatchOver()){
        stage.logic();
    }
    diff.endTime();
    Global::debug(0, "test") << diff.printTime("Success! Took") << endl;
}

int main(int argc, char ** argv){
    InputManager manager;
    Global::initNoGraphics();
    if (argc == 1){
        run();
    } else if (argc == 2){
        run(argv[1]);
    } else if (argc > 2){
        run(argv[1], argv[2]);
    }
}