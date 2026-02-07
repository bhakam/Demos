//
// Created by Brian Hakam on 10/7/25.
//

//Colors
#define white 0x00FFFFFF
#define blue 0x000000FF
#define green 0x0000FF00
#define red 0x00FF0000
#define orange 0x00FF5C00
#define black 0x00000000

//Degree & Radian conversions
#define deg2rad  * 0.01745329251994329577f
#define rad2deg  / 0.01745329251994329577f

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
};

vector<byte> autoToBytes(Vec3* a, objectTracker* t){
    vector<float> r;
    r.push_back(a->x); r.push_back(a->y); r.push_back(a->z);
    return autoToBytes(&r, t);
}

void autoFromBytes(Vec3* a, int* i, vector<byte> bytes, objectTracker* t){
    vector<float> r;
    autoFromBytes(&r, i, bytes, t);
    a->x = r[0]; a->y = r[1]; a->z = r[2];
}

static inline float dot(const Vec3& a, const Vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
static inline float length(const Vec3& v) { return std::sqrt(dot(v, v)); }
static inline Vec3 normalize(const Vec3& v) {
    float len = length(v); if (len <= 1e-8f) return v; return v * (1.0f / len);
}

static inline Vec3 rotateAroundAxis(const Vec3& v, const Vec3& axis, float angle) {
    Vec3 u = normalize(axis);
    float c = std::cos(angle), s = std::sin(angle);
    return v * c + cross(u, v) * s + u * (dot(u, v) * (1.0f - c));
}

static inline Vec3 rotateYawPitch0(Vec3 v, float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f){
    v = rotateAroundAxis(v, {0, 1, 0}, yawDegrees);
    v = rotateAroundAxis(v, {1, 0, 0}, pitchDegrees);
    return v;
}

static inline Vec3 rotateYawPitch(Vec3 v, float yawDegrees, float pitchDegrees, float rollDegrees = 0.0f){
    const float yaw   = yawDegrees   deg2rad;
    const float pitch = pitchDegrees deg2rad;
    const float roll  = rollDegrees  deg2rad;

    v = rotateAroundAxis(v, {0, 1, 0}, yaw);
    Vec3 right = rotateAroundAxis({1, 0, 0}, {0, 1, 0}, yaw);
    v = rotateAroundAxis(v, right, pitch);

    Vec3 forward = {0, 0, 1};
    forward = rotateAroundAxis(forward, {0, 1, 0}, yaw);
    forward = rotateAroundAxis(forward, right, pitch);
    v = rotateAroundAxis(v, forward, roll);

    return v;
}


struct Triangle { Vec3 v0, v1, v2; uint32_t color;

    Triangle& move(const Vec3& delta) {
        v0 += delta; v1 += delta; v2 += delta;
        return *this;
    }

    Triangle& rotate(float angleY, float angleX) {
        auto rotP = [&](const Vec3& p) {
            Vec3 q = p;
            q = rotateAroundAxis(q, {0, 1, 0}, angleY * 0.01745329251994329577f); // yaw
            q = rotateAroundAxis(q, {1, 0, 0}, angleX * 0.01745329251994329577f); // pitch
            return q;
        };
        v0 = rotP(v0); v1 = rotP(v1); v2 = rotP(v2);
        return *this;
    }

    Triangle& flip(){
        std::swap(this->v1, this->v2);
        return *this;
    }

};

class triV: public std::vector<Triangle>{
public:
    triV& move(const Vec3& a) {
        for (auto& t : *this) t.move(a);
        return *this;
    }
    triV& rotate(double pitch, double yaw) {
        for (auto& t : *this) t.rotate(pitch, yaw);
        return *this;
    }
};

triV& operator+=(triV& lhs, const triV& rhs) {
    lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    return lhs;
}

Vec3 getCoordClosestTo(triV t, Vec3 target){//TODO
    Vec3 r = t[0].v0;
    return r;
}


static inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
    return (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
}

static inline float clamp01(float x){ return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }

static inline uint32_t shadeColor(uint32_t c, float k){
    k = clamp01(k);
    uint8_t a = (c >> 24);
    uint8_t r = (c >> 16);
    uint8_t g = (c >>  8);
    uint8_t b = (c >>  0);
    r = uint8_t(r * k);
    g = uint8_t(g * k);
    b = uint8_t(b * k);
    return (uint32_t(a)<<24) | (uint32_t(r)<<16) | (uint32_t(g)<<8) | uint32_t(b);
}

#define camvars(x) x(Vec3, pos) x(float, yaw) x(float, pitch)
classDefinition(Camera, camvars, macroVoid)
    Camera(){
        pos={0, 0, -5};
        yaw = -180 deg2rad;   // left-right (radians)
        pitch = 0.0f; // up-down (radians)
    }

    void basis(Vec3& right, Vec3& up, Vec3& forward) const {
        forward = { std::cos(pitch) * std::sin(yaw), std::sin(pitch), std::cos(pitch) * std::cos(yaw) };
        Vec3 worldUp{0, 1, 0};
        right = normalize(cross(worldUp, forward)); // right-handed
        up = normalize(cross(forward, right));
    }

    Vec3 toCamera(const Vec3& p) const {
        Vec3 r,u,f; basis(r,u,f);
        Vec3 rel = p - pos;
        return { dot(rel, r), dot(rel, u), dot(rel, f) }; // camera looks along +Z
    }
endClass(Camera)


static inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
    return { a.x + (b.x - a.x)*t, a.y + (b.y - a.y)*t, a.z + (b.z - a.z)*t };
}

