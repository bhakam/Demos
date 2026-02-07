//
// Created by Brian Hakam on 12/4/25.
//
#pragma once

class Entity;
class InputController;

class GameStateHeader{
public:
    hashtable(int, InputController*)* playCon;
    vector<Entity*>* L;
    vector<int>* players;
    void swapPlayConValue(InputController* curr, InputController* neww){
        for (int i = 0; i < players->size(); ++i) {
            auto playerID = (*players)[i];
            if((*playCon)[playerID]==curr){
                (*playCon)[playerID]=neww;
            return;
            }
        }
    }
};

static Vec3 directionsToCoords(double yaw, double pitch){
    yaw   = yaw   deg2rad;
    pitch = pitch deg2rad;
    return {
            -std::cosf(pitch) * std::sinf(yaw),
            std::sinf(pitch),
            -std::cosf(pitch) * std::cosf(yaw)
    };
}

enum class useWhichTargetEnums : int {
    dontUse, dist, speed, accel
};

#define accvelVars(x) x(float, dist) x(float, speed) x(float, acc) x(float, maxSpeed) x(float, maxAccel) x(float, targetDist) x(float, targetSpeed) x(int, useWhichTarget)
classDefinition(accvel, accvelVars, macroVoid)
    accvel() {
        targetSpeed = -INFINITY;
        targetDist = -INFINITY;
        maxSpeed = INFINITY;
    }

    void stepPhysics(double dt) {
        myclamp(&acc, maxAccel);
        speed += acc*dt; acc=0;
        myclamp(&speed, maxSpeed);
        dist += speed*dt;
    }

    void syncTheseVars(float* targetVar, float* targetVal, double dt) {
        int k = *targetVar<*targetVal ? 1:-1;
        acc = maxAccel * k;
        stepPhysics(dt);
        int k2 = *targetVar<*targetVal ? 1:-1;
        if(k!=k2) *targetVar = *targetVal;
        *targetVal=0;
    }

    void updateaccvel(double dt) {
        switch(static_cast<useWhichTargetEnums>(useWhichTarget)) {
            case useWhichTargetEnums::dontUse:
                stepPhysics(dt); break;
            case useWhichTargetEnums::dist:
                syncTheseVars(&dist, &targetDist, dt); break;
            case useWhichTargetEnums::speed:
                syncTheseVars(&speed, &targetSpeed, dt); break;
        }

    }
endClass(accvel)

#define forxyz(X) X(x) X(y) X(z)

#define Entityvars(X) X(int, armor) X(float, x) X(float, y) X(float, z) X(float, pitch) X(float, yaw) X(float, health) X(float, hitBoxRadius) X(accvel, velocity) X(accvel, velocityX) X(accvel, velocityY) X(accvel, velocityZ) X(bool, doCollisions)
classDefinition(Entity, Entityvars, macroVoid)
    Entity(){
        hitBoxRadius = 0;
        health=1;
        doCollisions=1;
    }
    Vec3 loc(){ return {x, y, z};}
    triV render(){return mesh().rotate(yaw, pitch).move(loc());}
    virtual triV mesh(){return {};}
    virtual void onCollision(Entity* b){}

    void updateVelocity(double Δt) {
        if(distance(loc(), {0,0,0}) > 10000) health=0;

            //Update velocity from speed
#define updateVelo(veloc) veloc.updateaccvel(Δt);
        macroForAll(updateVelo, velocity, velocityX, velocityY, velocityZ)

#define helperAdder(xx) xx += c.xx;

#define forVelocity(veloc) \
        {Vec3 c = normalize(directionsToCoords(yaw, pitch));\
        c = c * veloc.speed * Δt;\
        forxyz(helperAdder)}

        macroForAll(forVelocity, velocityX, velocityZ, velocity)
//#define autoprintMacro(xx) autoPrint(&xx, #xx);
//        macroForAll(autoprintMacro, velocityX, velocityZ, velocity)

    }
endClass(Entity)

class InputController;

#define currControllableEntityvars(x) x(Entity*, a)
classDefinition(currControllableEntity, currControllableEntityvars, macroVoid)
};

#define de(W) if (ks[SDL_SCANCODE_##W]) W = true;
#define dee(W) if (ks[SDL_SCANCODE_##W]) Key##W = true;

