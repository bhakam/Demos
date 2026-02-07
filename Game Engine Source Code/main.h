//
// Created by Brian Hakam on 10/7/25.
//
#pragma once

#include <SDL2/SDL.h>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <limits>
#include <cstring>
#include <omp.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <cstdint>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <future>
#include <mutex>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#define tickRate (1.0/45)
#define simLagS 0
#define myIP "0.0.0.0"

using namespace std;
bool running = true;

#define numThreads 16

#include "ClassMacros.cpp"
#include "DataStructures.cpp"
#include "Networker.cpp"
#include "renderer.cpp"
#include "shapes.cpp"

#include "Entities/Entity.cpp"
//#include "Physics.cpp"

#include "GameServerAndClient.cpp"

vector<type_info> types;

