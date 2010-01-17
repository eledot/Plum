#include "../plum.h"

namespace Plum
{
	static Tileset** checkValidTilesetObject(lua_State* L, int index)
	{
		return (Tileset**) luaL_checkudata(L, index, "plum_tileset");
	}

	static int tilesetNew(lua_State* L)
	{
		const char* filename = luaL_checkstring(L, 1);

		Tileset** ts = (Tileset**) lua_newuserdata(L, sizeof(Tileset*));
		luaL_getmetatable(L, "plum_tileset");
		lua_setmetatable(L, -2);

		*ts = new Tileset(filename, L);
		
		return 1;
	}

	static int tilesetGC(lua_State* L)
	{
		Tileset** ts = checkValidTilesetObject(L, 1);
		delete *ts;

		return 0;
	}

	SCRIPT_OBJ_GETTER(tilesetGetField, Tileset**, "plum_tileset")
	SCRIPT_OBJ_SETTER(tilesetSetField, Tileset**, "plum_tileset")

	static int tilesetToString(lua_State* L)
	{
		checkValidTilesetObject(L, 1);
		lua_pushstring(L, "(plum.Tileset object)");
		return 1;
	}

	static int tilesetGetTiles(lua_State* L)
	{
		Tileset** ts = checkValidTilesetObject(L, 1); 

		// Push reference to this tileset, so the tileset stays around
		// as long as it's required for this image.
		lua_pushvalue(L, 1);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		PLUM_PUSH_DATA(L, Image, (*ts)->tiles, ref);
		return 1;
	}

	static int tilesetReplaceTiles(lua_State* L)
	{
		Tileset** ts = checkValidTilesetObject(L, 1); 
		ScriptLibrary::Wrapper<Image>* img = NULL;

		(*ts)->externalTileFile = "";
		if(lua_isstring(L, 1))
		{
			const char* filename = lua_tostring(L, 1);
			delete (*ts)->tiles;
			(*ts)->externalTileFile = filename;
			(*ts)->tiles = new Image(filename);
		}
		else
		{
			img = PLUM_CHECK_DATA(L, 2, Image);
			delete (*ts)->tiles;
			(*ts)->tiles = new Image(img->data->getCanvas());
		}

		return 0;
	}

	static int tilesetGetObs(lua_State* L)
	{
		Tileset** ts = checkValidTilesetObject(L, 1);

		// Push reference to this tileset, so the tileset stays around
		// as long as it's required for this image.
		lua_pushvalue(L, 1);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		PLUM_PUSH_DATA(L, Image, (*ts)->obs, ref);
		return 1;
	}

	static int tilesetReplaceObs(lua_State* L)
	{
		Tileset** ts = checkValidTilesetObject(L, 1); 
		ScriptLibrary::Wrapper<Image>* img = NULL;

		(*ts)->externalObsFile = "";
		if(lua_isstring(L, 1))
		{
			const char* filename = lua_tostring(L, 1);
			delete (*ts)->obs;
			(*ts)->externalObsFile = filename;
			(*ts)->obs = new Image(filename);
		}
		else
		{
			img = PLUM_CHECK_DATA(L, 2, Image);
			delete (*ts)->obs;
			(*ts)->obs = new Image(img->data->getCanvas());
		}

		return 0;
	}

	static int tilesetGetConfig(lua_State* L)
	{
		Tileset** ts = checkValidTilesetObject(L, 1); 
		lua_pushvalue((*ts)->config.lua, -1);
		lua_xmove((*ts)->config.lua, L, 1);
		return 1;
	}

	static int tilesetSave(lua_State* L)
	{
		Tileset** ts = checkValidTilesetObject(L, 1); 
		const char* filename = luaL_checkstring(L, 2);

		(*ts)->save(filename);

		return 0;
	}

	static const luaL_Reg tilesetMembers[] = {
		{ "__index", tilesetGetField },
		{ "__newindex",	tilesetSetField },
		{ "__tostring",	tilesetToString },
		{ "__gc", tilesetGC },
		{ "replaceTiles", tilesetReplaceTiles },
		{ "replaceObs", tilesetReplaceObs },
		{ "save", tilesetSave },
		{ "gettiles", tilesetGetTiles },
		{ "getobs", tilesetGetObs },
		{ "getconfig", tilesetGetConfig },
		{ NULL, NULL }
	};

	void Script::initTilesetClass(lua_State* L)
	{
		luaL_newmetatable(L, "plum_tileset");
		// Duplicate the metatable on the stack.
		lua_pushvalue(L, -1);
		// metatable.__index = metatable
		lua_setfield(L, -2, "__index");
		// Put the members into the metatable.
		luaL_register(L, NULL, tilesetMembers);
		lua_pop(L, 1);

		// Push plum namespace.
		lua_getglobal(L, "plum");

		// plum.tileset = <function tilesetNew>
		lua_pushstring(L, "Tileset");
		lua_pushcfunction(L, tilesetNew);
		lua_settable(L, -3);

		// Pop plum namespace.
		lua_pop(L, 1);
	}
}