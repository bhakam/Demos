//
// Created by Brian Hakam on 1/7/26.
//
#pragma once

class Gun;

#define Infantryvars(x) x(vector<Item*>, items) x(int, currItemInd) x(float, itemPitch)
#define Infantryinh(x) x(InputController)
classDefinition(Infantry, Infantryvars, Infantryinh)
    Infantry(){
        currItemInd = 0;
#define setUpVelocForInfantry(veloc)\
    veloc.useWhichTarget= static_cast<int>(useWhichTargetEnums::speed);\
    veloc.maxAccel=100;
        macroForAll(setUpVelocForInfantry, velocityX, velocityZ, velocity)
    }
    Item* currItem(){
        if(currItemInd < items.size() && currItemInd>=0) return items[currItemInd];
        return nullptr;
    }
    void leftClick(GameStateHeader* hdr)override{
        if(currItem()) currItem()->use(hdr, this);
    }

    void addItem(Item* item){
        items.push_back(item);
    }

    void Ehold(GameStateHeader* hdr) override{
        auto L = hdr->L;
        float closestCollision=INFINITY;
        RideableInputController* closestObj = NULL;
        int buffer=20;
        for (int i = 0; i < hdr->L->size(); ++i) {
            auto obj = dynamic_cast<RideableInputController*>((*L)[i]);
            if(!obj) continue;
            float dist = distance(loc(), obj->loc());
            bool collision = false;
            if(dist < obj->hitBoxRadius + hitBoxRadius+buffer) collision = true;
            if(collision && dist<closestCollision){closestCollision=dist; closestObj=obj;}
        }
        if(closestObj && !closestObj->pilot){
            closestObj->pilot=this;
            hdr->swapPlayConValue(this, closestObj);
            hdr->L->erase(
                    std::remove(hdr->L->begin(), hdr->L->end(), this),
                    hdr->L->end()
            );

        }
    }

    void Key1() override{
        currItemInd--;
        if(currItemInd<0) currItemInd=items.size()-1;
    }
    void Key2() override{
        currItemInd++;
        if(currItemInd>=items.size()) currItemInd=0;
    }

//void removeItem(Item* item){
//    items.find
//}


    float speed = 0.5;
    void W() override{ z+=speed*cos(yaw deg2rad) ; x+=speed*sin(yaw deg2rad);}
    void S() override{ z+=speed*cos(PI+yaw deg2rad); x+=speed*sin(PI+yaw deg2rad);}
    void A() override{ z+=speed*cos(-PI/2+yaw deg2rad); x+=speed*sin(-PI/2+yaw deg2rad);}
    void D() override{ z+=speed*cos(PI/2+yaw deg2rad); x+=speed*sin(PI/2+yaw deg2rad);}
    void updateCameraDir(Camera* cam)override{
        yaw = (cam->yaw rad2deg);//+180;
        itemPitch = -(cam->pitch rad2deg);
    }
    triV mesh() override{
        auto r = buildRecPrism(blue, 3, 10, 3);
        if(currItem()) r+=currItem()->playerHoldingMesh().rotate(0, itemPitch).move(relativeEyeLoc());//Move it from being zeroed in object center to camera center
        return r;
    }

#define itemKeyMapping(keyName, itemCommand) void keyName(GameStateHeader* hdr)override{currItem()->itemCommand(hdr, this);}
    itemKeyMapping(Rhold, utilityUse)
    itemKeyMapping(rightClickhold, useSecondary)

    Vec3 followingDistance() override{return {0,5,-5};}
    Vec3 relativeEyeLoc() override {return {0,4,2};}//Relative from entity center
    void setCameraLoc(Camera* cam)override{//First Person View
        Vec3 eyeLoc = relativeEyeLoc();//Middle of face, eye height, eye sticking out, remember this is from the center not bottom
        eyeLoc = rotateYawPitch(eyeLoc, cam->yaw rad2deg, 0);
        eyeLoc+=loc();
        cam->pos=eyeLoc;
    }
endClass(Infantry)

/*
#define Tankvars(x) x(float, turretAngle) x(float, gunPitch)
#define TankInh(x) x(InputController)
classDefinition(Tank, Tankvars, TankInh)
    void onCollision(Entity* b) {
    }
    triV mesh()override{
        return buildTank(turretAngle, gunPitch);
    }
    Vec3 returnRelativeCameraLoc(Camera* cam) override{
        return rotateYawPitch(setFollowingDistance(), cam->yaw rad2deg, cam->pitch rad2deg);
    }
    void leftClick(vector<Entity*>* L){
        accvel velo;
        velo.speed=100;
        L->push_back(Bullet::newBullet(gunPitch, turretAngle, x, y, z, velo, this));
    }

    float speed = 0.1;
    void W() override{ z+=-speed; }
    void S() override{ z+=speed; }
    void A() override{ x+=speed; }
    void D() override{ x+=-speed; }
    void K() override{ health=0;}
    void updateCameraDir(Camera* cam){
        turretAngle = (cam->yaw rad2deg)+180;
        gunPitch = cam->pitch rad2deg;
    }

endClass(Tank)
*/