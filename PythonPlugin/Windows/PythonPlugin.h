/*
Copyright (c) 2017 Adam Biser <adambiser@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef PYTHONPLUGIN_H_
#define PYTHONPLUGIN_H_

#include "..\AGKLibraryCommands.h"

/*
NOTE: Cannot use bool as an exported function return type because of AGK2 limitations.  Use int instead.
*/

//https://docs.python.org/3/c-api/init.html
extern "C" DLL_EXPORT void _Py_Initialize();
extern "C" DLL_EXPORT int _Py_IsInitialized();
extern "C" DLL_EXPORT int _Py_Finalize();
extern "C" DLL_EXPORT void _Py_SetProgramName(char *name);
extern "C" DLL_EXPORT char *_Py_GetProgramName();
extern "C" DLL_EXPORT char * _Py_GetPrefix();
extern "C" DLL_EXPORT char *_Py_GetExecPrefix();
extern "C" DLL_EXPORT char *_Py_GetProgramFullPath();
extern "C" DLL_EXPORT char *_Py_GetPath();
extern "C" DLL_EXPORT void _Py_SetPath(char *path);
extern "C" DLL_EXPORT char *_Py_GetVersion();
extern "C" DLL_EXPORT char *_Py_GetPlatform();
extern "C" DLL_EXPORT char *_Py_GetCopyright();
extern "C" DLL_EXPORT char *_Py_GetCompiler();
extern "C" DLL_EXPORT char *_Py_GetBuildInfo();
extern "C" DLL_EXPORT void _Py_SetPythonHome(char *home);
extern "C" DLL_EXPORT char *_Py_GetPythonHome();

// helper functions
extern "C" DLL_EXPORT int GetMainModuleDict();

//https://docs.python.org/3/c-api/veryhigh.html
extern "C" DLL_EXPORT int _PyRun_SimpleString(char *command);
extern "C" DLL_EXPORT int _PyRun_SimpleFile(const char *filename);
extern "C" DLL_EXPORT int _PyRun_String(char *script, int hglobals, int hlocals);
extern "C" DLL_EXPORT int _PyRun_File(const char *filename, int hglobals, int hlocals);
//PyObject* Py_CompileString(const char *str, const char *filename, int start)

//https://docs.python.org/3/c-api/refcounting.html
extern "C" DLL_EXPORT void _Py_INCREF(int hobject);
extern "C" DLL_EXPORT void _Py_XINCREF(int hobject);
extern "C" DLL_EXPORT void _Py_DECREF(int hobject);
extern "C" DLL_EXPORT void _Py_XDECREF(int hobject);
extern "C" DLL_EXPORT void _Py_CLEAR(int hobject);

//https://docs.python.org/3/c-api/structures.html
extern "C" DLL_EXPORT char *_Py_TYPE_NAME(int hobject);
extern "C" DLL_EXPORT int _Py_REFCNT(int hobject);
extern "C" DLL_EXPORT int _Py_SIZE(int hobject);

//https://docs.python.org/3/c-api/import.html
extern "C" DLL_EXPORT int _PyImport_ImportModule(const char *name);
extern "C" DLL_EXPORT int _PyImport_ImportModuleEx(const char *name, int hglobals, int hlocals, int hfromlist);
extern "C" DLL_EXPORT int _PyImport_Import(int hname);
extern "C" DLL_EXPORT int _PyImport_ImportS(const char *name);
extern "C" DLL_EXPORT int _PyImport_ReloadModule(int hModule);
extern "C" DLL_EXPORT int _PyImport_AddModule(char * name);
extern "C" DLL_EXPORT int _PyImport_GetModuleDict();

//https://docs.python.org/3/c-api/module.html
extern "C" DLL_EXPORT int _PyModule_Check(int hobject);
extern "C" DLL_EXPORT int _PyModule_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PyModule_New(const char *name);
extern "C" DLL_EXPORT int _PyModule_GetDict(int hmodule);
extern "C" DLL_EXPORT int _PyModule_GetNameObject(int hmodule);
extern "C" DLL_EXPORT char *_PyModule_GetName(int hmodule);

//https://docs.python.org/3/c-api/arg.html
extern "C" DLL_EXPORT int _Py_BuildValue(const char *format, char *csvtext); // Allowed format chars: szidf()[]{}

