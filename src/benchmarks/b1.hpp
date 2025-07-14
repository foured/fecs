#include "../registory.h"
#include "group.h"
#include "timer.hpp"
#include <type_traits>
#include <utility>
#include <vector>

struct vec3{
    vec3(float X = 0, float Y = 0, float Z = 0) 
        : x(X), y(Y), z(Z) { }

    float x = 0;
    float y = 0;
    float z = 0;

    vec3 operator+(const vec3& other){
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    vec3& operator+=(const vec3& other){
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }


};

struct transform{
    transform(const vec3& position) : pos(position) {}

    vec3 pos;
};

struct rigidbody{
    rigidbody(const vec3& velosity) : vel(velosity) {}

    vec3 vel;
};

void test_fecs_groups(size_t iterations){
    fecs::registory registory;
    registory.create_group<transform, rigidbody>();
    for(size_t i = 0; i < iterations; i++){
        registory.add_component<transform>(i, vec3(i*2, i*2, i*2));
        registory.add_component<rigidbody>(i, vec3(i, i, i));
    }

    auto g = registory.group<transform, rigidbody>();

    timer ft("g");

    g->for_each([](transform& t, rigidbody& r){
        t.pos += r.vel;
    });
    size_t dt = ft.end();
    std::cout << "avg: " << dt / float(iterations) << '\n';
}

void test_fecs_views(size_t iterations){
    fecs::registory registory;
    for(size_t i = 0; i < iterations; i++){
        registory.add_component<transform>(i, vec3(i*2, i*2, i*2));
        registory.add_component<rigidbody>(i, vec3(i, i, i));
    }

    auto v = registory.view<transform, rigidbody>();

    timer ft("v");

    v.for_each([](transform& t, rigidbody& r){
        t.pos += r.vel;
    });
    size_t dt = ft.end();
    std::cout << "avg: " << dt / float(iterations) << '\n';
}

void test_fecs_runner(size_t iterations){
    fecs::registory registory;
    for(size_t i = 0; i < iterations; i++){
        registory.add_component<transform>(i, vec3(i*2, i*2, i*2));
        registory.add_component<rigidbody>(i, vec3(i, i, i));
    }

    auto r = registory.runner<rigidbody>();

    timer ft("r");

    r.for_each([](rigidbody& r){
        r.vel += vec3(1, 4.443, 0.123);
    });

    size_t dt = ft.end();
    std::cout << "avg: " << dt / float(iterations) << '\n';
}

void test_fecs_direct(size_t iterations){
    fecs::registory registory;
    for(size_t i = 0; i < iterations; i++){
        registory.add_component<transform>(i, vec3(i*2, i*2, i*2));
        registory.add_component<rigidbody>(i, vec3(i, i, i));
    }

    timer ft("d");

    registory.direct_for_each<rigidbody>([](rigidbody& r){
        r.vel += vec3(1, 4.443, 0.123);
    });
    size_t dt = ft.end();
    std::cout << "avg: " << dt / float(iterations) << '\n';
}

class instance;

struct component{
    component(instance* i) : inst(i) { }

    virtual ~component() = default;
    virtual void update() = 0;
    instance* inst;
};

class instance{
public:
    instance(vec3 pos) : transf(pos) {}

    template<typename T, typename... Args>
    requires std::is_base_of_v<component, T> && std::is_constructible_v<T, instance*, Args&&...>
    void add_component(Args&&... args){
        _components.emplace_back(new T(this, std::forward<Args>(args)...));
    }

    void update(){
        for(component* c : _components){
            c->update();
        }
    }

    transform transf;

private:
    std::vector<component*> _components;
};

struct rigidbody_component : public component{
    rigidbody_component(instance* i, vec3 velocity) : component(i), vel(velocity) {}

    void update() override{
        inst->transf.pos += vel;
    }

    vec3 vel;
};

void test_usual(size_t iterations){
    std::vector<instance> instances;
    instances.reserve(iterations);
    for(size_t i = 0; i < iterations; i++){
        instances.emplace_back(vec3(i, i, i));
        instances.back().add_component<rigidbody_component>(vec3(i*2, i*2, i*2));
    }

    timer ut("u");
    for(auto&  ins : instances){
        ins.update();
    }
    size_t dt = ut.end();
    std::cout << "avg: " << dt / float(iterations) << '\n';
}

void b1(size_t iterations){
    test_usual(iterations);
    test_fecs_groups(iterations);
    test_fecs_views(iterations);
    test_fecs_runner(iterations);
    test_fecs_direct(iterations);
}