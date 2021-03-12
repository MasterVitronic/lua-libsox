/*

  lua-libsox.c - Lua bindings to libsox

  Copyright (c) 2021, Díaz Devera Víctor <mastervitronic@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/***
 * Lua bindings to libsox
 *
 * This documentation is partial, and doesn't cover all functionality yet.
 * @module libsox
 * @author Díaz Devera Víctor (Máster Vitronic) <mastervitronic@gmail.com>
 */
 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "compat.h"
#include <sox.h>

#define SOX "SOX"

typedef struct {
	lua_State *L ;
} ctx_t;


static int l_new (lua_State *L) {
	ctx_t *ctx = (ctx_t *)lua_newuserdata(L, sizeof(ctx_t));

	ctx->L = L;
	luaL_getmetatable(L, SOX);
	lua_setmetatable(L, -2);
	return 1;
}


static ctx_t * check_config (lua_State *L) {
    return (ctx_t *) luaL_checkudata(L, 1, SOX);
}


/***
 * libSoX library version.
 * @function version
 * @treturn string version string "major.minor.revision"
 * @see sox_version
 */
static int l_sox_version( lua_State * L ) {
	const char * version = sox_version();
	lua_pushstring(L, version);
	return 1;
}


/***
 * Initialize effects library.
 * @function sox_init
 * @treturn boolean true if successful.
 * @see sox_init
 */
static int l_sox_init( lua_State * L ) {
	if (sox_init() != SOX_SUCCESS) {
		lua_pushboolean(L, 0);			
	} else {
		lua_pushboolean(L, 1);
	}

	return 1;
}


/***
 * Close effects library and unload format handler plugins.
 * @function sox_quit
 * @treturn boolean true if successful.
 * @see sox_quit
 */
static int l_sox_quit( lua_State * L ) {
	if (sox_quit() != SOX_SUCCESS) {
		lua_pushboolean(L, 0);			
	} else {
		lua_pushboolean(L, 1);
	}

	/* remove all methods operating on config */
	lua_newtable(L);
	lua_setmetatable(L, -2);
	return 1;
}

/***
 * Set the buffer size
 * @function buffer
 * @tparam integer size  size of buffer
 * @treturn userdata Returned buffer handle .
 */
static int l_buffer(lua_State *L) {
	size_t buf_sz;
	if ( lua_isnumber(L, 2) ){
		buf_sz = luaL_checknumber(L, 2);
	} else {
		buf_sz = 8192;
	}
	sox_sample_t* buffer[buf_sz+sizeof(buf_sz)];
	lua_pushlightuserdata(L, buffer);

	return 1;
}

/***
 * Converts SoX native sample to a 32-bit float.
 * @function sample_to_float32
 * @tparam userdata buf Input sample to be converted.
 * @tparam integer  index index in the buffer array
 * @tparam integer  clips Number to increment if input sample is too large.
 */
static int l_sample_to_float32(lua_State *L) {
	sox_sample_t* buf = lua_touserdata(L, 2);
	int index         = luaL_checkinteger(L, 3);
	size_t clips      = luaL_checknumber(L, 4);
	SOX_SAMPLE_LOCALS;
	float sample = SOX_SAMPLE_TO_FLOAT_32BIT(buf[index],clips);
	lua_pushnumber(L, sample);

	return 1;
}


/***
 * Converts SoX native sample to a 64-bit float.
 * @function sample_to_float64
 * @tparam userdata buf Input sample to be converted.
 * @tparam integer  index index in the buffer array
 */
static int l_sample_to_float64(lua_State *L) {
	sox_sample_t* buf = lua_touserdata(L, 2);
	int index         = luaL_checkinteger(L, 3);
	SOX_SAMPLE_LOCALS;
	double sample = SOX_SAMPLE_TO_FLOAT_64BIT(buf[index],0);

	lua_pushnumber(L, sample);

	return 1;
}


/***
 * Return audio levels of the current buffer.
 * @function get_levels
 * @tparam userdata buf Input sample to be analize.
 * @tparam integer block_size the size of blocks to be read.
 * @treturn number level of right channel.
 * @treturn number level of left channel.
 */
