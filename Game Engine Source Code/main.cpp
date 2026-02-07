#include "main.h"
#pragma once

double sleepTillTick(uint64_t* lastTime){
    printVar(timeOffset)
    auto lastTimePassedms = time() - *lastTime;
    auto sleepFor = ((tickRate*1000) - lastTimePassedms) /1000;
    if (sleepFor>0) {slp(sleepFor);}// prints("Slept for: ") prints(sleepFor)}
    else {prints("Behind(s) ") printVar(sleepFor)}
    *lastTime = time();
    return sleepFor;
}

int main(){
    Renderer r = Renderer(1920/2, 1080/2);
    networkAddress add;
    add.address = "0.0.0.0";
    add.port = 7080;

    int runServer;
    cout << "runServer?";
    if(1) cin >> runServer;
    else runServer=2;

    auto lastTime = time();
    if(runServer==4){
        GameState g;
        g.addPlayer(1);
        while(running){
            sleepTillTick(&lastTime);
            inputContainer inp;
            inp.readInputs(&g.cam);
            inp.ent=g.playCon[1];
            vector<inputContainer> vc; vc.push_back(inp);
            g.tick(tickRate, &vc);
            r.render(g.renderAll(), 1, g.cam);
        }
    }
    if(runServer==2){
        add.address = "0.0.0.0";
        GameServer s;
        GameClient c; c.n.connectTo(s.n.getAddress());
        while(running){
            sleepTillTick(&lastTime);
            s.tick(tickRate);
            c.tick(tickRate);
            r.render(c.g->renderAll(), 1, c.g->cam);
        }
    }
//    else if(runServer==3){
//        GameClient c;
//        while(running){
//            sleepTillTick(&lastTime);
//            vector<inputContainer> inputs;
//            inputs.push_back(pastPlayerInputs.a[inputInd++]);
//            c.g->tick(Δt, &inputs);
//            r.render(c.g->renderAll(), 1, c.g->cam);
//        }
//    }
    else if(runServer==1){
        GameServer s;
        prints(s.n.getAddress().address)
        prints(s.n.getAddress().port)
        while(running){
            sleepTillTick(&lastTime);
            s.tick(tickRate);
        }
    }
    else if(runServer==0){
        GameClient c; c.n.connectTo(add);
        while(running){
            sleepTillTick(&lastTime);
            c.tick(tickRate);
            r.render(c.g->renderAll(), 1, c.g->cam);
        }
    }

}
