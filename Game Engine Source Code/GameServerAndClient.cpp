//
// Created by Brian Hakam on 12/25/25.
//


//Go through each player inputs, if they have 2 then consolidate them, if none then predict(null)
//Player wrap on input controller(linking player to the controller)

#pragma once


class GameServer{
public:
    hashtable(int, int) existingPlayers;

    networkServer n;
    GameState g;
    void tick(double Δt) {
        //Add players
        for (int j = 0; j < n.clients->size(); ++j) {
            int id = n.clients->operator[](j);
            if(!existingPlayers.contains(id)) {
                existingPlayers[id]=1;
                g.addPlayer(id);
            }
        }

        //Get inputs
        vector <inputContainer> inputs;
        for (int i = 0; i < n.clients->size(); ++i) {
            int id = n.clients->operator[](i);
            auto rec = n.receiveObjectFrom(id);

            if(rec.size()) {
                auto newinp = dynamic_cast<inputContainer*>(rec.back());
                newinp->ent = g.playCon[id];
                inputs.push_back(*newinp);
            } else prints("Didn't get inputs")

        }

        // Apply inputs and tick gamestate
        g.tick(Δt, &inputs);
        g.currentTime = now_ms();
        // Send gamestate
        for (int i = 0; i < n.clients->size(); ++i){
            int id = n.clients->operator[](i);
            g.currEntity = g.playCon[id];
            n.sendObjectTo(id, &g);
        }

    }

};

template <typename A, typename B>
class doubleList{
public:
    vector<A> a;
    vector<B> b;
    void add(A aa, B bb){
        a.push_back(aa);
        b.push_back(bb);
    }
    void remove(int i){
        a.erase(a.begin()+i);
        b.erase(b.begin()+i);
    }
    int size() { return a.size(); }
};

class GameClient{
public:
    networkClient n;
    GameState* g = new GameState();
    doubleList<inputContainer, uint64_t> pastPlayerInputs;
    void tick(double Δt) {
        GameState* newg = nullptr;
        auto received = n.receiveObjectFrom();
        if(received.size()) newg = dynamic_cast<GameState*>(received.back());
        auto currentTick = g->currentTick;
        if(newg) {
//            delete g;//Causes crashes because I reuse those sub objects, probably need to update objectTracker to reassign objIDs
            g=newg;
            int inputInd = 0;
            int lagms = simLagS*1000;//TODO for networker add lag func and time sync
            while(pastPlayerInputs.size() && pastPlayerInputs.b[0] < g->currentTime-lagms) pastPlayerInputs.remove(0);
            while (g->currentTime-lagms < now_ms() - Δt*1000 && pastPlayerInputs.size() > inputInd) {//Tick to now with my past inputs and interpolated other inputs
                //TODO add input interpolation, currently it just moves me and leaves everyone else with no inputs

                vector<inputContainer> inputs;
                inputs.push_back(pastPlayerInputs.a[inputInd++]);
                g->tick(Δt, &inputs);
            }

        }
        //Read server to update a gamestate then copy it and simulate it with my inputs and predicted inputs of everyone else
        auto inp = inputContainer();
        inp.readInputs(&(g->cam));
        inp.ent = dynamic_cast<InputController*>(g->currEntity);
        pastPlayerInputs.add(inp, now_ms());
        n.sendObjectToServer(&inp);
        vector<inputContainer> inputs;
        inputs.push_back(pastPlayerInputs.a.back());
        g->tick(Δt, &inputs);
    }
};

#define test0vars(x) x(int, a)
classDefinition(test0, test0vars, macroVoid)
endClass(test0)

#define test1vars(x) x(test0*, p)
classDefinition(test1, test1vars, macroVoid)
endClass(test1)

//todo Need cascading foreach function so I can mass delete objects given a single root,
//This pattern is very useful for other things as well



