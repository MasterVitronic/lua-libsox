#!/usr/bin/env lua

--[[
 @filename  example2.lua
 @version   1.0
 @autor     Máster Vitronic <mastervitronic@gmail.com>
 @date      Fri Mar 12 03:47:42 -04 2021
 @licence   MIT licence
 @use ./example2.lua sound.ogg 0 10
]]--

--similar to spectrogram.lua but with more didactic purpose
--E.g. ./example2.lua sound.ogg 0 10

local libsox 	   = require("libsox")
local sox    	   = libsox:new()
--local bit          = require('bit')

local block_period = 0.025 --default reading periods
local start_secs   = 0.0   --second to start (default value)
local period	   = 2.0   --seconds for end (default value)

if arg[2] then
	start_secs = tonumber(arg[2])
end

if arg[3] then
	period = tonumber(arg[3])
end

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
	'default', signal, 'alsa'
)
if not output then
	error("Failed to open output handler")
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

local stdout,blocks = nil , 0
while ((sox:read(input, buffer, block_size) == block_size) and ((blocks * block_period)< period) ) do
	local left, right = 0,0
	local  i=0

	--a way (require bit )
	--repeat i=i+1
		--local sample = sox:sample_to_float64(buffer,i)
		--if ( bit.band(i, 1) ~= 0 ) then
			--right = math.max(right, math.abs(sample)); --Find the peak volume in the block
		--else
			--left = math.max(left, math.abs(sample)); --Find the peak volume in the block
		--end
	--until (i == block_size)

	--another way (require bit )
	--while (i < block_size) do
		--i=i+1
		--if (i == block_size) then break end
		--local sample = sox:sample_to_float64(buffer,i)
		--if ( bit.band(i, 1) ~= 0 ) then
			--right = math.max(right, math.abs(sample)); --Find the peak volume in the block
		--else
			--left = math.max(left, math.abs(sample)); --Find the peak volume in the block
		--end
	--end

	--yet another way (require bit )
	--for i = 0, block_size, 1 do
		--if (i == block_size) then break end
		--local sample = sox:sample_to_float64(buffer,i)
		--if ( bit.band(i, 1) ~= 0 ) then
			--right = math.max(right, math.abs(sample)); --Find the peak volume in the block
		--else
			--left = math.max(left, math.abs(sample)); --Find the peak volume in the block
		--end
	--end

	--if you use any of the above proposals, you will have to
	--uncomment this block, and comment the fastest way to do it
	--local l = ((1.0 - left) * 35.0 + 0.5)
	--local r = ((1.0 - right) * 35.0 + 0.5)

	--the fastest way
	--no dependencies as the heavy lifting is done by the binding
	local r,l = sox:get_levels(buffer,block_size)

	--write in souncard
	sox:write(output, buffer, block_size)

	--I draw the spectrogram
	stdout = ("%8.3f%36s|%s\n"):format(
		(start_secs + blocks * block_period),
		line(l), line(r)
	)
	io.write(stdout)
	blocks = blocks + 1
end

--All done; tidy up:
sox:close(input)
sox:close(output)
sox:quit()
