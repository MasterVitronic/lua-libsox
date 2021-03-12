#!/usr/bin/env lua

--[[
 @filename  example0.lua
 @version   1.0
 @autor     Máster Vitronic <mastervitronic@gmail.com>
 @date      Fri Mar 12 03:47:42 -04 2021
 @licence   MIT licence
]]--

-- Reads input file, applies vol & flanger effects, and play in soundcard.
-- E.g. ./example0.lua sound.ogg


local libsox	= require("libsox")
local sox	= libsox:new()
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
local signal = sox:signal(input)

--Open soundcard
--Change "alsa" in this line to use an alternative audio device driver:
local output = sox:open_write(
	'default', signal, 'alsa' --Or pulseaudio Or ao etc
)
if not output then
	error("Failed to open output handler")
end

--Create an effects chain; some effects need to know about the input
--or output file encoding so we provide that information here
local chain = sox:create_effects_chain(input, output)
local effect = nil

--The first effect in the effect chain must be something that can source
--samples; in this case, we use the built-in handler that inputs
--data from an audio file
effect = sox:create_effect(sox:find_effect("input"))
	assert(sox:effect_options(effect, 1, input))
	--This becomes the first `effect' in the chain
	assert(sox:add_effect(chain, effect, input, input))
effect = nil

--Create the `vol' effect, and initialise it with the desired parameters:
effect = sox:create_effect(sox:find_effect("vol"))
	assert(sox:effect_options(effect, 1, "3dB"))
	--Add the effect to the end of the effects processing chain:
	assert(sox:add_effect(chain, effect, input, input))
effect = nil

--Create the `flanger' effect, and initialise it with default parameters:
effect = sox:create_effect(sox:find_effect("flanger"))
	assert(sox:effect_options(effect, 0, nil))
	--Add the effect to the end of the effects processing chain:
	assert(sox:add_effect(chain, effect, input, input))
effect = nil

--The last effect in the effect chain must be something that only consumes
--samples; in this case, we use the built-in handler that outputs
--data to an audio file
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
