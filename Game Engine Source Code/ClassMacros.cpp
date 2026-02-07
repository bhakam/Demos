//
// Created by Brian Hakam on 11/21/25.
//
#include <iostream>
#include "string"
#include <vector>
#include <type_traits>
#include <unordered_map>
#include <omp.h>
#include "main.h"

template<typename T>
void myclamp(T* a, T limit){
    *a = clamp(*a, -limit, limit);
}

class objectTracker;

class object;
struct ObjectPtrHash {
    std::size_t operator()(const object* p) const noexcept {
        return reinterpret_cast<std::uintptr_t>(p);
    }
};

struct ObjectPtrEqual {
    bool operator()(const object* a, const object* b) const noexcept {
        return a == b;
    }
};

#define objectHashtable(x) unordered_map<object*, x, ObjectPtrHash, ObjectPtrEqual>
//objectHashtable(int) addressToID;

#define hashtable(x, y) unordered_map<x, y>

#define macroVoid(x) \

#define macrovoid macroVoid

#define macroVoid2(x, y) \


template <typename T, typename Alloc>
vector<T, Alloc>& operator+=(vector<T, Alloc>& lhs,
const vector<T, Alloc>& rhs)
{
lhs.reserve(lhs.size() + rhs.size());          // optional but efficient
lhs.insert(lhs.end(), rhs.begin(), rhs.end()); // concatenate
return lhs;
}

#define prints(x) std::cout << x << std::endl;

#define printVar(x) std::cout << #x << ": " << x << std::endl;
#define displayVars(type, name) type name{};
#define autoPrintVars(type, name)  autoPrint(&name, #name);
#define autoPrintInheritance(type)  prints(string(typeid(type).name())) type::print();
#define printVars(type, name) std::cout << #name << " = " << name << std::endl;
#define printVarsNameOverride(type, name, stringName) std::cout << stringName << " = " << name << std::endl;

#define toBytesNum(type) \
vector<byte> toBytes(type* a, objectTracker* t){ \
    vector<byte> r; \
    for (int i = (sizeof *a) - 1; i >= 0; --i) r.push_back(static_cast<byte>((*a >> (8*i)) & 0xFF)); \
    return r; \
}


#define nums(x) x(bool) x(short) x(int) x(long) x(uint8_t) x(uint16_t) x(uint32_t) x(uint64_t)
#define floats(x) x(float) x(double)
#define primitiveTypes(x) nums(x)

nums(toBytesNum)

template<typename T>
void autoPrint(T* a, string name);

template<typename T>
void autoPrint(vector<T>* a, string name) {
for(int i=0; i<a->size(); i++) autoPrint(&(*a)[i], name+"["+to_string(i)+"]");
}

template <typename T>
constexpr bool is_streamable_v = requires(std::ostream& os, const T& v) { os << v; };

template<typename T>
void autoPrint(T* a, string name) {
    if constexpr (is_base_of_v<object, T>) {
        prints(string(typeid(T).name())+" "+name)
        a->print();
    }
    else if constexpr (is_streamable_v<T>) printVarsNameOverride(T, *a, name)
    else std::cout << name << " = <" << typeid(T).name() << ">" << std::endl;
    if constexpr (is_pointer_v<T>){
        if(*a) autoPrint((*a), name);
    }
}

#define PI 3.1415926535897932384626433

//Ignorable types for to/from bytes functions
#define ignorables (0 || 0)

template<typename T, std::size_t N>
vector<byte> toBytesArray(T (&arr)[N]);

class objectTracker;

template <typename T>
vector<byte> toBytesPointer(T**, objectTracker*);

template <typename T>
vector<byte> toBytes(vector<T>* a, objectTracker* t);

template <typename T, typename U>
vector<byte> toBytes(hashtable(T, U)* a, objectTracker* t){
    vector<byte> r;
    vector<T> forward;
    vector<U> backward;
    for (const auto& [key, value] : *a) {
        forward.push_back(key);
        backward.push_back(value);
    }
    r+=toBytes(&forward, t);
    r+=toBytes(&backward, t);
    return r;
}

