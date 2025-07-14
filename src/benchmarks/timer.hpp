#pragma once

#include <chrono>
#include <cstddef>
#include <iostream>

class timer{
public:
    timer(const char* name) 
        : _name(name) {
        _start = std::chrono::high_resolution_clock::now();
    }

    ~timer(){
        end();
    }

    size_t end(){
        if(!_ended){
            _ended = true;
            auto end = std::chrono::high_resolution_clock::now();

            size_t s = std::chrono::time_point_cast<std::chrono::nanoseconds>(_start).time_since_epoch().count();
            size_t e = std::chrono::time_point_cast<std::chrono::nanoseconds>(end).time_since_epoch().count();
            size_t d = e - s;
            std::cout << _name << ": " << d << " ns\n";
            return d;
        }
        return 0;
    }

private:
    std::chrono::high_resolution_clock::time_point _start;
    const char * _name;
    bool _ended = false;

};