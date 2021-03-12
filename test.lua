#!/usr/bin/env lua

--[[
 @filename  test.lua
 @version   1.0
 @autor     Máster Vitronic <mastervitronic@gmail.com>
 @date      Fri Mar 12 03:47:42 -04 2021
 @licence   MIT licence
]]--

local libsox = require("libsox")
local sox    = libsox:new()
print( sox:version() )

--All libSoX applications must start by initialising the SoX library
if not sox:init() then
	error("Failed to initialize SoX")
end

--Open audiofile
local input  = sox:open_read(arg[1])
if not input then
	error("Failed to open file")
end
local signal = sox:signal(input)

--Open soundcard
--Change "alsa" in this line to use an alternative audio device driver:
local output = sox:open_write('default',signal, 'alsa')
if not output then
	error("Failed to open output")
end

--a way to open the sound card with custom parameters
--local output = sox:open_write('default',{
	--rate=8000,channels=1,precision=32
--},'alsa')

--Or if you prefer, store all this to another file
--local output = sox:open_write('/tmp/coso.mp3',input, 'mp3')

--The buffer size
local buf_sz = 8192*2
--initialize the buffer
local buffer = sox:buffer(buf_sz)

--The main Loop
while true do
	local sz = sox:read(input,buffer,buf_sz)
	if sz == 0 then break end
	sox:write(output, buffer, sz)
end

--All done; tidy up:
sox:close(input)
sox:close(output)
sox:quit()

