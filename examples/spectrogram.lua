#!/usr/bin/env lua

--[[
 @filename  spectrogram.lua
 @version   1.0
 @autor     Máster Vitronic <mastervitronic@gmail.com>
 @date      Fri Mar 12 03:47:42 -04 2021
 @licence   MIT licence
]]--

--draws a beautiful spectrogram while playing back the sound
--E.g. ./spectrogram.lua sound.ogg

local libsox 	   = require("libsox")
local sox    	   = libsox:new()

local block_period = 0.025 --default reading periods
local start_secs   = 0.0   --second to start
local period	   = 2.0   --seconds for end

--print( sox:version() )
if not arg[1] then
	error("an audio file is required")
end

if arg[2] then
	start_secs = tonumber(arg[2])
end

if arg[3] then
	period     = tonumber(arg[3])
end

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

-- By default, if the period is not passed, it is set to the
-- total time of the audio.
if not arg[3] then
	period = math.modf(signal.length
		/ math.max(signal.channels, 1)
		/ math.max(signal.rate, 1)
	)
end

--Calculate the start position in number of samples:
seek = (start_secs * signal.rate * signal.channels + 0.5)
--Make sure that this is at a `wide sample' boundary:
seek = seek - seek % signal.channels
--Move the file pointer to the desired starting position
sox:seek(input,seek)

--Convert block size (in seconds) to a number of samples:
block_size = (block_period * signal.rate * signal.channels + 0.5)
--Make sure that this is at a `wide sample' boundary:
block_size = (block_size - block_size % signal.channels)
--Allocate a block of memory to store the block of audio samples:
local buffer = sox:buffer(block_size)

--whrite vumeter
function line(n)
	local LINE = "==================================="
	return (LINE):gsub("=", "", math.modf(n))
end

--Main loop
local stdout,blocks = nil , 0
while ((blocks * block_period) <= period) do
	local sz = sox:read(input, buffer, block_size)
	if sz == 0 then break end
	local right,left = sox:get_levels(buffer,sz)
	sox:write(output, buffer, sz)

	stdout = ("%8.3f%36s|%s\n"):format(
		(start_secs + blocks * block_period),
		line(left), line(right)
	)
	io.write(stdout)
	blocks = blocks + 1
end

--exit
sox:close(input)
sox:close(output)
sox:quit()