template<typename T>
vector<byte> autoToBytes(T* a, objectTracker* t) {
    if constexpr (std::is_array_v<T>) return toBytesArray(a);
    else if constexpr (is_pointer_v<T>) return toBytesPointer(a, t);
    else if constexpr (ignorables) return vector<byte>{};
    else if constexpr (is_class_v<T> && is_base_of_v<object, T>) return a->toBytes(t);
    else if constexpr (is_same_v<T, float>) return ::toBytes(reinterpret_cast<int*>(a), t);
    else if constexpr (is_same_v<T, double>) return ::toBytes(reinterpret_cast<long*>(a), t);
    else return ::toBytes(a, t);
}

template <typename T>
vector<byte> toBytes(vector<T>* a, objectTracker* t){
    vector<byte> r;
    int size = a->size();
    r += autoToBytes(&size, t);
    for (int i = 0; i < size; ++i) r+=autoToBytes(&((*a)[i]), t);
    return r;
}

template <typename K, typename V, typename Hash, typename Eq, typename Alloc>
vector<byte> toBytes(std::unordered_map<K, V, Hash, Eq, Alloc>* a, objectTracker* t){
    vector<byte> r;
    vector<K> forward;
    vector<V> backward;
    for (const auto& [key, value] : *a) {
        forward.push_back(key);
        backward.push_back(value);
    }
    r += toBytes(&forward, t);
    r += toBytes(&backward, t);
    return r;
}

template <typename K, typename V, typename Hash, typename Eq, typename Alloc>
void fromBytes(std::unordered_map<K, V, Hash, Eq, Alloc>* a, int* i, vector<byte> bytes, objectTracker* t) {
    a->clear();
    vector<K> forward;
    vector<V> backward;
    autoFromBytes(&forward, i, bytes, t);
    autoFromBytes(&backward, i, bytes, t);
    for (int j = 0; j < (int)forward.size(); ++j) {
        (*a)[forward[j]] = backward[j];
    }
}




template<typename T, std::size_t N>
vector<byte> toBytesArray(T (&arr)[N]) {
    vector<byte> r;
    for (std::size_t i = 0; i < N; ++i) {
        r += autoToBytes(&arr[i]);
    }
    return r;
}


template<typename T>
inline void fromBytes(T* a, int* i , vector<byte> bytes, objectTracker* t = nullptr) {
    if constexpr (ignorables) return;
    else {
        *a = 0;
        for (size_t j = 0; j < sizeof(T); ++j, (*i)++) {
            *a <<= 8;
            *a |= to_integer<unsigned>(bytes[*i]);
        }
    }

}

template<typename T>
void fromBytesPointer(T** a, int* i, vector<byte> bytes, objectTracker* t);


template<typename T>
void autoFromBytes(T* a, int* i , vector<byte> bytes, objectTracker* t) {
    if constexpr (is_array_v<T>) fromBytesArray(a, i, bytes);
    else if constexpr (is_pointer_v<T>) fromBytesPointer(a, i, bytes, t);
    else if constexpr (is_class_v<T> && is_base_of_v<object, T>) a->fromBytes(bytes, i, t);
    else if constexpr (is_same_v<T, float>) fromBytes(reinterpret_cast<int *>(a), i, bytes);
    else if constexpr (is_same_v<T, double>) fromBytes(reinterpret_cast<long *>(a), i, bytes);
    else fromBytes(a, i, bytes, t);
}


template <typename T>
void fromBytes(vector<T>* a, int* i, vector<byte> bytes, objectTracker* t){
    a->clear();
    int size;
    autoFromBytes(&size, i, bytes, t);
    a->resize(size);
    for (int j = 0; j < size; ++j) autoFromBytes(&(*a)[j], i, bytes, t);
}

