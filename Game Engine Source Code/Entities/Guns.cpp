//
// Created by Brian Hakam on 1/10/26.
//

#pragma once



double interpolate(double max, double min, double percent) { return (max - min) * percent + min; }

#define Gunvars(x) x(bool, isInADS) x(int, magCapacity) x(int, magSize) x(Infantry*, owner) x(uint64_t, lastFired) x(vector<uint64_t>, muzzles)
#define Guninh(x) x(Item)
classDefinition(Gun, Gunvars, Guninh)
    uint64_t duration=50;//in ms
    uint64_t reloadDuration=2000;
    uint64_t reloadStart=0;

    Gun(){
        magCapacity=30;
        magSize=magCapacity;
        isInADS=false;
    }

    bool isReloading(){ return time()<reloadStart+reloadDuration; }

    triV mesh()override{
        triV r;
        r+=buildRecPrism(black, 1, 2, 8.5);//Body
        r+=buildRecPrism(black, 1, 3, 1.5).move({0,-2, -1.5});//Grip
        r+=buildRecPrism(black, .7, .7, 5).move({0,1, 7});//Barrel
        r+=buildRecPrism(black, 1, 2.5, 5).move({0,0, -3});//Stock
        {
            float reloadingLeft = 1;
            if(isReloading()) {
                reloadingLeft = (float)(time()-reloadStart)/(float)(reloadDuration);
            }
            Vec3 reloadingLoc={0, static_cast<float>(interpolate(0.0, -5.0, reloadingLeft)),0};
            r += buildRecPrism(black, 1, 3, 2.5).rotate(0, 0).move({0, -2, 3}).move(reloadingLoc);//Magazine
            auto height = interpolate(1, 0,float(magSize)/magCapacity);
            if(!isReloading())
            r += buildRectangle(255, 0.5, height).rotate(-90, 0).move({-0.5,1, -0.5});
            r += buildRectangle(0xFF404040, 0.5, 1).rotate(-90, 0).move({-0.499,1, -0.5});
        }

        for (int i = muzzles.size()-1; i >= 0 ; --i) {
            auto muzzleFired = muzzles[i];
            uint64_t endTime = muzzleFired + duration;
            int64_t timeLeft = (endTime-(uint64_t)time());
            if(timeLeft<=0) {
                muzzles.erase(muzzles.begin()+i);
                continue;
            }
            auto size = interpolate(3, 1.5, 1-(double)(timeLeft)/(double)(duration));
            r += center(buildRecPrism(orange, size, size, size)).move(bulletSpawnLoc()).move({0, 0.5, 0});
        }
        return r;
    }

    void reload(){
        reloadStart=time();
        magSize=magCapacity;
    }
    void utilityUse(GameStateHeader* hdr, Infantry* owner) override{
        if(!isReloading() && magSize!=magCapacity) reload();
    }
    void use(GameStateHeader* hdr, Infantry* owner) override{
        if(isReloading()) return;
        float fireRateDelays = 1.0/10;
        uint64_t fireRateDelayms = fireRateDelays*1000;
        if(magSize<=0) {reload(); return;}
        if(time()>=lastFired+fireRateDelayms) {
            magSize--;
            lastFired = time();
            muzzles.push_back(lastFired);
            accvel velo;
            velo.speed = 500;
            Vec3 loc = owner->loc() + rotateYawPitch((meshOffsetWhenPlayerHolding() + bulletSpawnLoc() + owner->relativeEyeLoc()), owner->yaw, owner->itemPitch);
            hdr->L->push_back(Bullet::newBullet(-owner->itemPitch, owner->yaw + 180, loc.x, loc.y, loc.z, velo, owner));
        }
        if(magSize<=0) {reload(); return;}
    }
    void useSecondary(GameStateHeader* hdr, Infantry* owner)override{
        isInADS = !isInADS;
    }

    Vec3 meshOffsetWhenPlayerHolding(){
        if(isInADS && !isReloading()) return {-0.75, -2.5, 2};
        return {2, -2, 2};
    }

    Vec3 bulletSpawnLoc(){ return {0,0.75,13};}

    triV playerHoldingMesh() override{
        return mesh().move(meshOffsetWhenPlayerHolding());
    }

endClass(Gun)
