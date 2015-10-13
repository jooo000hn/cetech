from cetech.shared.proxy import ConsoleProxy


class ConsoleAPI(ConsoleProxy):
    def lua_execute(self, script):
        self.send_command('lua.execute', script=script)

    def compile_all(self):
        self.send_command('resource_compiler.compile_all')

    def autocomplete_list(self):
        self.lua_execute('autocomplite_list()')

    def resize(self, w, h):
        self.lua_execute('Application.resize(%s, %s)' % (w, h))