template <typename T, typename U>
void fromBytes(hashtable(T, U)* a, int* i, vector<byte> bytes, objectTracker* t) {
    a->clear();
    vector<T> forward;
    vector<U> backward;
    autoFromBytes(&forward, i, bytes, t);
    autoFromBytes(&backward, i, bytes, t);
    for (int j = 0; j < forward.size(); ++j) {
        (*a)[forward[j]]=backward[j];
    }
}

//todo: implement to/from bytes for fixed length arrays
//template<typename T, std::size_t N>
//void fromBytesArray(T (&arr)[N], int* i, vector<byte> bytes) {
//    for (std::size_t idx = 0; idx < N; ++idx) {
//        autoFromBytes(&arr[idx], i, bytes);
//    }
//}



#define inhertianceDefault(inheritance)  public inheritance
#define inhertianceHeader(inheritance) , inhertianceDefault(inheritance)
#define inhertianceHeaderFirst(inheritance)  : inhertianceDefault(inheritance)


#define addToByteVec(type, name) r += autoToBytes(&(this->name), t);
#define addClassToByteVec(myClass) r += myClass::toBytes(t);;

#define fromBytesToTM(type, name)  autoFromBytes(&name, i, bytes, tt);
#define fromBytesToTClass(myClass)  myClass::fromBytes(bytes, i, tt);

class object{
public:
    inline static int newID = 0;
    virtual int classIntID(){return 0;}
    virtual int size() {return sizeof *this;}
    object() {
    }
    virtual string classID(){return string(typeid(object).name());}
    virtual void print(){}
    virtual vector<byte> toBytes(objectTracker* t){return vector<byte>{};}
    virtual void fromBytes(vector<byte> bytes, int* i, objectTracker* t){}
    static object* constructor() {return new object();}
};

unordered_map<int, object* (*)()> constructors;

int nextTypeIdAndRegister(object* (*ctor)()) {
    static int counter = 0;
    int id = counter++;
    constructors[id] = ctor;
    return id;
}

template<typename T>
int getTypeId() {
    static const int id = nextTypeIdAndRegister(&T::constructor);
    return id;
}
#define to5(x) x(1) x(2) x(3) x(4) x(5)
#define to5Dual(x, y) x(1, y) x(2, y) x(3, y) x(4, y) x(5, y)

#define CALL_IF(obj, method, ...)                                           \
    do {                                                                    \
        auto&& _obj = (obj);                                                \
        auto _call_if_impl = [&]<class T>(T&& o) { if constexpr (requires(T t) { t.method(__VA_ARGS__); }) { o.method(__VA_ARGS__); } };                                                                  \
        _call_if_impl(_obj);                                                \
    } while (false)

#define forEachMemberMacro(type, name) f(this->name);
#define forEachInheritanceMacro(type) f(*static_cast<type*>(this));
//TODO add that object to and from bytes has its header as the first thing


//Implement classes using this definition will allow compile time reflection,
//Along with automatic to/from byte methods
#define classDefinition(myName, vars, inheritances) \
class myName inhertianceHeaderFirst(virtual object) inheritances(inhertianceHeader){ \
public:                                             \
    static int newID;                             \
    vars(displayVars)                               \
                                                  \
 vector<byte> toBytes(objectTracker* t)override{        \
    vector<byte> r;                                 \
    vars(addToByteVec)                                \
    inheritances(addClassToByteVec)\
    return r;\
}\
void fromBytes(vector<byte> bytes, int* i, objectTracker* tt)override{            \
     vars(fromBytesToTM)                          \
     inheritances(fromBytesToTClass)\
}                                                 \
void print() override{                                       \
prints(#myName)\
vars(autoPrintVars)                               \
inheritances(autoPrintInheritance)                                                  \
}                                                 \
int size() override {return sizeof *this;}                 \
string classID() override {return string(typeid(myName).name());}                      \
static object* constructor() {return new myName();}    \
int classIntID() override{return newID;}                     \
template <typename F>\
void forEachMember(F&& f) {\
    vars(forEachMemberMacro);                       \
inheritances(forEachInheritanceMacro)\
}\

#define endClass(name) \
};                     \
int name::newID = getTypeId<name>(); \

#define registerClassT(name, T1, T2) template<> int name<T1, T2>::newID = getTypeId<name<T1, T2>>();


#define endClassT(name) \
};                      \
registerClassT(name, int, double)