//https://docs.python.org/3/c-api/object.html
extern "C" DLL_EXPORT int _PyObject_HasAttr(int hobject, int hattr_name);
extern "C" DLL_EXPORT int _PyObject_HasAttrString(int hobject, const char *attr_name);
extern "C" DLL_EXPORT int _PyObject_GetAttrHandle(int hobject, int hattr_name);
extern "C" DLL_EXPORT int _PyObject_GetAttrHandleS(int hobject, const char *attr_name);
extern "C" DLL_EXPORT float _PyObject_GetAttrFloat(int hobject, const char *attr_name);
extern "C" DLL_EXPORT int _PyObject_GetAttrInt(int hobject, const char *attr_name);
extern "C" DLL_EXPORT const char *_PyObject_GetAttrString(int hobject, const char *attr_name);
extern "C" DLL_EXPORT int _PyObject_SetAttrHandle(int hobject, int hattr_name, int hvalue);
extern "C" DLL_EXPORT int _PyObject_SetAttrHandleS(int hobject, const char *attr_name, int hvalue);
extern "C" DLL_EXPORT int _PyObject_SetAttrFloat(int hobject, const char *attr_name, float value);
extern "C" DLL_EXPORT int _PyObject_SetAttrInt(int hobject, const char *attr_name, int value);
extern "C" DLL_EXPORT int _PyObject_SetAttrString(int hobject, const char *attr_name, const char *value);
extern "C" DLL_EXPORT int _PyObject_DelAttr(int hobject, int hattr_name);
extern "C" DLL_EXPORT int _PyObject_DelAttrString(int hobject, const char *attr_name);
extern "C" DLL_EXPORT int _PyObject_ReprObj(int hobject);
extern "C" DLL_EXPORT const char *_PyObject_Repr(int hobject);
extern "C" DLL_EXPORT int _PyObject_StrObj(int hobject);
extern "C" DLL_EXPORT const char *_PyObject_Str(int hobject);
extern "C" DLL_EXPORT int _PyCallable_Check(int hobject);
extern "C" DLL_EXPORT int _PyObject_Call(int hcallable_object, int hargs, int hkw);
extern "C" DLL_EXPORT int _PyObject_Length(int hobject);
extern "C" DLL_EXPORT int _PyObject_GetItem(int hobject, int hkey);
extern "C" DLL_EXPORT int _PyObject_SetItem(int hobject, int hkey, int hvalue);
extern "C" DLL_EXPORT int _PyObject_DelItem(int hobject, int hkey);
extern "C" DLL_EXPORT int _PyObject_GetIter(int hobject);

//https://docs.python.org/3/c-api/long.html
extern "C" DLL_EXPORT int _PyLong_Check(int hobject);
extern "C" DLL_EXPORT int _PyLong_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PyLong_FromLong(int value);
extern "C" DLL_EXPORT int _PyLong_AsLong(int hlong);

//https://docs.python.org/3/c-api/float.html
extern "C" DLL_EXPORT int _PyFloat_Check(int hobject);
extern "C" DLL_EXPORT int _PyFloat_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PyFloat_FromDouble(float value);
extern "C" DLL_EXPORT float _PyFloat_AsDouble(int hdouble);

//https://docs.python.org/3/c-api/unicode.html
extern "C" DLL_EXPORT int _PyUnicode_Check(int hobject);
extern "C" DLL_EXPORT int _PyUnicode_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PyUnicode_FromString(const char *value);
extern "C" DLL_EXPORT const char *_PyUnicode_AsStringPL(int hunicode);

//https://docs.python.org/3/c-api/tuple.html
extern "C" DLL_EXPORT int _PyTuple_Check(int hobject);
extern "C" DLL_EXPORT int _PyTuple_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PyTuple_New(int size);
//extern "C" DLL_EXPORT int _PyTuple_Pack(int size, ...);
extern "C" DLL_EXPORT int _PyTuple_Size(int hobject);
extern "C" DLL_EXPORT int _PyTuple_GetItemHandle(int hobject, int pos);
extern "C" DLL_EXPORT float _PyTuple_GetItemFloat(int hobject, int pos);
extern "C" DLL_EXPORT int _PyTuple_GetItemInt(int hobject, int pos);
extern "C" DLL_EXPORT const char *_PyTuple_GetItemString(int hobject, int pos);
extern "C" DLL_EXPORT int _PyTuple_GetSlice(int hobject, int low, int high);
extern "C" DLL_EXPORT int _PyTuple_SetItemHandle(int hobject, int pos, int hvalue);
extern "C" DLL_EXPORT int _PyTuple_SetItemFloat(int hobject, int pos, float value);
extern "C" DLL_EXPORT int _PyTuple_SetItemInt(int hobject, int pos, int value);
extern "C" DLL_EXPORT int _PyTuple_SetItemString(int hobject, int pos, const char *value);