static int l_get_levels(lua_State *L) {
	sox_sample_t* buf = lua_touserdata(L, 2);
	size_t block_size = luaL_checkinteger(L, 3);
	double left = 0, right = 0;
	size_t i; SOX_SAMPLE_LOCALS;

	for (i = 0; i < block_size; ++i) {
		double sample = SOX_SAMPLE_TO_FLOAT_64BIT(buf[i],0);
		if (i & 1) {
			right = fmax(right, fabs(sample));
		} else {
			left = fmax(left, fabs(sample));
		}
	}

	lua_pushnumber(L, ((1 - right) * 35 + .5));
	lua_pushnumber(L, ((1 - left) * 35 + .5));

	return 2;
}

/***
 * Return information already known about audio stream, or NULL if none.
 * @function signal
 * @tparam userdata ft the current handler
 */
static int l_signal(lua_State *L) {
	sox_format_t* ft = lua_touserdata(L, 2);

	/*signalinfo table*/
	lua_newtable(L);
	lua_pushstring(L, "channels");
		lua_pushinteger(L, ft->signal.channels);
	lua_settable(L, -3);
	lua_pushstring(L, "length");
		lua_pushinteger(L, ft->signal.length);
	lua_settable(L, -3);
	//lua_pushstring(L, "mult");
		//lua_pushnumber(L, ft->signal.mult); /*@FIXE type double */
	//lua_settable(L, -3);
	lua_pushstring(L, "precision");
		lua_pushinteger(L, ft->signal.precision);
	lua_settable(L, -3);
	lua_pushstring(L, "rate");
		lua_pushinteger(L, ft->signal.rate);
	lua_settable(L, -3);

	return 1;
}


/***
 * Return information already known about sample encoding
 * @function encoding
 * @tparam userdata ft the current handler
 */
static int l_encoding(lua_State *L) {
	sox_format_t* ft = lua_touserdata(L, 2);

	/*encoding table*/
	lua_newtable(L);
	lua_pushstring(L, "bits_per_sample");
		lua_pushinteger(L, ft->encoding.bits_per_sample);
	lua_settable(L, -3);
	lua_pushstring(L, "compression");
		lua_pushinteger(L, ft->encoding.compression);
	lua_settable(L, -3);
	lua_pushstring(L, "encoding");
		lua_pushinteger(L, ft->encoding.encoding);
	lua_settable(L, -3);
	lua_pushstring(L, "opposite_endian");
		lua_pushboolean(L, ft->encoding.opposite_endian);
	lua_settable(L, -3);

	return 1;
}

/***
 * Initializes an effects chain. 
 * @function sox_create_effects_chain
 * @tparam userdata in_enc Input encoding.
 * @tparam userdata out_enc Output encoding.
 * @treturn userdata handle Returned must be closed with sox:delete_effects_chain().
 * @see sox_create_effects_chain
 */
static int l_sox_create_effects_chain( lua_State * L ) {
	sox_format_t* in_enc  = lua_touserdata(L, 2);
	sox_format_t* out_enc = lua_touserdata(L, 3);
	sox_effects_chain_t * handle = sox_create_effects_chain(
		&in_enc->encoding, &out_enc->encoding
	);

	if ( handle == NULL ){
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, handle);
	}

	return 1;
}


/***
 * Creates an effect using the given handler.
 * @function sox_create_effect
 * @tparam userdata eh Handler to use for effect.
 * @see sox_create_effect
 */
static int l_sox_create_effect( lua_State * L ) {
	sox_effect_handler_t* eh  = lua_touserdata(L, 2);
	sox_effect_t* effect = sox_create_effect(eh);

	if ( effect == NULL ){
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, effect);
	}

	return 1;
}


/***
 * Finds the effect handler with the given name.
 * @function sox_find_effect
 * @tparam string name Name of effect to find.
 * @see sox_find_effect
 */
