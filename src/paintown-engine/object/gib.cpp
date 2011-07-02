#include "util/bitmap.h"
#include "util/trans-bitmap.h"
#include <iostream>
#include "object_nonattack.h"
#include "gib.h"
#include "util/funcs.h"
#include "globals.h"
#include <math.h>

namespace Paintown{

Gib::Gib(const int x, const int y, const int z, double dx, double dy, double dz, Graphics::Bitmap * image):
ObjectNonAttack(x, z),
dx(dx),
dy(dy),
dz(dz),
angle(0),
fade(0),
image(image){
    setY(y);
    setMaxHealth(1);
    setHealth(1);

    Graphics::RestoreState graphicsState;
    bloodImage = new Graphics::Bitmap(2, 2);
    bloodImage->clearToMask();
    bloodImage->circleFill(bloodImage->getWidth() / 2, bloodImage->getHeight() / 2, 1, Graphics::makeColor(255, 0, 0));
}

Gib::Gib(const Gib & g):
ObjectNonAttack(g),
image(g.image),
bloodImage(g.bloodImage){
}

void Gib::draw(Graphics::Bitmap * work, int rel_x, int rel_y){
    if (fade > 0){
        // Bitmap::dissolveBlender( 0, 0, 0, 255 - fade );
        Graphics::Bitmap::transBlender(0, 0, 0, 255 - fade);
        image->translucent().draw(getRX() - rel_x - image->getWidth() / 2, getRY() - image->getHeight() / 2, *work);
    } else {
        Graphics::Bitmap::transBlender(0, 0, 0, 200);
        work->startDrawing();
        for (std::vector< Point >::iterator it = blood.begin(); it != blood.end(); it++){
            const Point & p = *it;

            bloodImage->translucent().draw(p.x - rel_x, p.y, *work);
            // bloodImage->draw(p.x - rel_x, p.y, *work);
            /*
            int l = 200 + p.life * 15;
            Graphics::Color red = Graphics::makeColor(l > 255 ? 255 : l, 0, 0);
            work->translucent().circleFill(p.x - rel_x, p.y, 1, red);
            */
            // work->putPixel( p.x - rel_x, p.y, red );
        }
        work->endDrawing();
        // image->draw( getRX() - rel_x - image->getWidth() / 2, getRY() - image->getHeight() / 2, *work );
        image->drawRotate(getRX() - rel_x - image->getWidth() / 2, getRY() - image->getHeight() / 2, angle, *work);
    }
}
	
Object * Gib::copy(){
    return new Gib( *this );
}

bool Gib::isCollidable( Object * obj ){
    return false;
}

bool Gib::isGettable(){
    return false;
}

int Gib::getWidth() const {
    return 1;
}

int Gib::getHeight() const {
    return 1;
}
	
Network::Message Gib::getCreateMessage(){
    Network::Message m;

    return m;
}
	
void Gib::act( std::vector< Object * > * others, World * world, std::vector< Object * > * add ){
    if ( fade > 0 ){
        fade += 2;
        if ( fade > 255 ){
            setHealth( -1 );
        }
    } else {

        moveX(dx);
        moveY(dy);
        moveZ(dz);

        dy -= 0.1;
        if ( getY() <= 0 ){
            dy = -dy / 2;
            dx = dx / 2;
            dz = dz / 2;
            if ( fade == 0 && fabs( dy ) < 0.1 ){
                fade = 1;
            }

            /*
               if ( fabs( dy ) < 0.1 ){
               setHealth( -1 );
               }
               */
        }

        for ( int i = 0; i < Util::rnd(3) + 2; i++ ){
            int x = getRX() + Util::rnd(5) - 2;
            int y = getRY() + Util::rnd(5) - 2;
            blood.push_back(Point(x, y, Util::rnd(10) + 5));
        }

        for ( std::vector< Point >::iterator it = blood.begin(); it != blood.end(); ){
            Point & p = *it;
            p.life -= 1;
            if ( p.life <= 0 ){
                it = blood.erase( it );
            } else {
                it++;
            }
        }

        angle += (int) sqrt( dx * dx + dy * dy ) * 3;
    }
}

Gib::~Gib(){
}

}