static std::vector<Vec3> clipToNear(const std::vector<Vec3>& poly, float nearPlane) {
    std::vector<Vec3> out;
    if (poly.empty()) return out;

    Vec3 S = poly.back();
    float dS = S.z - nearPlane;
    for (const Vec3& E : poly) {
        float dE = E.z - nearPlane;
        bool Ein = dE >= 0.0f, Sin = dS >= 0.0f;

        if (Ein) {
            if (!Sin) {
                float t = dS / (dS - dE);
                out.push_back(lerp(S, E, t));
            }
            out.push_back(E);
        } else if (Sin) {
            float t = dS / (dS - dE);
            out.push_back(lerp(S, E, t));
        }
        S = E; dS = dE;
    }
    return out;
}

// Clip val convex polygon against z <= farPlane (optional but nice to have).
static std::vector<Vec3> clipToFar(const std::vector<Vec3>& poly, float farPlane) {
    std::vector<Vec3> out;
    if (poly.empty()) return out;

    Vec3 S = poly.back();
    float dS = S.z - farPlane;
    for (const Vec3& E : poly) {
        float dE = E.z - farPlane;
        bool Ein = dE <= 0.0f, Sin = dS <= 0.0f;

        if (Ein) {
            if (!Sin) {
                float t = dS / (dS - dE);
                out.push_back(lerp(S, E, t));
            }
            out.push_back(E);
        } else if (Sin) {
            float t = dS / (dS - dE);
            out.push_back(lerp(S, E, t));
        }
        S = E; dS = dE;
    }
    return out;
}


class Renderer {
public:
    int width, height;
    float fovDeg = 90.0f;
    float nearPlane = 0.05f;
    float farPlane = 1000.0f;
    bool cull = true;

    SDL_Window* window = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    SDL_Texture* frameTex = nullptr;

    std::vector<uint32_t> colorBuf;
    std::vector<float> depthBuf;    // Z buffer

    Vec3 lightDir{ 1, 1, 0 }; //Direction of ambient light/sun
    float ambient = 0.5f;
    float diffuse = 0.7f;
    bool  twoSided = false; //Render both faces of a traingle

    Renderer(int width = 1920/2, int height = 1080/2){
        if (!init("SDL2 Software 3D Renderer", width, height)) {
            SDL_Log("Renderer init failed: %s", SDL_GetError());
            exit(1);
        }

        SDL_SetWindowSize(window, width, height);
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }

    ~Renderer() {
        if (frameTex) SDL_DestroyTexture(frameTex);
        if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool init(const char* title, int w, int h) {
        width = w; height = h;
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
            SDL_Log("SDL_Init Error: %s", SDL_GetError());
            return false;
        }
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
        if (!window) { SDL_Log("CreateWindow Error: %s", SDL_GetError()); return false; }
        sdlRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
        if (!sdlRenderer) { SDL_Log("CreateRenderer Error: %s", SDL_GetError()); return false; }
        frameTex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!frameTex) { SDL_Log("CreateTexture Error: %s", SDL_GetError()); return false; }
        colorBuf.resize(size_t(width) * size_t(height));
        depthBuf.resize(size_t(width) * size_t(height));
        lightDir = normalize(lightDir);