#define inputContainerVars(x) forKeys2(x, bool) forNumberKeys2(x, bool) x(float, mouseX) x(float, mouseY) x(InputController*, ent)
classDefinition(inputContainer, inputContainerVars, macroVoid)
    void readInputs(Camera* cam){
        const Uint8* ks = SDL_GetKeyboardState(nullptr);
        if(!ks) return;
        forAlphabet(de)
        for0Through9(dee)
        if (ks[SDL_SCANCODE_ESCAPE]) running = false;
        mouseX =cam->yaw; mouseY =cam->pitch;
        SDL_Event e; int xx, yy;
        uint32_t buttons = SDL_GetMouseState(&xx, &yy);
        if(buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) leftClick=true;
        if(buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) rightClick=true;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode()) {
                float mouseSensitivity = 0.0025f; // radians per pixel
                mouseX += e.motion.xrel * mouseSensitivity;
                mouseY += -e.motion.yrel * mouseSensitivity;
                myclamp(&mouseY, 89 deg2rad);
            }
        }
    }
endClass(inputContainer)



#define aa(W) virtual void W(){ return;} virtual void W(GameStateHeader* hdr){ return;} virtual void W##hold(){ return;} virtual void W##hold(GameStateHeader* hdr){ return;}
#define bb(W) virtual void W(double a, double b){ return;}
#define dd(var) x(bool, var)
#define ee(W) if (inp->W){ W(); W(hdr); if(!prev->W) {W##hold(); W##hold(hdr);}}
#define bbb(W) virtual void W(GameStateHeader* hdr){ return;}

inputContainer defaultInputContainer={};

#define InputControllerInh(x) x(Entity)
classDefinition(InputController, macroVoid, InputControllerInh)
    static InputController* currInpCont;

    forAlphabet(aa)//keyClick
    forNumberKeys(aa)
    aa(leftClick)
    aa(rightClick)
    bb(mouseMovement)
    aa(onSelect)//When switching to this input controller, for repositioning the camera...
    aa(initCameraDir)
    virtual void updateCameraDir(Camera* cam){ return;}
    virtual Vec3 relativeEyeLoc(){}
    virtual void setCameraLoc(Camera* cam){ cam->pos=returnRelativeCameraLoc(cam)+loc();}
    virtual Vec3 returnRelativeCameraLoc(Camera* cam){return rotateYawPitch(followingDistance(), cam->yaw rad2deg, cam->pitch rad2deg);}
    virtual Vec3 followingDistance(){return {0,3,-5};}

    void doAction(double Δt, inputContainer* inp, inputContainer* prev, GameStateHeader* hdr, Camera* cam = NULL){
        if(!prev) prev = &defaultInputContainer;
        forAlphabet(ee)
        forNumberKeys(ee)
        ee(leftClick)
        ee(rightClick)
        Camera tcam{};
        tcam.yaw = inp->mouseX; tcam.pitch = inp->mouseY;
        updateCameraDir(&tcam);
        if(cam) {//cam is the cam to change
            cam->yaw = inp->mouseX;
            cam->pitch = inp->mouseY;
            setCameraLoc(cam);
        }
    }
    triV mesh() override{return {};}

endClass(InputController)

#define mMICInh(x) x(InputController)
classDefinition(mouseMovableInputController, macroVoid, mMICInh)
    void setCameraLoc(Camera* cam)override{cam->pos= {0,0,0};}
endClass(mouseMovableInputController)


vector<vector<bool>> pairwiseDistances(vector<Vec3> coords, vector<float> hitboxSizes) {
    vector<vector<bool>> r;
    for (int i = 0; i < coords.size(); ++i) {
        vector<bool> ab(coords.size());
        r.push_back(ab);
        for (int j = i+1; j < coords.size(); ++j) {
            float dist = distance(coords[i], coords[j]);
            bool collision = false;
            if(dist < hitboxSizes[i] + hitboxSizes[j]) collision = true;
            r[i][j]=collision;
        }
    }
    return r;
}

void calcAndCallCollisions(vector<Entity*> L){
    vector<Vec3> coords;
    vector<float> hitboxSizes;
    for (int i = 0; i < L.size(); ++i) {
        coords.push_back(L[i]->loc());
        int rad = L[i]->hitBoxRadius;
        if(rad==0) rad = maxVertexDeviationFromAvg(center(L[i]->render()));

        hitboxSizes.push_back(rad);
    }
    auto distL = pairwiseDistances(coords, hitboxSizes);
    for (int i = 0; i < distL.size(); ++i) {
        if(!L[i]->doCollisions) continue;
        for (int j = i+1; j < distL[i].size(); ++j) {
            if(!L[j]->doCollisions) continue;
            if(distL[i][j]) {L[i]->onCollision(L[j]); L[j]->onCollision(L[i]);}
        }
    }
}

#define Bulletvars(x) x(InputController*, owner)
#define BulletInh(x) x(Entity)
classDefinition(Bullet, Bulletvars, BulletInh)
    uint32_t color = red;
    void onCollision(Entity* b)override {
        if(b==owner) return;
        color=blue;
        b->health--;
        this->health=0;
    }

