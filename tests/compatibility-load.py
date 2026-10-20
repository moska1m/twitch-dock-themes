import ctypes,os,sys,json
runtime=sys.argv[1];pluginpath=sys.argv[2]
handle=os.add_dll_directory(runtime)
obs=ctypes.CDLL(os.path.join(runtime,'obs.dll'))
obs.obs_get_version.restype=ctypes.c_uint32
qt=ctypes.CDLL(os.path.join(runtime,'Qt6Core.dll'))
qversion=getattr(qt,'qVersion');qversion.restype=ctypes.c_char_p
plugin=ctypes.CDLL(pluginpath)
plugin.obs_module_ver.restype=ctypes.c_uint32
plugin.obs_module_load.restype=ctypes.c_bool
assert plugin.obs_module_ver()<=obs.obs_get_version()
assert plugin.obs_module_load()
print(json.dumps({'obsApi':hex(obs.obs_get_version()),'qt':qversion().decode(),'compiledObsApi':hex(plugin.obs_module_ver()),'dllLoad':True}))
