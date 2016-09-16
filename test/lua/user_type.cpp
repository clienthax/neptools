#include "lua/user_type.hpp"
#include "lua/dynamic_object.hpp"
#include "lua/function_call.hpp"
#include <catch.hpp>

#define FT(x) decltype(&x), &x

using namespace Neptools;
using namespace Neptools::Lua;

static int global;

struct Foo final : public DynamicObject
{
    int local = 0;
    void DoIt(int x) { local = x; }

    Foo() = default; // no ctor generated otherwise.. (bug?)
    ~Foo() { global += 13; }

    NEPTOOLS_DYNAMIC_OBJECT;
};

namespace Bar
{
namespace Baz
{
struct Asdfgh final : public DynamicObject
{
    Asdfgh() = default;
    NEPTOOLS_DYNAMIC_OBJECT;
};
}
}

struct Baz : public DynamicObject
{
    Baz() = default;
    void SetGlobal(int val) { global = val; }
    int GetRandom() { return 4; }

    NEPTOOLS_DYNAMIC_OBJECT;
};

TEST_CASE("shared check memory", "[lua]")
{
    {
        State vm;

        global = 0;
        const char* str = nullptr;
        SECTION("normal") str = "local x = foo.new()";
        SECTION("short-cut") str = "local x = foo()";
        SECTION("explicit call") str = "local x = foo():__gc()";
        if (!str) return; // khrr, clang...

        if (luaL_dostring(vm, str))
            FAIL(lua_tostring(vm, -1));
    }
    CHECK(global == 13);
}

TEST_CASE("resurrect shared object", "[lua]")
{
    {
        State vm;

        global = 0;
        auto ptr = MakeShared<Foo>();
        vm.Push(ptr);
        lua_setglobal(vm, "fooobj");

        if (luaL_dostring(vm, "fooobj:__gc() assert(getmetatable(fooobj) == nil)"))
            FAIL(lua_tostring(vm, -1));
        REQUIRE(global == 0);

        vm.Push(ptr);
        lua_setglobal(vm, "fooobj");
        if (luaL_dostring(vm, "fooobj:do_it(123)"))
            FAIL(lua_tostring(vm, -1));
        CHECK(global == 0);
        CHECK(ptr->local == 123);
    }
    CHECK(global == 13);
}

TEST_CASE("member function without helpers", "[lua]")
{
    State vm;

    REQUIRE(luaL_loadstring(vm, "local x = foo() x:do_it(77) return x") == 0);
    lua_call(vm, 0, 1);
    CHECK(vm.Get<Foo>().local == 77);
}

TEST_CASE("member function with helpers", "[lua]")
{
    State vm;

    const char* str = nullptr;
    int val;
    SECTION("normal call") { str = "baz():set_global(42)"; val = 42; }
    SECTION("sugar") { str = "baz().global = 43"; val = 43; }
    SECTION("read") { str = "local x = baz() x.global = x.random"; val = 4; }
    if (!str) return; // khrr, clang...

    if (luaL_dostring(vm, str))
        FAIL(lua_tostring(vm, -1));
    CHECK(global == val);
}

TEST_CASE("dotted type name", "[lua]")
{
    State vm;

    if (luaL_dostring(vm, "local x = bar.baz.asdfgh()"))
        FAIL(lua_tostring(vm, -1));
}

#include "user_type.binding.hpp"