#define redefs(x) b->x = x;
    static Bullet* newBullet(double pitch, double yaw, double x, double y, double z, accvel velocity, InputController* owner){
        auto b = new Bullet();
        macroForAll(redefs, pitch, yaw, x, y, z, velocity, owner)
        return b;
    }
    static Bullet* newBullet(accvel velocity, InputController* owner){
        auto loc = owner->loc();
        return newBullet(owner->pitch, owner->yaw, loc.x, loc.y, loc.z, velocity, owner);
    }

    triV mesh() override{return buildRecPrism(color, 1, 1, 3);}
endClass(Bullet)

static triV buildTank(float turretAngle, float gunAngle, float scale = 4){
    triV tris;
    auto body= center(buildRecPrism(0x00203539, 3*scale, 2*scale, 4*scale));
    tris += body;

    auto turret= center(buildRecPrism(0x002C3539, 2*scale, 1*scale, 2*scale));
    turret.move({0, static_cast<float>(1.0*scale), 0}).rotate(turretAngle, 0);
    tris += turret;

    auto gun = center(buildRecPrism(0x0043464B, 0.5*scale, 0.5*scale, 4*scale));
    gun.move({0, static_cast<float>(1.2*scale), static_cast<float>(-2.35*scale)}).rotate(0, gunAngle).rotate(turretAngle, 0);
    tris += gun;

    return tris;
}

class Infantry;
#define RideableInputControllerinh(x) x(InputController)
#define RideableInputControllervars(x) x(Infantry*, pilot)
classDefinition(RideableInputController, RideableInputControllervars, RideableInputControllerinh)
    void Ehold(GameStateHeader* hdr) override{
        if(pilot){
            hdr->swapPlayConValue(this, reinterpret_cast<InputController*>(pilot));
            hdr->L->push_back(reinterpret_cast<Entity*>(pilot));
            pilot= nullptr;
        }
    }
endClass(RideableInputController)

#define Tankvars(x) x(float, turretAngle) x(float, gunPitch)
#define TankInh(x) x(RideableInputController)
classDefinition(Tank, Tankvars, TankInh)
    void onCollision(Entity* b) override{
    }
    triV mesh()override{
        return buildTank(turretAngle, gunPitch, 1);
    }
    void leftClick() override{

    }
    void leftClick(GameStateHeader* hdr) override{
        accvel velo;
        velo.speed=100;
        hdr->L->push_back(Bullet::newBullet(gunPitch, turretAngle, x, y, z, velo, this));
    }

    Vec3 followingDistance() override{
        return{0,12, -20};
    }

    float speed = 0.1;
    void W() override{ z+=-speed; }
    void S() override{ z+=speed; }
    void A() override{ x+=speed; }
    void D() override{ x+=-speed; }
    void K() override{ health=0;}
    void updateCameraDir(Camera* cam)override{
        turretAngle = (cam->yaw rad2deg)+180;
        gunPitch = cam->pitch rad2deg;
    }

endClass(Tank)

class Infantry;
class GameState;

#define Itemvars(x)
#define Iteminh(x) x(Entity)
classDefinition(Item, Itemvars, Iteminh)
    Item(){
        doCollisions=false;
    }
    virtual triV playerHoldingMesh(){ return {}; }
    virtual void use(GameStateHeader* hdr, Infantry* owner){}
    virtual void useSecondary(GameStateHeader* hdr, Infantry* owner){}
    virtual void utilityUse(GameStateHeader* hdr, Infantry* owner){}
endClass(Item)


#include "Vehicles.cpp"
#include "Guns.cpp"


#define playerEntityControllervars(x) x(Entity*, ent)
classDefinition(playerEntityController, playerEntityControllervars, macroVoid)
    playerEntityController(){
        ent = new Tank();
    }
endClass(playerEntityController)

#define forAllEntitiesDo(x, ...) for (int i = 0; i < L.size(); ++i) CALL_IF(*L[i], x, __VA_ARGS__);

#define cPHvars(x) x()
classDefinition(clientPrivHeader, macroVoid, macroVoid)
endClass(clientPrivHeader)