#define objectHeadervars(x) x(int, objClassID) x(int, objID)
classDefinition(objectHeader, objectHeadervars, macroVoid) endClass(objectHeader)

vector<object*> bytesToL(vector<byte> bytes, objectTracker* t);

classDefinition(objectTracker, macroVoid, macroVoid)
    objectHashtable(int) addressToID;
    hashtable(int, object*) idToAddress;

    objectHashtable(int) seen;
    vector<object*> toAdd;
    int currentObjID = 0;

    objectTracker(){
    }

    void clear(){
        seen.clear();
        toAdd.clear();
    }

    void appendObject(object* a){
        if(!a || seen.contains(a)) return;
        seen[a]=1;
        toAdd.push_back(a);
    }

    object* findObject(objectHeader h) {
        if(h.objID==-1 && h.objClassID==-1) return nullptr;
        if(idToAddress.contains(h.objID)) return idToAddress[h.objID];
        else {
            object* newO = constructors[h.objClassID]();
            addObject(&h, newO);
            return newO;
        }
    }
    objectHeader addObject(object* a) {
        objectHeader h;
        if (!a) { h.objID = -1; h.objClassID = -1; return h; }
        if(addressToID.contains(a)) return getObjectHeader(a);//Already added
        addressToID[a] = currentObjID;
        idToAddress[currentObjID] = a;
        h.objID=currentObjID++;
        h.objClassID = a->classIntID();
        return h;
    }
    void addObject(objectHeader* h, object* a) {//Adding object created by other system
        if(h->objID>currentObjID) currentObjID = h->objID + 1;
        addressToID[a] = h->objID;
        idToAddress[h->objID] = a;
    }
    objectHeader getObjectHeader(object* a) {
        objectHeader h;
        h.objID = addressToID[a];
        h.objClassID = a->classIntID();
        return h;
    }

    vector<byte>* objectToBytesWithPointedToObjects(object* a){
        vector<byte>* b = new vector<byte>();
        this->clear();
        this->appendObject(a);
        int i=0;
        while(this->toAdd.size()>i) {
            auto obj = this->toAdd[i++];

            objectHeader h = this->addObject(obj);
            *b += autoToBytes(&h, this);
            *b += autoToBytes(obj, this);
        }
        return b;
    }

    object* getSingleObjectFrom(vector<byte> bytes){
        return bytesToL(bytes, this)[0];
    }

endClass(objectTracker)



template <typename T>
vector<byte> toBytesPointer(T** a, objectTracker* t){
    static_assert(is_base_of_v<object, T>,
                  "Pointer serialization only supported for object-derived types");
    auto obj = static_cast<object*>(*a);
    t->appendObject(obj);
    auto h = t->addObject(obj);
    return autoToBytes(&h, t);
}

vector<byte> LtoBytes(vector<object*> L, objectTracker* t){
    vector<byte> r;
    for (int i = 0; i < L.size(); ++i) {

        objectHeader h = t->addObject(L[i]);

        //TODO: Add Op id(change, delete), or maybe just add a delete object that just contains a pointer to an object
        r+=autoToBytes(&h, t);
        if(!L[i]) continue;
        r+=autoToBytes(L[i], t);
    }
    return r;
}

