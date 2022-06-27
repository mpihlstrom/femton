#pragma once

extern unsigned int gid;

struct Entity {
    Entity() : id(++gid) {}
protected:
    Entity(uint32_t id) : id(id) {}
public:
    mutable uint32_t id;
};

struct Class : Entity {
    enum Name {
        Class_void = 0x00,
        Class_trigon = 0x01,
        Class_node = 0x02,
    };
    Class() : name_(Class_void) {}
    virtual Name name() const = 0;
protected:
    Class(uint32_t id) : Entity(id) {}
private:
    Name name_;
};

struct Void : Class {
    Void() : Class(0) {}
    virtual Name name() const { return Class_void; }
};

struct Eumetry {
    virtual ~Eumetry() {}
    virtual bool invalid() const = 0;
};