        return true;
    }



    void clear(uint32_t clearColor = rgba(20,20,26,255)) {
        int numCols = sqrt(numThreads);
        int numRows = sqrt(numThreads);
        int tileWidth = width / numCols;
        int tileHeight = height / numRows;
#pragma omp parallel for collapse(2)
        for (int j = 0; j < numRows; ++j) {
            int miny = j * tileHeight;
            int maxy = (j == numRows - 1) ? height : (j + 1) * tileHeight;

            for (int k = 0; k < numCols; ++k) {
                int minx = k * tileWidth;
                int maxx = (k == numCols - 1) ? width : (k + 1) * tileWidth;
                for (int x = minx; x < maxx; ++x) {
                    for (int y = miny; y < maxy; ++y) {
                        size_t idx = size_t(y) * size_t(width) + size_t(x);
                        colorBuf[idx] = clearColor;
                        depthBuf[idx] = std::numeric_limits<float>::infinity();
                    }
                }
            }
        }
        (depthBuf.begin(), depthBuf.end(), numeric_limits<float>::infinity());
    }

    bool project(const Vec3& cam, float& outX, float& outY, float& outZ) const {
        if (cam.z <= nearPlane || cam.z > farPlane) return false; //Plane clipping
        float aspect = float(width) / float(height);
        float f = 1.0f / std::tan((fovDeg * 0.5f) * float(M_PI) / 180.0f);
        float x_ndc = (cam.x * (f / aspect)) / cam.z;
        float y_ndc = (cam.y * f) / cam.z;
        outX = (x_ndc * 0.5f + 0.5f) * float(width);
        outY = (-y_ndc * 0.5f + 0.5f) * float(height);
        outZ = cam.z;
        return true;
    }

    static inline float edgeFn(float x0, float y0, float x1, float y1, float x, float y) {
        return (x - x0) * (y1 - y0) - (y - y0) * (x1 - x0);
    }

    void setPixel(int x, int y, float z, uint32_t color) {
        if (x < 0 || y < 0 || x >= width || y >= height) return;
        size_t idx = size_t(y) * size_t(width) + size_t(x);
        if (z < depthBuf[idx]) { depthBuf[idx] = z; colorBuf[idx] = color; }
    }

    void fillTriangle(float x0, float y0, float z0,
                      float x1, float y1, float z1,
                      float x2, float y2, float z2,
                      uint32_t color, int minx = 0, int maxx = 0, int miny = 0, int maxy = 0) {
        if(!maxx) maxx = width - 1;
        if(!maxy) maxy = height - 1;

        int minX = std::max(minx, (int) std::floor(std::min({x0, x1, x2})));
        int minY = std::max(miny, (int) std::floor(std::min({y0, y1, y2})));
        int maxX = std::min(maxx, (int) std::ceil(std::max({x0, x1, x2})));
        int maxY = std::min(maxy, (int) std::ceil(std::max({y0, y1, y2})));

        float area = edgeFn(x0, y0, x1, y1, x2, y2);
        if (std::fabs(area) < 1e-5f) return; // degenerate

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                float px = x + 0.5f, py = y + 0.5f;
                float w0 = edgeFn(x1, y1, x2, y2, px, py);
                float w1 = edgeFn(x2, y2, x0, y0, px, py);
                float w2 = edgeFn(x0, y0, x1, y1, px, py);

                if ((area > 0 && w0 >= 0 && w1 >= 0 && w2 >= 0) ||
                    (area < 0 && w0 <= 0 && w1 <= 0 && w2 <= 0)) {
                    w0 /= area;
                    w1 /= area;
                    w2 /= area;
                    float z = w0 * z0 + w1 * z1 + w2 * z2; // simple depth interpolate
                    setPixel(x, y, z, color);
                }
            }
        }
    }

    void drawTriangleWorld(const Triangle& tri, const Camera& cam, int minx = 0, int maxx = 0, int miny = 0, int maxy = 0) {
        Vec3 e1 = tri.v1 - tri.v0;
        Vec3 e2 = tri.v2 - tri.v0;
        Vec3 n  = normalize(cross(e1, e2));
        float ndotl = dot(n, lightDir);
        if (twoSided && ndotl < 0.0f) ndotl = -ndotl;
        float lambert   = clamp01(ndotl);
        float intensity = clamp01(ambient + diffuse * lambert);
        uint32_t litColor = shadeColor(tri.color, intensity);

        Vec3 a = cam.toCamera(tri.v0);
        Vec3 b = cam.toCamera(tri.v1);
        Vec3 c = cam.toCamera(tri.v2);

        const float eps = 1e-4f;
        std::vector<Vec3> poly = { a, b, c };
        poly = clipToNear(poly, nearPlane + eps);
        if (poly.size() < 3) return;                // fully clipped
        poly = clipToFar(poly,  farPlane  - eps);   // optional; keep if you want strict far clipping
        if (poly.size() < 3) return;


        float x0, y0, z0;
        if (!project(poly[0], x0, y0, z0)) return;  // should succeed post-clip

        for (size_t i = 1; i + 1 < poly.size(); ++i) {
            float x1, y1, z1, x2, y2, z2;
            if (!project(poly[i],   x1, y1, z1) || !project(poly[i+1], x2, y2, z2)) continue;

            float area = edgeFn(x0, y0, x1, y1, x2, y2);
            if (cull && area >= 0) continue;
            fillTriangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, litColor, minx, maxx, miny, maxy);

        }
    }


    void present() {
        // Upload color buffer to texture and present
        void* pixels = nullptr; int pitch = 0;
        if (SDL_LockTexture(frameTex, nullptr, &pixels, &pitch) == 0) {
            // pitch is bytes per row; our buffer is tightly packed ARGB8888
            uint8_t* dst = static_cast<uint8_t*>(pixels);
            const uint8_t* src = reinterpret_cast<const uint8_t*>(colorBuf.data());
            for (int y = 0; y < height; ++y) {
                std::memcpy(dst + y * pitch, src + y * width * 4, width * 4);
            }
            SDL_UnlockTexture(frameTex);
        }
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, frameTex, nullptr, nullptr);
        SDL_RenderPresent(sdlRenderer);
    }
    // In Renderer.h (or renderer.cpp)
    SDL_Texture* resolvedTex = nullptr;
    int resolvedW = 0, resolvedH = 0;
    std::vector<uint32_t> resolved; // BASE_W x BASE_H

    static inline uint32_t pack8(int r, int g, int b) {
        r = std::min(std::max(r,0),255);
        g = std::min(std::max(g,0),255);
        b = std::min(std::max(b,0),255);
        return 0xFF000000u | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
    }

    void presentSSAA(int ss, int outW, int outH) {
        // Downsample the internal color buffer (width x height) to outW x outH
        if ((int)resolved.size() != outW * outH) resolved.assign(outW * outH, 0);

        const int inW = width, inH = height;      // internal high-res buffers
        const int stride = inW;                   // uint32_t RGBA8
        const uint32_t* src = colorBuf.data();    // whatever your color buffer is called

        // Box filter ss×ss (use linear for speed; tent or gamma-correct after)
#pragma omp parallel for collapse(2)

        for (int y = 0; y < outH; ++y) {
            for (int x = 0; x < outW; ++x) {
                int r = 0, g = 0, b = 0;
                const int sx0 = x * ss;
                const int sy0 = y * ss;
                for (int dy = 0; dy < ss; ++dy) {
                    const uint32_t* row = src + (sy0 + dy) * stride + sx0;
                    for (int dx = 0; dx < ss; ++dx) {
                        uint32_t c = row[dx];
                        r += (c >> 16) & 255;
                        g += (c >>  8) & 255;
                        b += (c      ) & 255;
                    }
                }
                const int n = ss * ss;
                resolved[y * outW + x] = pack8(r / n, g / n, b / n);
            }
        }

        // Create/update val texture at the downsampled size
        if (!resolvedTex || resolvedW != outW || resolvedH != outH) {
            if (resolvedTex) SDL_DestroyTexture(resolvedTex);
            resolvedTex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
                                            SDL_TEXTUREACCESS_STREAMING, outW, outH);
            resolvedW = outW; resolvedH = outH;
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // nice scaling if window != out size
            SDL_RenderSetLogicalSize(sdlRenderer, outW, outH); // optional letterbox
        }

        SDL_UpdateTexture(resolvedTex, nullptr, resolved.data(), outW * 4);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, resolvedTex, nullptr, nullptr);
        SDL_RenderPresent(sdlRenderer);
    }

    void render(triV scene, int SS, Camera cam){
        clear(0x0087CEEB);
        int numCols = sqrt(numThreads);
        int numRows = sqrt(numThreads);
        int tileWidth = width / numCols;
        int tileHeight = height / numRows;

#pragma omp parallel for collapse(2)
        for (int j = 0; j < numRows; ++j) {
            int miny = j * tileHeight;
            int maxy = (j == numRows - 1) ? height : (j + 1) * tileHeight;

            for (int k = 0; k < numCols; ++k) {
                int minx = k * tileWidth;
                int maxx = (k == numCols - 1) ? width : (k + 1) * tileWidth;

                for (int i = 0; i < scene.size(); ++i) drawTriangleWorld(scene[i], cam, minx, maxx, miny, maxy);
            }
        }
//        r.present();
        presentSSAA(SS, width/SS, height/SS);
    }

};