static triV buildChessFloor(uint32_t color1, uint32_t color2, int numTilesWide, int tileSize){
    triV tris;
    auto rec1 = buildRectangle(color1, tileSize, tileSize);
    auto rec2 = buildRectangle(color2, tileSize, tileSize);
    for (int i = 0; i < rec1.size(); ++i) rec1[i].rotate(0, 90).flip();
    for (int i = 0; i < rec2.size(); ++i) rec2[i].rotate(0, 90).flip();
    for (int i = -numTilesWide/2; i < numTilesWide/2; ++i) {
        for (int j = -numTilesWide/2; j < numTilesWide/2; ++j) {
            auto rec = rec1;
            if((i+j)%2) rec = rec2;
            for (int k = 0; k < rec1.size(); ++k) rec[k].move({(float)i*tileSize, -2, (float)j*tileSize});

            tris += rec;
        }
    }
    return tris;
}

#define GrassFloorinh(x) x(Entity)
classDefinition(GrassFloor, macroVoid, GrassFloorinh)
    GrassFloor(){
        doCollisions=0;
        y=-5;
    }
    triV mesh()override{
        return buildChessFloor(0x0000BF00, 0x00009900, 10, 50);
    }
endClass(GrassFloor)


int findPlayerID(hashtable(int, InputController*)* playCon, vector<int>* players, InputController* ent){
    for (int k = 0; k < players->size(); ++k) {
        int playerID = (*players)[k];
        if((*playCon)[playerID]==ent) return playerID;
    }
    return -1;
}

#define GameStatevars(x) x(vector<int>, players) x(hashtable(int, InputController*), playCon) x(hashtable(int, inputContainer), prevInputs) x(uint64_t, currentTime) x(long, currentTick) x(InputController*, currEntity) x(vector<Entity*>, L) x(objectTracker, t) x(vector<inputContainer>, inp)
classDefinition(GameState, GameStatevars, macroVoid)

    Camera cam;
    GameStateHeader hdr;

    GameState(){
        currentTime = time();
        hdr.playCon=&playCon;
        hdr.L=&L;
        hdr.players=&players;
        L.push_back(new Tank());
        auto inf = new Infantry();
        for (int i = -20; i < 20; ++i) {
            auto nI = new Infantry();
            nI->items.push_back(new Gun());
            nI->x+=i*10;
            L.push_back(nI);
        }
        L.push_back(new GrassFloor());

    }

    ~GameState() {
//        for (int i = 0; i < L.size(); ++i) delete L[i];
    }

    void tick(double Δt, vector<inputContainer>* inputs){

        currentTime+=(Δt*1000);//convert to milliseconds
        currentTick++;
        if(inputs)
            for (int i = 0; i < inputs->size(); ++i){
                Camera* mycam = NULL;
                inputContainer* prev = nullptr;
                if(i==0) {//Local player
                    mycam = &cam;
                }
                auto ent = (*inputs)[i].ent;
                if (ent) {
                    int currPlayerID=findPlayerID(&playCon, &players, ent);
                    if (currPlayerID!=-1 && prevInputs.contains(currPlayerID)) prev = &prevInputs[currPlayerID];
                    if(currPlayerID==-1) prints("Whaaat")
                    ent->doAction(Δt, &(*inputs)[i], prev, &hdr, mycam);
                    prevInputs[currPlayerID]=(*inputs)[i];

                }
            }


        calcAndCallCollisions(L);
        forAllEntitiesDo(updateVelocity, Δt)
        for (int i = L.size()-1; i >= 0; --i)
            if(L[i]->health<=0) {//todo condition that if entity is far enough away it deletes it here
                for (int j = 0; j < players.size(); ++j) {
                    int id = players[j];
                    if (playCon[id] == L[i]) respawn(id);}
//                delete L[i];
                L.erase(L.begin() + i);
            }

    }

    void addPlayer(int id) {
        players.push_back(id);
        respawn(id);
    }

    void respawn(int id){
        if(0) playCon[id] = new Tank();
        else {
            auto inf = new Infantry();
            playCon[id] = inf;
            inf->items.push_back(new Gun());

        }
        L.push_back(playCon[id]);
    }

    triV renderAll(){
        triV r;
        for (int i = 0; i < L.size(); ++i) r+=L[i]->render();
        return r;
    }

    GameState copy() {
        auto bytes = this->toBytes(&t);
        GameState r;
        int i=0;
        autoFromBytes(&r, &i, bytes, &t);
        for (int j = 0; j < L.size(); ++j) {
            auto classID = L[j]->classIntID();
            auto h = t.getObjectHeader(L[j]);
            r.t.addObject(&h, L[j]);
            Entity* a = dynamic_cast<Entity*>(constructors[classID]());
            int k = 0;
            auto bytes = L[j]->toBytes(&t);
            autoToBytes(a, &t);
            k = 0;
            a->fromBytes(bytes, &k, &t);
            r.L.push_back(a);
        }

        for (const auto& [key, value] : this->playCon) {
            r.playCon[key] = value;
        }

        return r;
    }
endClass(GameState)