vector<object*> bytesToL(vector<byte> bytes, objectTracker* t){
    vector<object*> r;
    int i=0;
    while(i<bytes.size()){
        //TODO: Add Op id(change, delete)
        objectHeader h; autoFromBytes(&h, &i, bytes, t);
        object* a = t->findObject(h);
        if(a) a->fromBytes(bytes, &i, t);
        r.push_back(a);
    }
    return r;
}

#define forAlphabet(x) \
    x(A) x(B) x(C) x(D) x(E) x(F) x(G) x(H) x(I) x(J) \
    x(K) x(L) x(M) x(N) x(O) x(P) x(Q) x(R) x(S) x(T) \
    x(U) x(V) x(W) x(X) x(Y) x(Z)

#define for0Through9(x) x(0) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9)
//#define for0Through9_2(x, y) y(x(0)) x(1) x(2) x(3) x(4) x(5) x(6) x(7) x(8) x(9)
//#define numToKey(x, y) key##x
//#define forNumberKeys(x) x(for0Through9(numToKey))
#define forNumberKeys(x) x(Key0) x(Key1) x(Key2) x(Key3) x(Key4) x(Key5) x(Key6) x(Key7) x(Key8) x(Key9)
#define forNumberKeys2(x, t) x(t, Key0) x(t, Key1) x(t, Key2) x(t, Key3) x(t, Key4) x(t, Key5) x(t, Key6) x(t, Key7) x(t, Key8) x(t, Key9)

#define forMouseButtons(x) x(leftClick) x(rightClick)

#define forKeys(x) \
    forAlphabet(x) \
    x(forMouseButtons)

#define forAlphabet2(x, t) \
    x(t, A) x(t, B) x(t, C) x(t, D) x(t, E) x(t, F) x(t, G) x(t, H) x(t, I) x(t, J) \
    x(t, K) x(t, L) x(t, M) x(t, N) x(t, O) x(t, P) x(t, Q) x(t, R) x(t, S) x(t, T) \
    x(t, U) x(t, V) x(t, W) x(t, X) x(t, Y) x(t, Z)

#define forKeys2(x, T) \
    forAlphabet2(x, T) \
    x(T, leftClick) x(T, rightClick)


template<typename T>
void fromBytesPointer(T** a, int* i, vector<byte> bytes, objectTracker* t) {

    objectHeader h;
    autoFromBytes(&h, i, bytes, t);

    *a = dynamic_cast<T*>(t->findObject(h));
}

#define PP_APPLY_EACH_SEMI_IMPL(r, MACRO, x) MACRO(x);

//First arg is the macro, rest are inputs
#define macroForAll(MACRO, ...) \
  BOOST_PP_SEQ_FOR_EACH(PP_APPLY_EACH_SEMI_IMPL, MACRO, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))


//Examples of class definition with automatic serialization via to/from bytes
//#define vehivars(x) x(int, speed) x(long, velocity)
//classDefinition(myVehicle, vehivars, macroVoid)
//};
//
//#define tankvars(x) x(int, damage) x(long, armor) x(int, health)
//#define tankInheritances(x) x(myVehicle)
//classDefinition(Tank, tankvars, tankInheritances)
//};
//
//#define bobvars(x) x(int, h)
//classDefinition(Bob, bobvars, macroVoid)
//};
//#define clarvars(x) x(int, f)
//#define clarInh(x) x(Bob)
//classDefinition(clar, clarvars, clarInh)
//};

/*
int main() {
    vector<object*> obL;
    vector<object*> obL2;
    Bob* hi = new Bob();
    hi->h=1234;
    obL.push_back(hi);
    hi = new Bob();
    hi->h=2468;
    obL.push_back(hi);
    clar* he = new clar();
    he->f=9876;
    he->h=3456;
    obL.push_back(he);
    auto bytes = LtoBytes(obL);
    obL2 = bytesToL(bytes);
    for (int i = 0; i < obL2.size(); ++i) obL2[i]->print();
    for (auto p : obL2) delete p;
}*/