static int l_sox_find_effect( lua_State * L ) {
	char const* name  = luaL_checkstring(L, 2);
	const sox_effect_handler_t* effect = sox_find_effect(name);

	if ( effect == NULL ){
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, (void*)effect);
	}

	return 1;
}


/***
 * Applies the command-line options to the effect.
 * @function sox_effect_options
 * @tparam userdata effp Effect pointer on which to set options.
 * @tparam integer argc Number of arguments in argv.
 * @tparam userdata|string argv Array of command-line options.
 * @see sox_effect_options
 */
static int l_sox_effect_options( lua_State * L ) {
	sox_effect_t *  effp  = lua_touserdata(L, 2);
	int argc              = luaL_checkinteger(L, 3);
	char * args[10];
	if ( lua_islightuserdata(L, 4) ){
		sox_effect_t *  argv  = lua_touserdata(L, 4);
		args[0] = (char *)argv;
	} else if ( lua_isstring(L, 4) ) {
		char const  * argv  = luaL_checkstring(L, 4);
		args[0] = (char *)argv;
	}

	int rc = sox_effect_options(effp, argc, args);

	lua_pushinteger(L, rc);

	return 1;
}


/***
 * Adds an effect to the effects chain.
 * @function sox_add_effect
 * @tparam userdata chain Effects chain to which effect should be added .
 * @tparam userdata effp Effect to be added.
 * @tparam userdata in Input format.
 * @tparam userdata out Output format.
 * @treturn boolean returns true if successful.
 * @see sox_add_effect
 */
static int l_sox_add_effect( lua_State * L ) {
	sox_effects_chain_t * chain  = lua_touserdata(L, 2);
	sox_effect_t *        effp   = lua_touserdata(L, 3);
	sox_format_t*         in     = lua_touserdata(L, 4);
	sox_format_t*         out    = lua_touserdata(L, 5);
	int success = sox_add_effect(
		chain, effp, &in->signal, &out->signal
	);
	
	if (success != SOX_SUCCESS) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, 1);
	}	
	return 1;
}


/***
 * Runs the effects chain.
 * @function sox_flow_effects
 * @tparam userdata chain Effects chain to run.
 * @see sox_flow_effects
 * @treturn boolean returns true if successful.
 * @todo WIP
 */
 //* @tparam Callback for monitoring flow progress.
 //* @tparam Data to pass into callback.
static int l_sox_flow_effects( lua_State * L ) {
	sox_effects_chain_t * chain  = lua_touserdata(L, 2);
	//sox_flow_effects_callback callback   = lua_touserdata(L, 3);
	//void * 	client_data    = lua_touserdata(L, 4);
	//int success = sox_flow_effects(chain, callback, client_data);
	int success = sox_flow_effects(chain, NULL, NULL);
	
	if (success != SOX_SUCCESS) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, 1);
	}

	return 1;
}


/***
 * Closes an effects chain.
 * @function sox_delete_effects_chain
 * @tparam userdata ecp Effects chain pointer.
 * @see sox_delete_effects_chain
 */
static int l_sox_delete_effects_chain( lua_State * L ) {
	sox_effects_chain_t * ecp  = lua_touserdata(L, 2);
	sox_delete_effects_chain(ecp);

	return 0;
}


/***
 * Opens a decoding session for a file.
 * @function sox_open_read
 * @tparam string path Path to file to be opened (required).
 * @treturn userdata handle Returned handle must be closed with sox:close(handle).
 * @see sox_open_read
 */
static int l_sox_open_read( lua_State * L ) {
	char const  * path  = luaL_checkstring(L, 2);
	sox_format_t* input = sox_open_read(path, NULL, NULL, NULL);

	if ( input == NULL ){
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, input);
	}

	return 1;
}


/***
 * Opens a decoding session for a memory buffer.
 * @function sox_open_mem_read
 * @tparam string buf Pointer to audio data buffer (required).
 * @tparam integer buf_sz Number of bytes to read from audio data buffer.
 * @tparam array signal Information already known about audio stream, or NULL if none.
 * @tparam string filetype Information already known about sample encoding, or NULL if none.
 * @treturn userdata handle Returned handle must be closed with sox:close(handle).
 * @see sox_open_mem_read
 * @todo WIP
 */
