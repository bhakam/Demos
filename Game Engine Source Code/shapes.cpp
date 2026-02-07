#pragma once

#define AVG2(V) AVG(x, V); AVG(y, V); AVG(z, V)
#define AVG(d, V) avg.d += tris[i].V.d

static Vec3 avgPoint(triV tris){
    if (tris.empty()) return {0,0,0};
    Vec3 avg = {0,0,0};
    for (int i = 0; i < tris.size(); ++i) {
        AVG2(v0); AVG2(v1); AVG2(v2);
    }
    avg.x/=3*tris.size(); avg.y/=3*tris.size(); avg.z/=3*tris.size();
    return avg;
}

static float distance(Vec3 a, Vec3 b){
    float x = a.x-b.x;
    float y = a.y-b.y;
    float z = a.z-b.z;
    return sqrt(x*x + y*y + z*z);
}

static float maxVertexDeviationFromAvg(triV tris){
    if (tris.empty()) return 0;
    Vec3 avg = avgPoint(tris);
    float maxDistance = 0;
    for (int i = 0; i < tris.size(); ++i) {
        float dist = distance(avg, tris[i].v0);
        if(dist>maxDistance) maxDistance = dist;
        dist = distance(avg, tris[i].v1);
        if(dist>maxDistance) maxDistance = dist;
        dist = distance(avg, tris[i].v2);
        if(dist>maxDistance) maxDistance = dist;
    }
    return maxDistance;
}

static triV center(triV tris){
    Vec3 avg = avgPoint(tris);

    for (int i = 0; i < tris.size(); ++i) {
        tris[i].v0.x-=avg.x;
        tris[i].v0.y-=avg.y;
        tris[i].v0.z-=avg.z;
        tris[i].v1.x-=avg.x;
        tris[i].v1.y-=avg.y;
        tris[i].v1.z-=avg.z;
        tris[i].v2.x-=avg.x;
        tris[i].v2.y-=avg.y;
        tris[i].v2.z-=avg.z;
    }
    return tris;
}


static triV buildTriangle(uint32_t color, float width, float height){
    triV tris;
    tris.push_back({{width/2,0,0},{-width/2,0,0},{0,height,0}, color});
    return tris;
}

static triV equilateralTriangle(uint32_t color, float width){
    float height = width/2 * sqrt(3);
    triV tris;
    tris.push_back({{width/2,0,0},{-width/2,0,0},{0,height,0}, color});
    return tris;
}

static triV buildRightTriangle(uint32_t color, float width, float height){
    triV tris;
    tris.push_back({{width,0,0},{0,height,0}, {0,0,0}, color});
    return tris;
}

static triV buildRectangle(uint32_t color, float width, float height){
    triV tris;
    for (int i = 0; i < 2; ++i) tris += buildRightTriangle(color, width, height);
    tris[1].rotate(180, 180).move({width,height,0});
    return tris;
}


static triV buildRecPrism(uint32_t color, float width, float height, float depth){
    triV tris;
    auto rec1= buildRectangle(color, width, height);
    for (int i = 0; i < rec1.size(); ++i) rec1[i].flip();
    tris += rec1;
    for (int i = 0; i < rec1.size(); ++i) rec1[i].flip().move({0,0,depth});//.rotate(180, 0);
    tris += rec1;

    auto rec2= buildRectangle(color, depth, height);
    for (int i = 0; i < rec2.size(); ++i) rec2[i].rotate(-90, 0);
    tris += rec2;
    for (int i = 0; i < rec2.size(); ++i) rec2[i].flip().move({width,0,0});
    tris += rec2;

    auto rec3= buildRectangle(color, width, depth);
    for (int i = 0; i < rec3.size(); ++i) rec3[i].rotate(0, 90);
    tris += rec3;
    for (int i = 0; i < rec3.size(); ++i) rec3[i].flip().move({0,height,0});
    tris += rec3;

    return tris;
}

static triV triangularPrism(uint32_t color, float s = 1.0f){
    const float R = s / std::sqrt(3.0f);
    const float h = std::sqrt(2.0f / 3.0f) * s;

    Vec3 A{  R,               0.0f, 0.0f };
    Vec3 B{ -R * 0.5f,  +0.5f * s, 0.0f };
    Vec3 C{ -R * 0.5f,  -0.5f * s, 0.0f };
    Vec3 D{ 0.0f, 0.0f, h };

    triV tris; tris.reserve(4);
    tris.push_back({ B, A, C, color });
    tris.push_back({ A, B, D, color });
    tris.push_back({ B, C, D, color });
    tris.push_back({ C, A, D, color });
    return tris;
}