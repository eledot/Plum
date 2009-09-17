#include "../plum.h"


namespace Plum
{
	namespace ScriptLibrary
	{
		int exit(lua_State* L)
		{
			int argumentCount = lua_gettop(L);

			if(argumentCount >= 1)
			{
				Script::getInstance(L)->engine->quit(lua_tostring(L, 1));
			}
			else
			{
				Script::getInstance(L)->engine->quit();
			}
			
			return 0;
		}

		int refresh(lua_State* L)
		{
			Script::getInstance(L)->engine->refresh();
			return 0;
		}

		int setTitle(lua_State* L)
		{
			int argumentCount = lua_gettop(L);

			if(argumentCount >= 1)
			{
				Script::getInstance(L)->engine->setTitle(lua_tostring(L, 1));
			}
			
			return 0;
		}

		int rgb(lua_State* L)
		{
			int r = luaL_checkint(L, 1);
			int g = luaL_checkint(L, 2);
			int b = luaL_checkint(L, 3);
			int a = luaL_optint(L, 4, 255);
			lua_pushinteger(L, Plum::rgba(r, g, b, a).value);
			return 1;
		}

		int hsv(lua_State* L)
		{
			int h = luaL_checkint(L, 1);
			int s = luaL_checkint(L, 2);
			int v = luaL_checkint(L, 3);
			int a = luaL_optint(L, 4, 255);
			lua_pushinteger(L, Plum::hsv(h, s, v, a).value);
			return 1;
		}

		int loadConfig(lua_State* L)
		{
			Config cfg;
			cfg.init("plum.cfg", "config", L);
			lua_pushvalue(cfg.lua, -1);
			lua_xmove(cfg.lua, L, 1);
			return 1;
		}

		int hookInput(lua_State* L)
		{
			Script* script = Script::getInstance(L);

			// Check that first argument is an input object.
			// If so, happily grab the pointer.
			Input** inp = (Input**) luaL_checkudata(L, 1, "plum_input");

			// Get that function on the stack!
			if(!lua_isfunction(L, 2) && !lua_isnil(L, 2))
			{
				luaL_error(L, "Invalid argument #2 to plum.hookInput. Must be a valid function, or nil.");
				return 0;
			}

			// Nuke old callback reference (if any)
			{
				std::vector<Script::InputHook>& hooks = script->inputHooks;
				int ref = LUA_NOREF;
				for(u32 i = 0; i < hooks.size(); i++)
				{
					// Okay, convert our reference number input to userdata.
					Script::InputHook& hook = hooks[i];
					lua_rawgeti(L, LUA_REGISTRYINDEX, hook.inputRef);
					Input** p = (Input**) luaL_checkudata(L, -1, "plum_input");

					// If the passed argument is the same input object,
					// Then discard that old references.
					if(*p == *inp)
					{
						hooks.erase(hooks.begin() + i);

						luaL_unref(L, LUA_REGISTRYINDEX, hook.inputRef);
						luaL_unref(L, LUA_REGISTRYINDEX, hook.callbackRef);
						break;
					}
				}
			}
			// Store new callback reference stuff.
			// Unless nil, in which case we just remove old entries.
			// argh.
			if(!lua_isnil(L, 2))
			{
				Script::InputHook hook;

				lua_pushvalue(L, 1);
				hook.inputRef = luaL_ref(L, LUA_REGISTRYINDEX);

				lua_pushvalue(L, 2);
				hook.callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);

				script->inputHooks.push_back(hook);
			}
			return 0;
		}

		int unhookAllInput(lua_State* L)
		{
			Script* script = Script::getInstance(L);
			std::vector<Script::InputHook>& hooks = script->inputHooks;
			for(u32 i = 0; i < hooks.size(); i++)
			{
				Script::InputHook& hook = hooks[i];
				luaL_unref(L, LUA_REGISTRYINDEX, hook.inputRef);
				luaL_unref(L, LUA_REGISTRYINDEX, hook.callbackRef);
			}
			hooks.clear();
			return 0;
		}

		static void encodeImageData(lua_State* L, Image* img)
		{
			// Allocate buffer, four bytes representing texture dimension stuff.
			// (occupied width, height (occupied size), width, height (overall size))
			// And the rest for compressed data (might not use all of it).
			u8* buffer = new u8[sizeof(u32) * 4 + img->width * img->height * sizeof(Color)];
			// Pack the dimensions in 4 unsigned 32-bit integers.
			((u32*) buffer)[0] = (u32) img->occupiedWidth;
			((u32*) buffer)[1] = (u32) img->occupiedHeight;
			((u32*) buffer)[2] = (u32) img->width;
			((u32*) buffer)[3] = (u32) img->height;

			printf("Encoded thing has dimensions (%u, %u, %u, %u)\r\n", ((u32*) buffer)[0], ((u32*) buffer)[1], ((u32*) buffer)[2], ((u32*) buffer)[3]);
			// Compress.
			int compressedSize = Compression::compressData((u8*)(img->data),
				img->width * img->height * sizeof(Color),
				buffer + sizeof(u32) * 4,
				img->width * img->height * sizeof(Color)
			);
			// Encode.
			std::string encodedText = Base64::encode(std::string(buffer, buffer + sizeof(u32) * 4 + compressedSize));
			// Push encoded text.
			lua_pushstring(L, encodedText.c_str());
			// Destroy buffer.
			delete buffer;
		}

		int encodeImage(lua_State* L)
		{
			Wrapper<Image>* wrapper = PLUM_CHECK_DATA(L, 1, Image);
			encodeImageData(L, wrapper->data);

			return 1;
		}

