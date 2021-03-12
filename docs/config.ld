-- Config file for ldoc
project = "lua-libsox"
description = "Lua bindings to libsox"
dir = "docs" -- for compatibility with github pages
-- allow links to upstream documentation
local upat = "https://fossies.org/dox/sox-14.4.2/formats_8c.html"
custom_see_handler('^(sox_[%w_]+)$',function(name)
    local url = upat:format(name)
    return name, url
end)

format='discount'
-- backtick_references=false
file={
    '../lua-libsox.c',
    '../compat.h',
--    exclude = {
--	'../extra/test/'
--    }
}
readme='../README.md'
style='!pale'
kind_names={topic='Manual',script='Programs'}
examples = {
    '../examples/example0.lua',
    '../examples/example2.lua',
    '../examples/example3.lua',
    '../examples/spectrogram.lua'
}