static int l_sox_open_mem_read( lua_State * L ) {
	const char * buf    = luaL_checkstring(L, 2);
	size_t buf_sz = luaL_checkinteger(L, 3);

	sox_signalinfo_t signal = {};
	lua_getfield(L, 4, "rate");
		signal.rate = luaL_checkinteger(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 4, "channels");
		signal.channels = luaL_checkinteger(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 4, "precision");
		signal.precision = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	const char * filetype = luaL_checkstring(L, 5);
	sox_format_t* input   = sox_open_mem_read(
		(void *)buf, buf_sz, &signal, NULL, NULL
	);

	if ( input == NULL ) {
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, input);
	}

	return 1;
}


/***
 * Opens an encoding session for a memstream buffer.
 * @function sox_open_memstream_write
 * @tparam string buf_ptr Receives pointer to audio data buffer that receives data (required).
 * @tparam integer buf_sz_ptr Receives size of data written to audio data buffer (required).
 * @tparam array signal Information about desired audio stream (required).
 * @treturn userdata handle Returned handle must be closed with sox:close(handle).
 * @see sox_open_memstream_write
 * @todo WIP
 */
 //* @tparam ? Information about desired sample encoding, or NULL to use defaults.
 //* @tparam ? Previously-determined file type, or NULL to auto-detect.
 //* @tparam ? Out-of-band data to add to file, or NULL if none.
static int l_sox_open_memstream_write( lua_State * L ) {
	const char * buf_ptr    = luaL_checkstring(L, 2);
	size_t buf_sz_ptr       = luaL_checkinteger(L, 3);
	sox_signalinfo_t signal = {};
	lua_getfield(L, 4, "rate");
		signal.rate = luaL_checkinteger(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 4, "channels");
		signal.channels = luaL_checkinteger(L, -1);
	lua_pop(L, 1);
	lua_getfield(L, 4, "precision");
		signal.precision = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	sox_format_t* input = sox_open_memstream_write(
		(char **)buf_ptr, (size_t *)buf_sz_ptr,
		&signal, NULL, NULL, NULL
	);

	if ( input == NULL ){
		lua_pushnil(L);
	}else{
		lua_pushlightuserdata(L, input);
	}

	return 1;
}


/***
 * Reads samples from a decoding session into a sample buffer.
 * @function sox_read
 * @tparam userdata ft Format pointer.
 * @tparam userdata buf Buffer from which to read samples.
 * @tparam integer len Number of samples available in buf.
 * @treturn integer Number of samples decoded, or 0 for EOF.
 * @see sox_read
 */
static int l_sox_read( lua_State * L ) {
	ctx_t *ctx        = check_config(L);
	sox_format_t* ft  = lua_touserdata(L, 2);
	sox_sample_t* buf = lua_touserdata(L, 3);
	size_t len  	  = luaL_checkinteger(L, 4);

	size_t sz 	  = sox_read(ft, buf, len);

	lua_pushinteger(L, sz);
	return 1;
}


/***
 * Opens an encoding session for a file. Returned handle must be closed with sox:close().
 * @function sox_open_write
 * @tparam string path Path to file to be written (required).
 * @tparam array signal Information about desired audio stream (required).
 * @tparam string filetype Previously-determined file type, or NULL to auto-detect.
 * @treturn userdata|nil handle The new session handle, or nil on failure..
 * @see sox_open_write
 */
static int l_sox_open_write( lua_State * L ) {
	//ctx_t *ctx            = check_config(L);
	char const * path     = luaL_checkstring(L, 2);
	char const * filetype = luaL_checkstring(L, 4);
	sox_signalinfo_t signal;

	if ( lua_islightuserdata(L, 3) ){
		sox_format_t* ft = lua_touserdata(L, 3);
		signal = ft->signal;		
	} else if ( lua_istable(L, 3) ) {
		sox_signalinfo_t signal = {};
		lua_getfield(L, 3, "rate");
			signal.rate = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 3, "channels");
			signal.channels = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 3, "precision");
			signal.precision = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
	}

	sox_format_t* output = sox_open_write(
		path, &signal, NULL, filetype, NULL, NULL
	);

	if ( output == NULL ) {
		lua_pushnil(L);
	} else {
		lua_pushlightuserdata(L, output);
	}

	return 1;
}


