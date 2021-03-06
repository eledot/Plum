
#include "../core/transform.h"
#include "script.h"

namespace plum
{
    namespace script
    {
        template<> const char* meta<Point>()
        {
            return "plum.Point";
        }
    }

    namespace
    {
        typedef Point Self;
        int create(lua_State* L)
        {
            auto x = script::get<double>(L, 1);
            auto y = script::get<double>(L, 2);
            script::push(L, new Point(x, y), LUA_NOREF);
            return 1;
        }

        int gc(lua_State* L)
        {
            return script::wrapped<Self>(L, 1)->gc(L);
        }

        int index(lua_State* L)
        {
            return script::wrapped<Self>(L, 1)->index(L);
        }

        int newindex(lua_State* L)
        {
            return script::wrapped<Self>(L, 1)->newindex(L);
        }

        int tostring(lua_State* L)
        {
            return script::wrapped<Self>(L, 1)->tostring(L);
        }

        int len(lua_State* L)
        {
            auto p = script::ptr<Point>(L, 1);
            script::push(L, 2);
            return 1;
        }

        int get_x(lua_State* L)
        {
            auto p = script::ptr<Point>(L, 1);
            script::push(L, p->x);
            return 1;
        }

        int get_y(lua_State* L)
        {
            auto p = script::ptr<Point>(L, 1);
            script::push(L, p->y);
            return 1;
        }

        int set_x(lua_State* L)
        {
            auto p = script::ptr<Point>(L, 1);
            auto val = script::get<double>(L, 2);
            p->x = val;
            return 0;
        }

        int set_y(lua_State* L)
        {
            auto p = script::ptr<Point>(L, 1);
            auto val = script::get<double>(L, 2);
            p->y = val;
            return 0;
        }

        int setPoint(lua_State* L)
        {
            auto p = script::ptr<Point>(L, 1);
            auto x = script::get<double>(L, 2);
            auto y = script::get<double>(L, 3);
            p->x = x;
            p->y = y;
            return 0;
        }
    }

    namespace script
    {
        void initPointObject(lua_State* L)
        {
            luaL_newmetatable(L, meta<Self>());
            // Duplicate the metatable on the stack.
            lua_pushvalue(L, -1);
            // metatable.__index = metatable
            lua_setfield(L, -2, "__index");

            // Put the members into the metatable.
            const luaL_Reg functions[] = {
                {"__gc", gc},
                {"__len", len},
                {"__index", index},
                {"__newindex", newindex},
                {"__tostring", tostring},
                {"get_x", get_x},
                {"get_y", get_y},
                {"set_x", set_x},
                {"set_y", set_y},
                {"setPoint", setPoint},
                {nullptr, nullptr},
            };
            luaL_setfuncs(L, functions, 0);

            lua_pop(L, 1);
                
            // Push plum namespace.
            lua_getglobal(L, "plum");

            // plum.Point = <function create>
            script::push(L, "Point");
            lua_pushcfunction(L, create);
            lua_settable(L, -3);

            // Pop plum namespace.
            lua_pop(L, 1);
        }
    }
}