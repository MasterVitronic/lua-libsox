package = "lua-libsox"
version = "scm-0"

source = {
  url = "git://github.com/MasterVitronic/lua-libsox"
}

description = {
  summary = "Lua Binding for libsox",
  detailed = [[
    Lua-libsox is a Lua binding library for the Swiss Army knife of sound processing programs (SoX)
  ]],
  license = "MIT/X11",
  homepage = "https://gitlab.com/vitronic/lua-libsox"
}

dependencies = {
  "lua >= 5.1"
}

build = {
  type = "builtin",
  copy_directories = { 'examples' },
  modules = {
    ["libsox"] = {
      sources   = { "lua-libsox.c" },
      libraries = { "sox"          },
      incdirs   = { "$(PWD)" }
    }
  },
}
