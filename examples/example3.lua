#!/usr/bin/env lua

--[[
 @filename  example3.lua
 @version   1.0
 @autor     Máster Vitronic <mastervitronic@gmail.com>
 @date      Fri Mar 12 03:47:42 -04 2021
 @licence   MIT licence
]]--

--[[
On an alsa capable system, plays an audio file starting 10 seconds in.
Copes with sample-rate and channel change if necessary since its
common for audio drivers to support a subset of rates and channel
counts.
E.g. ./example3.lua sound.ogg

Can easily be changed to work with other audio device drivers supported
by libSoX; e.g. "oss", "ao", "coreaudio", etc.
See the soxformat(7) manual page.
]]--

local libsox	= require("libsox")
local sox	= libsox:new()
local effect	= nil
--print( sox:version() )


--All libSoX applications must start by initialising the SoX library
if not sox:init() then
	error("Failed to initialize SoX")
end

--Open audiofile
local input = sox:open_read(arg[1])
if not input then
	error("Failed to open input handler")
end
local in_signal = sox:signal(input)

--Open soundcard
--Change "alsa" in this line to use an alternative audio device driver:
local output = sox:open_write(
	'default', in_signal, 'alsa'
)
if not output then
	error("Failed to open output handler")
end
local out_signal = sox:signal(output)

--Create an effects chain; some effects need to know about the input
--or output file encoding so we provide that information here
local chain = sox:create_effects_chain(input, output)

local interm_signal = input  -- NB: deep copy

effect = sox:create_effect(sox:find_effect("input"))
	assert(sox:effect_options(effect, 1, input))
	assert(sox:add_effect(chain, effect, input, input))
effect = nil

effect = sox:create_effect(sox:find_effect("trim"))
	assert(sox:effect_options(effect, 1, "10"))
	assert(sox:add_effect(chain, effect, interm_signal, input))
effect = nil

if ( in_signal.rate ~= out_signal.rate ) then
	effect = sox:create_effect(sox:find_effect("rate"))
		assert(sox:effect_options(effect, 0, nil))
		assert(sox:add_effect(chain, effect, interm_signal, input))
	effect = nil
end

if ( in_signal.channels ~= out_signal.channels ) then
	effect = sox:create_effect(sox:find_effect("channels"))
		assert(sox:effect_options(effect, 0, nil))
		assert(sox:add_effect(chain, effect, interm_signal, output))
	effect = nil
end

effect = sox:create_effect(sox:find_effect("output"))
	assert(sox:effect_options(effect, 1, output))
	assert(sox:add_effect(chain, effect, input, input))
effect = nil

--Flow samples through the effects processing chain until EOF is reached
sox:flow_effects(chain)

--All done; tidy up:
sox:delete_effects_chain(chain)
sox:close(input)
sox:close(output)
sox:quit()