//https://docs.python.org/3/c-api/list.html
extern "C" DLL_EXPORT int _PyList_Check(int hobject);
extern "C" DLL_EXPORT int _PyList_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PyList_New(int size);
extern "C" DLL_EXPORT int _PyList_Size(int hlist);
extern "C" DLL_EXPORT int _PyList_GetItemHandle(int hlist, int index);
extern "C" DLL_EXPORT float _PyList_GetItemFloat(int hlist, int index);
extern "C" DLL_EXPORT int _PyList_GetItemInt(int hlist, int index);
extern "C" DLL_EXPORT const char *_PyList_GetItemString(int hlist, int index);
extern "C" DLL_EXPORT int _PyList_SetItemHandle(int hlist, int index, int hitem);
extern "C" DLL_EXPORT int _PyList_SetItemFloat(int hlist, int index, float item);
extern "C" DLL_EXPORT int _PyList_SetItemInt(int hlist, int index, int item);
extern "C" DLL_EXPORT int _PyList_SetItemString(int hlist, int index, const char *item);
extern "C" DLL_EXPORT int _PyList_InsertHandle(int hlist, int index, int hitem);
extern "C" DLL_EXPORT int _PyList_InsertFloat(int hlist, int index, float value);
extern "C" DLL_EXPORT int _PyList_InsertInt(int hlist, int index, int value);
extern "C" DLL_EXPORT int _PyList_InsertString(int hlist, int index, const char *value);
extern "C" DLL_EXPORT int _PyList_AppendHandle(int hlist, int hitem);
extern "C" DLL_EXPORT int _PyList_AppendFloat(int hlist, float value);
extern "C" DLL_EXPORT int _PyList_AppendInt(int hlist, int value);
extern "C" DLL_EXPORT int _PyList_AppendString(int hlist, const char *value);
extern "C" DLL_EXPORT int _PyList_GetSlice(int hlist, int low, int high);
extern "C" DLL_EXPORT int _PyList_SetSlice(int hlist, int low, int high, int hitemlist);
extern "C" DLL_EXPORT int _PyList_Sort(int hlist);
extern "C" DLL_EXPORT int _PyList_Reverse(int hlist);
extern "C" DLL_EXPORT int _PyList_AsTuple(int hlist);

//https://docs.python.org/3/c-api/dict.html
extern "C" DLL_EXPORT int _PyDict_Check(int hobject);
extern "C" DLL_EXPORT int _PyDict_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PyDict_New();
extern "C" DLL_EXPORT void _PyDict_Clear(int hdict);
extern "C" DLL_EXPORT int _PyDict_ContainsKey(int hdict, int hkey);
extern "C" DLL_EXPORT int _PyDict_ContainsKeyS(int hdict, const char *key);
extern "C" DLL_EXPORT int _PyDict_Copy(int hdict);
extern "C" DLL_EXPORT int _PyDict_SetItemHandle(int hdict, int hkey, int hvalue);
extern "C" DLL_EXPORT int _PyDict_SetItemHandleS(int hdict, const char *key, int hvalue);
extern "C" DLL_EXPORT int _PyDict_SetItemFloat(int hdict, const char *key, float value);
extern "C" DLL_EXPORT int _PyDict_SetItemInt(int hdict, const char *key, int value);
extern "C" DLL_EXPORT int _PyDict_SetItemString(int hdict, const char *key, const char *value);
extern "C" DLL_EXPORT int _PyDict_DelItem(int hdict, int hkey);
extern "C" DLL_EXPORT int _PyDict_DelItemString(int hdict, const char *key);
extern "C" DLL_EXPORT int _PyDict_GetItemHandle(int hdict, int hkey);
extern "C" DLL_EXPORT int _PyDict_GetItemHandleS(int hdict, const char *key);
extern "C" DLL_EXPORT float _PyDict_GetItemFloat(int hdict, const char *key);
extern "C" DLL_EXPORT int _PyDict_GetItemInt(int hdict, const char *key);
extern "C" DLL_EXPORT const char *_PyDict_GetItemString(int hdict, const char *key);
//extern "C" DLL_EXPORT int _PyDict_GetItemWithError(int hdict, int hkey);
extern "C" DLL_EXPORT int _PyDict_SetDefault(int hdict, int hkey, int hdefault);
extern "C" DLL_EXPORT int _PyDict_Items(int hdict);
extern "C" DLL_EXPORT int _PyDict_Keys(int hdict);
extern "C" DLL_EXPORT int _PyDict_Values(int hdict);
extern "C" DLL_EXPORT int _PyDict_Size(int hdict);
extern "C" DLL_EXPORT int _PyDict_Merge(int hdicta, int hdictb, int override);
extern "C" DLL_EXPORT int _PyDict_Update(int hdicta, int hdictb);

//https://docs.python.org/3/c-api/set.html
extern "C" DLL_EXPORT int _PySet_Check(int hobject);
extern "C" DLL_EXPORT int _PyFrozenSet_Check(int hobject);
extern "C" DLL_EXPORT int _PyAnySet_Check(int hobject);
extern "C" DLL_EXPORT int _PyAnySet_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PyFrozenSet_CheckExact(int hobject);
extern "C" DLL_EXPORT int _PySet_New(int hiterable);
extern "C" DLL_EXPORT int _PyFrozenSet_New(int hiterable);
extern "C" DLL_EXPORT int _PySet_Size(int hset);
extern "C" DLL_EXPORT int _PySet_Contains(int hset, int hkey);
extern "C" DLL_EXPORT int _PySet_Add(int hset, int hkey);
extern "C" DLL_EXPORT int _PySet_Discard(int hset, int hkey);
extern "C" DLL_EXPORT int _PySet_Pop(int hset);
extern "C" DLL_EXPORT int _PySet_Clear(int hset);

#endif // PYTHONPLUGIN_H_