		int encodeTexture(lua_State* L)
		{
			Wrapper<Texture>* texture = PLUM_CHECK_DATA(L, 1, Texture);
			encodeImageData(L, texture->data->getImage());

			return 1;
		}

		static Image* decodeImageData(const char* s)
		{
			std::string blob = Base64::decode(s);
			u8* data = (u8*) blob.data();

			// Unpack the dimensions from 4 unsigned 32-bit integers.
			u32 occupiedWidth = ((u32*) data)[0];
			u32 occupiedHeight = ((u32*) data)[1];
			u32 width = ((u32*) data)[2];
			u32 height = ((u32*) data)[3];
			// Allocate image.
			Image* img = new Image(width, height);
			img->occupiedWidth = occupiedWidth;
			img->occupiedHeight = occupiedHeight;
			img->restoreClipRegion();
			// Decompress.
			data += sizeof(u32) * 4;
			Compression::decompressData(data, blob.length(), (u8*)(img->data), img->width * img->height * sizeof(Color));
			// Done!
			return img;
		}

		int decodeImage(lua_State* L)
		{
			const char* s = luaL_checkstring(L, 1);
			Image* img = decodeImageData(s);
			// Push decoded image.
			PLUM_PUSH_DATA(L, Image, img, true);
			return 1;
		}

		int decodeTexture(lua_State* L)
		{
			const char* s = luaL_checkstring(L, 1);
			Image* img = decodeImageData(s);
			// Push decoded texture.
			PLUM_PUSH_DATA(L, Texture, new Texture(img), true);
			// Destroy temporary image.
			delete img;
			return 1;
		}

		int digestString(lua_State* L)
		{
			// Get the SHA1 digest of the string argument passed.
			std::string s = luaL_checkstring(L, 1);
			std::string result;
			SHA1::digestString(s, result);
			lua_pushstring(L, Base64::encode(result).c_str());
			return 1;
		}

		int digestFile(lua_State* L)
		{
			// Get the SHA1 digest of the file named by the string argument passed.
			std::string s = luaL_checkstring(L, 1);
			std::string result;
			SHA1::digestFile(s, result);
			lua_pushstring(L, Base64::encode(result).c_str());
			return 1;
		}

		void openLibrary(lua_State* L)
		{
			PLUM_BIND_FUNC_BEGIN()
			PLUM_BIND_FUNC(exit)
			PLUM_BIND_FUNC(refresh)
			PLUM_BIND_FUNC(setTitle)
			PLUM_BIND_FUNC(loadConfig)
			PLUM_BIND_FUNC(hookInput)
			PLUM_BIND_FUNC(unhookAllInput)
			PLUM_BIND_FUNC_END()

			// Create the 'data' table.
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setfield(L, -3, "data");

			lua_pushcfunction(L, encodeImage);
			lua_setfield(L, -2, "encodeImage");
			lua_pushcfunction(L, decodeImage);
			lua_setfield(L, -2, "decodeImage");
			lua_pushcfunction(L, encodeTexture);
			lua_setfield(L, -2, "encodeTexture");
			lua_pushcfunction(L, decodeTexture);
			lua_setfield(L, -2, "decodeTexture");
			lua_pushcfunction(L, digestString);
			lua_setfield(L, -2, "digestString");
			lua_pushcfunction(L, digestFile);
			lua_setfield(L, -2, "digestFile");

			// Done with 'data' now.
			lua_pop(L, 1);

			// Create the 'color' table.
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setfield(L, -3, "color");

			lua_pushinteger(L, Color::White);
			lua_setfield(L, -2, "White");
			lua_pushinteger(L, Color::Red);
			lua_setfield(L, -2, "Red");
			lua_pushinteger(L, Color::Green);
			lua_setfield(L, -2, "Green");
			lua_pushinteger(L, Color::Blue);
			lua_setfield(L, -2, "Blue");
			lua_pushinteger(L, Color::Magenta);
			lua_setfield(L, -2, "Magenta");
			lua_pushinteger(L, Color::Cyan);
			lua_setfield(L, -2, "Cyan");
			lua_pushinteger(L, Color::Yellow);
			lua_setfield(L, -2, "Yellow");
			lua_pushinteger(L, Color::Black);
			lua_setfield(L, -2, "Black");

			lua_pushcfunction(L, rgb);
			lua_setfield(L, -2, "rgb");

			lua_pushcfunction(L, hsv);
			lua_setfield(L, -2, "hsv");

			// Done with 'color' now.
			lua_pop(L, 1);

			// Create the 'blend' table.
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setfield(L, -3, "blend");

			lua_pushinteger(L, BlendOpaque);
			lua_setfield(L, -2, "Opaque");
			lua_pushinteger(L, BlendMerge);
			lua_setfield(L, -2, "Merge");
			lua_pushinteger(L, BlendMerge);
			lua_setfield(L, -2, "Alpha");
			lua_pushinteger(L, BlendPreserve);
			lua_setfield(L, -2, "Preserve");
			lua_pushinteger(L, BlendAdd);
			lua_setfield(L, -2, "Add");
			lua_pushinteger(L, BlendSubtract);
			lua_setfield(L, -2, "Subtract");

			// Done with 'blend' now.
			lua_pop(L, 1);

			// Pop the library.
			lua_pop(L, 1);


			ImageObject::openLibrary(L);
			TextureObject::openLibrary(L);
			SpritesheetObject::openLibrary(L);
			TilemapObject::openLibrary(L);
		}
	}
}