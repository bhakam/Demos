//
// Created by Brian Hakam on 11/8/25.
//

void mySend(int socketFD, const void * data, size_t size) {
    const char* p = static_cast<const char*>(data);
    size_t sentTotal = 0;

    while (sentTotal < size) {
        ssize_t n = send(socketFD, p + sentTotal, size - sentTotal, 0);
        sentTotal += static_cast<size_t>(n);
    }
}

#define packetHeadervars(x) x(int, length)
classDefinition(packetHeader, packetHeadervars, macroVoid)
endClass(packetHeader)

void receiveMasterFunction(mutex* m, int socketFD, vector<vector<byte>*>** data, AutoLock<vector<int>>* clients= nullptr) {
    thread([socketFD, m, data, clients] {
        mutex a;
        while (1) {
            vector<byte> *r = new vector<byte>();
            int eosIndex = 0;
            int eosLength = -1;
            vector<byte> sizeBuf;
            packetHeader h;
            objectTracker tgt{};
            int packetHeaderSize = h.toBytes(&tgt).size();//Since sizeof gives in memory size and not toBytes size

            while (1) {
                char buf[1024];
                int n = recv(socketFD, buf, sizeof(buf), 0);//todo add ondisconnect in game server and trigger that here aswell(remove from clients list)
                if(n==0) {
                    if(clients){
                        for (int i = 0; i < (*clients)->size(); ++i) {
                            if ((*clients)[i] == socketFD) {
                                (*clients)->erase((*clients)->begin() + i);
                                return;
                            }
                        }
                    }
                    return;
                }
                for (int i = 0; i < n; ++i) {

                    if (eosIndex < packetHeaderSize) {//Capturing length of message
                        sizeBuf.push_back(static_cast<byte>(buf[i]));
                        if (eosIndex == packetHeaderSize - 1) {//Captured full header now writing eosLength
                            int g = 0;
                            objectTracker t{};
                            h.fromBytes(sizeBuf, &g, &t);
                            eosLength = h.length + packetHeaderSize;
                            sizeBuf.clear();
                        }
                    } else r->push_back(static_cast<byte>(buf[i]));

                    eosIndex++;
                    if (eosIndex == eosLength) {//End of message

                        eosIndex = 0;
                        eosLength = -1;
                        sizeBuf.clear();
                        auto temp = r;
                        r = new vector<byte>();
                        //Simulate lag
                        if(simLagS==0) { lock_guard <mutex> lock(*m); (*data)->push_back(temp); }
                        else thread([&data, temp, &m] {
                            slp(simLagS);//Simulated Lag
                            lock_guard <mutex> lock(*m);
                            (*data)->push_back(temp);
                        }).detach();
                        continue;
                    }
                }
            }
        }
    }).detach();
}

void sendMaster(int socketFD, vector<byte>* a) {
    packetHeader h;
    h.length = a->size();
    auto temp = objectTracker();
    auto b = h.toBytes(&temp);

    mySend(socketFD, b.data(), b.size());
    mySend(socketFD, a->data(), a->size());
}



vector<vector<byte>*>* recieveFromMaster(mutex* m, vector<vector<byte>*>** data) {
    lock_guard<mutex> lock(*m);
    auto r = *data;
    *data = new vector<vector<byte>*>();
    return r;
}

vector<vector<object*>>* receiveObjectsFromMaster(vector<vector<byte>*>** data, mutex* m, objectTracker* t) {
    auto O = recieveFromMaster(m, data);
    if(!O) return nullptr;

    vector<vector<object*>>* r = new vector<vector<object*>>();
    for (int i = 0; i < O->size(); ++i) {
        vector<byte>* payload = (*O)[i];
        if(!payload) continue;
        r->push_back(bytesToL(*payload, t));
        delete payload;
    }

    delete O;
    return r;
}

//Func returning active connections' addresses
//Func for sending to an address
//Blocking func for recieving from an address
//Maybe different client and server classes with their own init

//Store messages until they're requested by a func
#define networkAddressVar(x) x(uint16_t, port)
classDefinition(networkAddress, networkAddressVar, macroVoid)
    string address;
endClass(networkAddress)

classDefinition(networker, macroVoid, macroVoid)
uint64_t time();
uint64_t lag;
void syncVariable(object* a, string s);
endClass(networker)

#define NetworkHeadervars(x) x(uint64_t, time)
classDefinition(NetworkHeader, NetworkHeadervars, macroVoid)
endClass(NetworkHeader)

int64_t timeOffset=0;
uint64_t time(){ return timeOffset + now_ms();}

