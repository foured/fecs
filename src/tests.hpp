#include <chrono>
#include <unordered_map>
#include <vector>
#include <iostream>

struct a{
    virtual ~a() = default;
 };

template<size_t idx>
struct b : public a { };

template<size_t Begin, size_t End>
void generate_instances_chunk(std::vector<a*>& vec) {
    []<size_t... Is>(std::vector<a*>& vec, std::index_sequence<Is...>) {
        (..., vec.push_back(new b<Begin + Is>()));
    }(vec, std::make_index_sequence<End - Begin>{});
}

template<size_t Begin, size_t End>
void generate_map_chunk(std::unordered_map<size_t, a*>& map) {
    []<size_t... Is>(std::unordered_map<size_t, a*>& map, std::index_sequence<Is...>) {
        (..., map.emplace(Begin + Is, new b<Begin + Is>()));
    }(map, std::make_index_sequence<End - Begin>{});
}

std::chrono::time_point<std::chrono::steady_clock> start_timepoint;

void start_timer(){
    start_timepoint = std::chrono::high_resolution_clock::now();
}

void end_timer(){
    auto end_timepoint = std::chrono::high_resolution_clock::now();
	int64_t start = std::chrono::time_point_cast<std::chrono::nanoseconds>(start_timepoint)
		.time_since_epoch().count();
	int64_t end = std::chrono::time_point_cast<std::chrono::nanoseconds>(end_timepoint)
		.time_since_epoch().count();

    std::cout << (end - start) << "  us\n";
}

size_t delta_time(std::chrono::time_point<std::chrono::steady_clock>& s,
                std::chrono::time_point<std::chrono::steady_clock>& e){
	int64_t start = std::chrono::time_point_cast<std::chrono::nanoseconds>(s)
		.time_since_epoch().count();
	int64_t end = std::chrono::time_point_cast<std::chrono::nanoseconds>(e)
		.time_since_epoch().count();

    return end - start;
}

void test2(size_t iterations){
    std::unordered_map<size_t, a*> map;

    for(size_t i = 0; i < 10'000'000; i++){
        map.emplace(i, new b<10>());
    }

    a* dest = nullptr;
    size_t key = 9'000;

    std::cout << "Test 2 (try emplace)" << iterations << " iterations:\n"; 

    auto s = std::chrono::high_resolution_clock::now();

    for(size_t i = 0; i < iterations; i++) {
        auto[iter, inserted] = map.try_emplace(key, new b<100>());
        dest = static_cast<a*>(iter->second);
    }

    auto e = std::chrono::high_resolution_clock::now();

    size_t dt = delta_time(s, e);
    std::cout << dt << "  us avg:   " << dt / iterations << "   us\n";

    std::cout << "Test 2 (contains + at)" << iterations << " iterations:\n"; 

    s = std::chrono::high_resolution_clock::now();

    for(size_t i = 0; i < iterations; i++) {
        if(map.contains(key)){
            dest = static_cast<a*>(map[key]);
        }
        else{
            auto[iter, inserted] = map.emplace(key, new b<100>());
            dest = static_cast<a*>(iter->second);
        }
    }

    e = std::chrono::high_resolution_clock::now();

    dt = delta_time(s, e);
    std::cout << dt << "  us avg:   " << dt / iterations << "   us\n";
}