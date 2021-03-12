## Lua-libsox: Lua bindings for libSoX

Lua-libsox is a Lua binding library for the [Swiss Army knife of sound processing programs (SoX)](http://sox.sourceforge.net/).

It runs on GNU/Linux and requires [Lua](http://www.lua.org/) (>=5.1)
and [SoX](http://sox.sourceforge.net/) (>=14.4.2).

_Authored by:_ _[Díaz Devera Víctor Diex Gamar (Máster Vitronic)](https://www.linkedin.com/in/Master-Vitronic)_

[![Lua logo](https://gitlab.com/vitronic/lua-libsox/-/raw/3a2ad8f5c78b54f1de6dec8019f2c743901e621f/docs/powered-by-lua.gif)](http://www.lua.org/)

#### License

MIT license . See [LICENSE](./LICENSE).

#### Documentation

See the [Reference Manual](https://vitronic.gitlab.io/lua-libsox/).

#### Motivation:

I was looking for a long time without success, to manipulate and play
audio files from Lua, unfortunately these features were not available
until now, certainly there are other bindings (like SDL2 or ao) for audio support,
but none comes close to the power of [SoX](http://sox.sourceforge.net/) to encode/decode, play and add
effects to audio files.


#### Getting and installing

```sh
$ git clone https://gitlab.com/vitronic/lua-libsox.git
$ cd lua-libsox
lua-libsox$ make
lua-libsox$ make install # or 'sudo make install' (Ubuntu)
```

#### Example

```lua
-- Script: player.lua

local libsox = require("libsox")
local sox    = libsox:new()

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
local output = sox:open_write('default',signal, 'alsa') --Or pulseaudio Or ao etc
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
```

The script can be executed at the shell prompt with the standard Lua interpreter:

```shell
$ lua player.lua sound.ogg
```

Other examples can be found in the **examples/** directory contained in the release package

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