//TODO for networker add lag func and time sync
//todo Lag func just sends a packet and times its echoed path back, periodically sends this
//todo Perhaps make a networker class that client and server inherit from or even just use as a var
//todo Add a time func that sends a synced time with the server, update this occasionally
#define clientVars(x)
classDefinition(networkClient, clientVars, macroVoid)
    vector<vector<byte>*>* data = new vector<vector<byte>*>();
    int sock;
    mutex m;
    objectTracker tout, tin;
    uint64_t delay;


    void connectTo(networkAddress netAdd) {
        sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(netAdd.port);
        prints("Starting to connect")
        if (-1==inet_pton(AF_INET, netAdd.address.c_str(), &addr.sin_addr)) {prints("Invalid address: " << netAdd.address) return;}
        prints("Starting to connect2")
        if(-1==connect(sock, (sockaddr*)&addr, sizeof(addr))) {prints("Failed to connect")
            fprintf(stderr, "connect failed: errno=%d (%s)\n", errno, strerror(errno));
            return;}
        prints("Connected to Server")
        receiveMasterFunction(&m, sock, &data);
        auto nn = new NetworkHeader();
        uint64_t sentAt = now_ms();
        sendObjectToServer(nn);
        while(1){//Don't need to sleep because this is initialization
            auto a = receiveObjectFrom();
            if(a.size()){
                delay=(now_ms()-sentAt)/2;
                timeOffset = dynamic_cast<NetworkHeader*>(a[0])->time + delay - now_ms();
                return;
            }
        }
    }


    void sendToServer(vector<byte>* a) {
        sendMaster(sock, a);
    }

    vector<vector<byte>*>* recieveFromServer() {
        return recieveFromMaster(&m, &data);
    }

    void sendObjectToServer(object* a) {
        auto b = tout.objectToBytesWithPointedToObjects(a);
        sendToServer(b);
    }

    vector<object*> receiveObjectFrom() {
        vector<object*> r;
        auto O = receiveObjectsFrom();
        if(!O) return {};
        for (int j = 0; j < O->size(); ++j) {r.push_back((*O)[j][0]);}
        delete O;
        return r;
    }

    vector<vector<object*>>* receiveObjectsFrom() {
        auto O = recieveFromServer();
        if(!O) return nullptr;

        vector<vector<object*>>* r = new vector<vector<object*>>();
        for (int i = 0; i < O->size(); ++i) {
            vector<byte>* payload = (*O)[i];
            if(!payload) continue;
            r->push_back(bytesToL(*payload, &tin));
            delete payload;
        }

        delete O;
        return r;
    }

endClass(networkClient)

uint32_t string2Add(string a){
    uint32_t r=0;
    string temp;
    for (int j = 0; j < a.length(); ++j) {
        if(a[j]=='.') {
            r=r<<8;
            unsigned long v = std::stoul(temp);
            r+=static_cast<uint32_t>(v);
            temp="";
        }
        else{
            temp.push_back(a[j]);
        }
    }
    r=r<<8;
    unsigned long v = std::stoul(temp);
    r+=static_cast<uint32_t>(v);
    return r;
}

#define serverVars(x) //x(vector<int>, clients)//x(hashtable(string, int), addToFD)
classDefinition(networkServer, serverVars, macroVoid)
    AutoLock<vector<int>> clients;
    AutoLock<hashtable(int, vector<vector<byte>*>**)> data;
    AutoLock<hashtable(int, mutex*)> mutexes;
    AutoLock<hashtable(int, objectTracker)> tout, tin;
    char ip_str[INET_ADDRSTRLEN];
    int port = 6969;
    networkAddress addy{};

    networkServer() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
//        addr.sin_addr.s_addr = htonl(string2Add(myIP));;
        addr.sin_addr.s_addr = INADDR_ANY;
        while(-1==::bind(server_fd, (sockaddr *) &addr, sizeof(addr))) {
            prints("Error: " << strerror(errno) << ", trying again!")
            addr.sin_port = htons(++port);
        }
        inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str));
        prints("Server success! Address: " << ip_str << " Port: " << ntohs(addr.sin_port))

        listen(server_fd, 16);
        thread([this, server_fd] {
            while(1) {
                int client_fd = accept(server_fd, nullptr, nullptr);
                clients->push_back(client_fd);
                data->operator[](client_fd) = new vector<vector<byte>*>*(new vector<vector<byte>*>());
                mutexes[client_fd] = new mutex;
                prints("New Connection!")
                receiveMasterFunction(mutexes[client_fd], client_fd, (data->operator[](client_fd)), &clients);
                thread([this, client_fd]{
                    while(1){
                        auto a = receiveObjectFrom(client_fd);
                        if (a.size()) {
                            auto nethead = dynamic_cast<NetworkHeader *>(a[0]);
                            nethead->time = now_ms();
                            sendObjectTo(client_fd, nethead);
                            return;
                        }
                    }
                }).detach();
            }
        }).detach();
    }

    networkAddress getAddress() {
        networkAddress r;
        r.address = string(ip_str); r.port = port;
        return r;
    }

    void sendTo(int client, vector<byte>* a) {
        sendMaster(client, a);
    }

    void sendObjectTo(int client, object* a) {
        auto b = tout->operator[](client).objectToBytesWithPointedToObjects(a);
        sendTo(client, b);
    }

    vector<object*> receiveObjectFrom(int client) {
        vector<object*> r;
        auto O = receiveObjectsFrom(client);
        if(!O) return {};
        for (int j = 0; j < O->size(); ++j) {r.push_back((*O)[j][0]);}
        delete O;
        return r;
    }

    void sendObjectsTo(int client, vector<object*>* a) {
        auto B = LtoBytes(*a, &(tout->operator[](client)));
        sendTo(client, &B);
    }

    vector<vector<object*>>* receiveObjectsFrom(int client) {
        auto O = recieveFrom(client);
        if(!O) return nullptr;

        vector<vector<object*>>* r = new vector<vector<object*>>();
        for (int i = 0; i < O->size(); ++i) {
            vector<byte>* payload = (*O)[i];
            if(!payload) continue;
            r->push_back(bytesToL(*payload, &(tin->operator[](client))));
            delete payload;
        }

        delete O;
        return r;
    }

    vector<vector<byte>*>* recieveFrom(int client) {
        return recieveFromMaster(mutexes[client], (data->operator[](client)));
    }

endClass(networkServer)