/***
 * Writes samples to an encoding session from a sample buffer.
 * @function sox_write
 * @tparam userdata ft Format pointer.
 * @tparam userdata buf Buffer from which to read samples.
 * @tparam integer len Number of samples available in buf.
 * @treturn integer Number of samples encoded..
 * @see sox_write
 */
static int l_sox_write( lua_State * L ) {
	if (!lua_islightuserdata(L, 2)) {
		return luaL_argerror(L, 2, "Invalid argument");
	} else if (!lua_islightuserdata(L, 3)) {
		return luaL_argerror(L, 3, "Invalid argument");
	}

	sox_format_t* ft  = lua_touserdata(L, 2);
	sox_sample_t* buf = lua_touserdata(L, 3);
	size_t len        = luaL_checkinteger(L, 4);
	size_t sz 	  = sox_write(ft, buf, len);

	lua_pushinteger(L, sz);
	return 1;
}


/***
 * Sets the location at which next samples will be decoded.
 * @function sox_seek
 * @tparam userdata ft Format pointer.
 * @tparam integer offset Sample offset at which to position reader.
 * @treturn boolean returns true if successful.
 * @see sox_seek
 */
static int l_sox_seek( lua_State * L ) {
	sox_format_t* ft    = lua_touserdata(L, 2);
	sox_uint64_t offset = luaL_checkinteger(L, 3);
	//int whence 	    = luaL_checkinteger(L, 4);
	if (sox_seek(ft, offset, SOX_SEEK_SET) != SOX_SUCCESS) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, 1);
	}
	return 1;
}


/***
 * Closes an encoding or decoding session.
 * @function sox_close
 * @treturn boolean returns true if successful.
 * @see sox_close
 */
static int l_sox_close( lua_State * L ) {
	if (!lua_islightuserdata(L, 2)){
		return luaL_argerror(L, 2, "Invalid argument");
	}
	sox_format_t* ft = lua_touserdata(L, 2);
	if (sox_close(ft) != SOX_SUCCESS) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, 1);
	}	
	return 1;
}


static const struct luaL_Reg funcs [] = {
    {"new", l_new},
    {NULL, NULL}
};

static const struct luaL_Reg meths [] = {
    {"version", l_sox_version},
    {"init", l_sox_init},
    {"quit", l_sox_quit},
    {"buffer", l_buffer},
    {"sample_to_float32", l_sample_to_float32},
    {"sample_to_float64", l_sample_to_float64},
    {"get_levels", l_get_levels},
    {"signal", l_signal},
    {"encoding", l_encoding},
    {"create_effects_chain", l_sox_create_effects_chain},
    {"create_effect", l_sox_create_effect},
    {"find_effect", l_sox_find_effect},
    {"effect_options", l_sox_effect_options},
    {"add_effect", l_sox_add_effect},
    {"flow_effects", l_sox_flow_effects},
    {"delete_effects_chain", l_sox_delete_effects_chain},
    {"open_read", l_sox_open_read},
    {"open_mem_read", l_sox_open_mem_read},
    {"open_memstream_write", l_sox_open_memstream_write},
    {"read", l_sox_read},
    {"open_write", l_sox_open_write},
    {"write", l_sox_write},
    {"seek", l_sox_seek},
    {"close", l_sox_close},
    {NULL, NULL}
};


int luaopen_libsox (lua_State *L) {
	luaL_newmetatable(L, SOX);
	lua_pushvalue(L, -1);

	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, meths, 0);

#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, funcs);
#else
	luaL_register(L, "sox", funcs);
#endif
	return 1;
}

