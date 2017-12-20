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

/*
NOTE: Cannot use bool as a return or parameter type for exported functions because of AGK2 limitations.  Use int instead.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <regex>
#include <string>
#include <vector>
#include <stdio.h>

#include "PythonPlugin.h"
#include "PythonErrorHandling.h"
#ifdef PLUGIN
#include "..\AGKLibraryCommands.h"
#endif

// Force use of the release build of python36.dll.
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

// These need to be stored staticly and should not be changed while Python is initialized.
wchar_t *m_ProgramName;
wchar_t *m_PythonHome;

/*
PyObject handles.

Rather than open the potential mess of passing pointer values back and forth between the DLL and AGK code,
this plugin indexes the PyObject pointers and hands back a 1-based index value instead.
*/
std::vector<PyObject *> m_Objects;

PyObject *GetPyObject(int handle)
{
	if (handle == 0)
	{
		return NULL;
	}
	// Handles are 1-based!
	if (handle > 0 && handle <= (int)m_Objects.size())
	{
		return m_Objects[handle - 1];
	}
	agk::PluginError("Invalid PyObject handle.");
	return NULL;
}

// Used to check required handles and report when they are 0.
#define REQUIRED_HANDLEV(handle)						\
	if (handle == 0)									\
	{													\
		std::string msg = __FUNCTION__;					\
		msg += ": Given required handle was null.";		\
		agk::PluginError(msg.c_str());					\
		return;											\
	}
#define REQUIRED_HANDLE(handle)							\
	if (handle == 0)									\
	{													\
		std::string msg = __FUNCTION__;					\
		msg += ": Given required handle was null.";		\
		agk::PluginError(msg.c_str());					\
		return NULL;									\
	}

int GetHandle(PyObject *object)
{
	if (object == NULL)
	{
		return 0;
	}
	int index = std::find(m_Objects.begin(), m_Objects.end(), object) - m_Objects.begin();
	if (index < (int)m_Objects.size())
	{
		// Handles are 1-based!
		return index + 1;
	}
	m_Objects.push_back(object);
	return m_Objects.size();
}

void ResetPyObjectHandleList()
{
	// Don't DECREF the objects here.  It causes PyFinalize to throw an exception.
	// Those who create the objects need to DECREF them themselves.
	//for (int index = 0; index < (int)m_Objects.size(); index++)
	//{
	//	PyObject *object = m_Objects.back();
	//	Py_CLEAR(object);
	//	m_Objects.pop_back();
	//}
	m_Objects.clear();
	m_Objects.push_back(Py_None); // Store Py_None as handle 1.
}

/*
Converts a wchar_t* to agk:string.
*/
char *CreateString(const char *text)
{
	if (text)
	{
		int length = (int)strlen(text) + 1;
		char *result = agk::CreateString(length);
		strcpy_s(result, length, text);
		return result;
	}
	char *result = agk::CreateString(1);
	strcpy_s(result, 1, "");
	return result;
}

char *CreateString(wchar_t *wtext)
{
	if (wtext == NULL)
	{
		return CreateString((const char *)NULL);
	}
	char *text = Py_EncodeLocale(wtext, NULL);
	if (text != NULL)
	{
		char *result = NULL;
		result = CreateString(text);
		PyMem_Free(text);
		return result;
	}
	return CreateString((const char *)NULL);
}

char *CreateString(PyObject *object)
{
	return CreateString(_PyUnicode_AsString(object));
}

wchar_t *DecodeStringEx(char *text, const char *caller)
{
	wchar_t *wtext = Py_DecodeLocale(text, NULL);
	if (wtext == NULL)
	{
		std::string msg = caller;
		msg += ": Cannot decode string.";
		agk::PluginError(msg.c_str());
	}
	return wtext;
}

#define DecodeString(text) DecodeStringEx(text, __FUNCTION__)

void FreeWChar(wchar_t * wtext)
{
	if (wtext != NULL)
	{
		PyMem_RawFree(wtext);
		wtext = NULL;
	}
}

std::vector<std::string> ParseCSV(std::string csv)
{
	std::regex re("(\"[^\"]*\")+|('[^']*')+|[^,]+");
	std::sregex_iterator next(csv.begin(), csv.end(), re);
	std::sregex_iterator end;
	std::vector<std::string> parts;
	while (next != end) {
		std::smatch match = *next;
		std::string part = match.str();
		switch (part[0])
		{
		case '"':
			part = part.substr(1, part.size() - 2);
			part = std::regex_replace(part, std::regex("\"\""), "\"");
			break;
		case '\'':
			part = part.substr(1, part.size() - 2);
			part = std::regex_replace(part, std::regex("''"), "'");
			break;
		}
		parts.push_back(part);
		next++;
	}
	return parts;
}

std::vector<std::string> ParseCSV(char * text)
{
	std::string csv(text);
	return ParseCSV(csv);
}

/*
https://docs.python.org/3/c-api/init.html
*/
void _Py_Initialize()
{
	ResetPyObjectHandleList();
	Py_InitializeEx(0);
	//Py_Initialize();
}

int _Py_IsInitialized()
{
	return Py_IsInitialized();
}

int _Py_Finalize()
{
	ResetPyObjectHandleList();
	FreeWChar(m_ProgramName);
	FreeWChar(m_PythonHome);
	return Py_FinalizeEx();
}

void _Py_SetProgramName(char *name)
{
	//  The argument should point to a zero-terminated wide character string in static storage whose contents will not change for the duration of the program's execution.
	if (Py_IsInitialized())
	{
		agk::PluginError("Py_SetProgramName cannot be called while Python is initialized.");
		return;
	}
	FreeWChar(m_ProgramName);
	m_ProgramName = DecodeString(name);
	if (m_ProgramName != NULL)
	{
		Py_SetProgramName(m_ProgramName);
	}
}

char *_Py_GetProgramName()
{
	return CreateString(Py_GetProgramName());
}

char *_Py_GetPrefix()
{
	return CreateString(Py_GetPrefix());
}

char *_Py_GetExecPrefix()
{
	return CreateString(Py_GetExecPrefix());
}

char *_Py_GetProgramFullPath()
{
	return CreateString(Py_GetProgramFullPath());
}

char *_Py_GetPath()
{
	return CreateString(Py_GetPath());
}

void _Py_SetPath(char *path)
{
	wchar_t *wPath = DecodeString(path);
	if (wPath != NULL)
	{
		Py_SetPath(wPath);
	}
	FreeWChar(wPath);
}

char *_Py_GetVersion()
{
	return CreateString(Py_GetVersion());
}

char *_Py_GetPlatform()
{
	return CreateString(Py_GetPlatform());
}

char *_Py_GetCopyright()
{
	return CreateString(Py_GetCopyright());
}

char *_Py_GetCompiler()
{
	return CreateString(Py_GetCompiler());
}

char *_Py_GetBuildInfo()
{
	return CreateString(Py_GetBuildInfo());
}

void _Py_SetPythonHome(char *home)
{
	if (Py_IsInitialized())
	{
		agk::PluginError("Py_SetPythonHome cannot be called while Python is initialized.");
		return;
	}
	FreeWChar(m_PythonHome);
	m_PythonHome = DecodeString(home);
	if (m_PythonHome != NULL)
	{
		Py_SetPythonHome(m_PythonHome);
	}
}

char *_Py_GetPythonHome()
{
	return CreateString(Py_GetPythonHome());
}

/*
Helper functions
*/
int GetMainModuleDict()
{
	PyObject *module = PyImport_AddModule("__main__"); // borrowed ref
	if (module == NULL)
	{
		agk::PluginError("GetMainModuleDict: PyImport_AddModule call failed.");
		return NULL;
	}
	//Py_IncRef(module);
	PyObject *dict = PyModule_GetDict(module); // borrowed ref
	if (dict == NULL)
	{
		agk::PluginError("GetMainModuleDict: PyModule_GetDict call failed.");
		return NULL;
	}
	return GetHandle(dict);
}

//int CallFunction(int hModule, const char *func_name, const char *format, char *args_csv)
//{
//	REQUIRED_HANDLE(hModule)
//	if (strlen(func_name) == 0)
//	{
//		agk::PluginError("CallFunction: No function name given.");
//		return 0;
//	}
//	PyObject *func = PyObject_GetAttrString(GetPyObject(hModule), func_name);
//	if (func == NULL)
//	{
//		agk::PluginError("CallFunction: Function not found.");
//		return 0;
//	}
//	PyObject *args;
//	if (strlen(format) == 0)
//	{
//		args = PyTuple_New(0);
//	}
//	else
//	{
//		args = _Py_BuildValue(format, args_csv);
//	}
//
//	PyObject *result = PyObject_Call(func, args, NULL);
//	Py_DecRef(func);
//}

/*
https://docs.python.org/3/c-api/veryhigh.html
*/
int _PyRun_SimpleString(char *command)
{
	int result = PyRun_SimpleString(command);
	if (result == -1)
	{
		agk::PluginError("PyRun_SimpleString; Error in string.");
	}
	return result;
}

int _PyRun_SimpleFile(const char *filename)
{
	// _Py_fopen must be used in order for this to work, not fopen.
	//if (FILE *fp = fopen(filename, "r"))
	// Only diference appears to be that _Py_fopen clears HANDLE_FLAG_INHERIT.
	if (FILE *fp = _Py_fopen(filename, "r"))
	{
		// Using PyRun_SimpleFile itself causes an assertion to occur in fclose.
		//int result = PyRun_SimpleFile(fp, filename);
		//fclose(fp);
		// Use PyRun_SimpleFileExFlags with closeit set to 1 so it does the fclose.
		int result = PyRun_SimpleFileExFlags(fp, filename, 1, NULL);
		if (result == -1)
		{
			agk::PluginError("PyRun_SimpleFile; Error in file.");
		}
		return result;
	}
	else
	{
		agk::PluginError("PyRun_SimpleFile: Failed to open file.");
	}
	return -1;
}

int _PyRun_String(char *script, int hglobals, int hlocals)
{
	// If globals and/or locals aren't provided, use an empty dict for them.
	PyObject *globals = (hglobals) ? GetPyObject(hglobals) : PyDict_New();
	PyObject *locals = (hlocals) ? GetPyObject(hlocals) : PyDict_New();
	PyObject *result = PyRun_String(script, Py_file_input, globals, locals);
	CheckError();
	// DECREF any created dict.
	if (!hglobals)
	{
		Py_DecRef(globals);
	}
	if (!hlocals)
	{
		Py_DecRef(locals);
	}
	return GetHandle(result);
}

int _PyRun_File(const char *filename, int hglobals, int hlocals)
{
	//if (!hglobals)
	//{
	//	// If globals aren't provided, use the main module dict.
	//	hglobals = GetMainModuleDict();
	//}
	if (FILE *fp = _Py_fopen(filename, "r"))
	{
		// If globals and/or locals aren't provided, use an empty dict for them.
		PyObject *globals = (hglobals) ? GetPyObject(hglobals) : PyDict_New();
		PyObject *locals = (hlocals) ? GetPyObject(hlocals) : PyDict_New();
		PyObject *result = PyRun_FileExFlags(fp, filename, Py_file_input, globals, locals, 1, NULL);
		CheckError();
		// DECREF any created dict.
		if (!hglobals)
		{
			Py_DecRef(globals);
		}
		if (!hlocals)
		{
			Py_DecRef(locals);
		}
		return GetHandle(result);
	}
	else
	{
		agk::PluginError("PyRun_File: Failed to open file.");
		return NULL;
	}
}

/*
https://docs.python.org/3/c-api/refcounting.html
*/
void _Py_INCREF(int hobject)
{
	REQUIRED_HANDLEV(hobject)
	PyObject *object = GetPyObject(hobject);
	Py_IncRef(object);
}

void _Py_XINCREF(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	Py_XINCREF(object);
}

void _Py_DECREF(int hobject)
{
	REQUIRED_HANDLEV(hobject)
	PyObject *object = GetPyObject(hobject);
	if (object != NULL && object->ob_refcnt == 1)
	{
		// Clear vector object.
		m_Objects[hobject - 1] = NULL;
	}
	Py_DecRef(object);
}

void _Py_XDECREF(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	if (object != NULL && object->ob_refcnt == 1)
	{
		// Clear vector object.
		m_Objects[hobject - 1] = NULL;
	}
	Py_XDECREF(object);
}

void _Py_CLEAR(int hobject)
{
	// Handles are 1-based!
	if (hobject > 0 && hobject <= (int)m_Objects.size())
	{
		Py_CLEAR(m_Objects[hobject - 1]);
		// Clear vector object.
		m_Objects[hobject - 1] = NULL;
	}
}

/*
https://docs.python.org/3/c-api/structures.html
*/
char *_Py_TYPE_NAME(int hobject)
{
	//REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	if (object == NULL)
	{
		return CreateString("null");
	}
	return CreateString(Py_TYPE(object)->tp_name);
}

int _Py_REFCNT(int hobject)
{
	//REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	if (object == NULL)
	{
		return 0;
	}
	return Py_REFCNT(object);
}

int _Py_SIZE(int hobject)
{
	//REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	if (object == NULL)
	{
		return -1;
	}
	return (int) Py_SIZE(object);
}

/*
https://docs.python.org/3/c-api/import.html
*/
int _PyImport_ImportModule(char *name)
{
	PyObject *module = PyImport_ImportModule(name);
	CheckError();
	return GetHandle(module);
}

int _PyImport_ImportModuleEx(const char *name, int hglobals, int hlocals, int hfromlist)
{
	PyObject *globals = GetPyObject(hglobals);
	PyObject *locals = GetPyObject(hlocals);
	PyObject *fromlist = GetPyObject(hfromlist);
	PyObject *module = PyImport_ImportModuleEx(name, globals, locals, fromlist);
	CheckError();
	return GetHandle(module);
}

int _PyImport_Import(int hname)
{
	REQUIRED_HANDLE(hname)
	PyObject *name = GetPyObject(hname);
	PyObject *import = PyImport_Import(name);
	CheckError();
	return GetHandle(import);
}

int _PyImport_ImportS(const char *name)
{
	PyObject *oname = PyUnicode_FromString(name);
	if (oname == NULL)
	{
		return NULL;
	}
	PyObject *import = PyImport_Import(oname);
	Py_DecRef(oname);
	CheckError();
	return GetHandle(import);
}

int _PyImport_ReloadModule(int hhodule)
{
	PyObject *module = GetPyObject(hhodule);
	PyObject *reloaded = PyImport_ReloadModule(module);
	CheckError();
	return GetHandle(reloaded);
}

int _PyImport_AddModule(char * name)
{
	PyObject *module = PyImport_AddModule(name);
	CheckError();
	return GetHandle(module);
}

int _PyImport_GetModuleDict()
{
	PyObject *dict = PyImport_GetModuleDict();
	return GetHandle(dict);
}

//https://docs.python.org/3/c-api/module.html
int _PyModule_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyModule_Check(object);
}

int _PyModule_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyModule_CheckExact(object);
}

int _PyModule_New(const char *name)
{
	PyObject *module = PyModule_New(name);
	return GetHandle(module);
}

int _PyModule_GetDict(int hmodule)
{
	PyObject *module = GetPyObject(hmodule);
	PyObject *dict = PyModule_GetDict(module);
	return GetHandle(dict);
}

int _PyModule_GetNameObject(int hmodule)
{
	PyObject *module = GetPyObject(hmodule);
	PyObject *object = PyModule_GetNameObject(module);
	return GetHandle(object);
}

char *_PyModule_GetName(int hmodule)
{
	PyObject *module = GetPyObject(hmodule);
	return CreateString(PyModule_GetName(module));
}

//https://docs.python.org/3/c-api/arg.html
// Allowed format chars: szidf()[]{}
int _Py_BuildValue(const char *format, char *csvtext)
{
	/*
	This section is a huge hack!
	Dynamically build a va_list from parsed csv text based upon the given format.
	*/
	std::vector<std::string> values = ParseCSV(csvtext);
	std::string sformat = std::regex_replace(format, std::regex("[^szifd]"), "");
	// Verify that this was given the number of values needed for the given format string.
	if (values.size() != sformat.size())
	{
		agk::PluginError("Py_BuildValue: Given argument count does not match format argument count.");
		return NULL;
	}
	int vamemsize = 0;
	vamemsize += std::regex_replace(format, std::regex("[^sz]"), "").size() * sizeof(const char*);
	vamemsize += std::regex_replace(format, std::regex("[^i]"), "").size() * sizeof(int);
	vamemsize += std::regex_replace(format, std::regex("[^df]"), "").size() * sizeof(double);
	char *va = new char[vamemsize];
	for (int argindex = 0, pindex = 0; argindex < (int)sformat.size(); argindex++)
	{
		switch (sformat[argindex])
		{
		case 's':
		case 'z':
		{
			const char *cstr = values[argindex].c_str();
			std::memcpy(&va[pindex], &cstr, sizeof(const char *));
			pindex += sizeof(const char *);
			break;
		}
		case 'i':
		{
			int i = atoi(values[argindex].c_str());
			std::memcpy(&va[pindex], &i, sizeof(int));
			pindex += sizeof(int);
			break;
		}
		case 'f':
		case 'd':
		{
			double d = atof(values[argindex].c_str());
			std::memcpy(&va[pindex], &d, sizeof(double));
			pindex += sizeof(double);
			break;
		}
		}
	}
	PyObject *result = Py_VaBuildValue(format, reinterpret_cast<va_list>(va));
	// Delete the argument pointer.
	delete[] va;
	//agk::PluginError(_PyUnicode_AsString(PyObject_Repr(result)));
	return GetHandle(result);
}

//https://docs.python.org/3/c-api/object.html
int _PyObject_HasAttr(int hobject, int hattr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *attr_name = GetPyObject(hattr_name);
	return PyObject_HasAttr(object, attr_name);
}

int _PyObject_HasAttrString(int hobject, const char *attr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	return PyObject_HasAttrString(object, attr_name);
}

int _PyObject_GetAttrHandle(int hobject, int hattr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *attr_name = GetPyObject(hattr_name);
	return GetHandle(PyObject_GetAttr(object, attr_name));
}

int _PyObject_GetAttrHandleS(int hobject, const char *attr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	return GetHandle(PyObject_GetAttrString(object, attr_name));
}

float _PyObject_GetAttrFloat(int hobject, const char *attr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	return (float) PyFloat_AsDouble(PyObject_GetAttrString(object, attr_name));
}

int _PyObject_GetAttrInt(int hobject, const char *attr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	return PyLong_AsLong(PyObject_GetAttrString(object, attr_name));
}

const char *_PyObject_GetAttrString(int hobject, const char *attr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	return CreateString(PyObject_GetAttrString(object, attr_name));
}

int _PyObject_SetAttrHandle(int hobject, int hattr_name, int hvalue)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *attr_name = GetPyObject(hattr_name);
	PyObject *value = GetPyObject(hvalue);
	return PyObject_SetAttr(object, attr_name, value);
}

int _PyObject_SetAttrHandleS(int hobject, const char *attr_name, int hvalue)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *value = GetPyObject(hvalue);
	return PyObject_SetAttrString(object, attr_name, value);
}

int _PyObject_SetAttrFloat(int hobject, const char *attr_name, float value)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *v = PyFloat_FromDouble(value);
	int result = PyObject_SetAttrString(object, attr_name, v);
	Py_DecRef(v);
	return result;
}

int _PyObject_SetAttrInt(int hobject, const char *attr_name, int value)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *v = PyLong_FromLong(value);
	int result = PyObject_SetAttrString(object, attr_name, v);
	Py_DecRef(v);
	return result;
}

int _PyObject_SetAttrString(int hobject, const char *attr_name, const char *value)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *v = PyUnicode_FromString(value);
	int result = PyObject_SetAttrString(object, attr_name, v);
	Py_DecRef(v);
	return result;
}

int _PyObject_DelAttr(int hobject, int hattr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *attr_name = GetPyObject(hattr_name);
	return PyObject_DelAttr(object, attr_name);
}

int _PyObject_DelAttrString(int hobject, const char *attr_name)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	return PyObject_DelAttrString(object, attr_name);
}

int _PyObject_ReprObj(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return GetHandle(PyObject_Repr(object));
}

const char *_PyObject_Repr(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	PyObject *repr = PyObject_Repr(object);
	if (repr == NULL)
	{
		return CreateString((const char *)NULL);
	}
	char *result = CreateString(_PyUnicode_AsString(repr));
	Py_DecRef(repr);
	return result;
}

int _PyObject_StrObj(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return GetHandle(PyObject_Str(object));
}

const char *_PyObject_Str(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	PyObject *str = PyObject_Str(object);
	if (str == NULL)
	{
		return CreateString((const char *)NULL);
	}
	char *result = CreateString(_PyUnicode_AsString(str));
	Py_DecRef(str);
	return result;
}

int _PyCallable_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyCallable_Check(object);
}

int _PyObject_Call(int hcallable_object, int hargs, int hkw)
{
	REQUIRED_HANDLE(hcallable_object)
	// If no named arguments are needed, kw may be NULL. args must not be NULL, use an empty tuple if no arguments are needed.
	PyObject *callable_object = GetPyObject(hcallable_object);
	PyObject *args = (hargs) ? GetPyObject(hargs) : PyTuple_New(0);
	PyObject *kw = GetPyObject(hkw);
	PyObject *result = PyObject_Call(callable_object, args, kw);
	CheckError();
	// DECREF any created tuple.
	if (!hargs)
	{
		Py_DecRef(args);
	}
	return GetHandle(result);
}

int _PyObject_Length(int hobject)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	return PyObject_Length(object);
}

int _PyObject_GetItem(int hobject, int hkey)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *key = GetPyObject(hkey);
	return GetHandle(PyObject_GetItem(object, key));
}

int _PyObject_SetItem(int hobject, int hkey, int hvalue)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *key = GetPyObject(hkey);
	PyObject *value = GetPyObject(hvalue);
	return PyObject_SetItem(object, key, value);
}

int _PyObject_DelItem(int hobject, int hkey)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	PyObject *key = GetPyObject(hkey);
	return PyObject_DelItem(object, key);
}

int _PyObject_GetIter(int hobject)
{
	REQUIRED_HANDLE(hobject)
	PyObject *object = GetPyObject(hobject);
	return GetHandle(PyObject_GetIter(object));
}

/*
https://docs.python.org/3/c-api/long.html
*/
int _PyLong_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyLong_Check(object);
}

int _PyLong_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyLong_CheckExact(object);
}

int _PyLong_FromLong(int value)
{
	PyObject *object = PyLong_FromLong(value);
	return GetHandle(object);
}

int _PyLong_AsLong(int hlong)
{
	PyObject *object = GetPyObject(hlong);
	return PyLong_AsLong(object);
}

/*
https://docs.python.org/3/c-api/float.html
*/
int _PyFloat_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyFloat_Check(object);
}

int _PyFloat_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyFloat_CheckExact(object);
}

int _PyFloat_FromDouble(float value)
{
	PyObject *object = PyFloat_FromDouble(value);
	return GetHandle(object);
}

float _PyFloat_AsDouble(int hdouble)
{
	PyObject *object = GetPyObject(hdouble);
	return (float)PyFloat_AsDouble(object);
}

//https://docs.python.org/3/c-api/unicode.html
int _PyUnicode_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyUnicode_Check(object);
}

int _PyUnicode_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyUnicode_CheckExact(object);
}

int _PyUnicode_FromString(const char *value)
{
	PyObject *object = PyUnicode_FromString(value);
	return GetHandle(object);
}

// Note: Can't name this _PyUnicode_AsString because that's a macro that exists in the Python header.
const char *_PyUnicode_AsStringPL(int hunicode)
{
	PyObject *object = GetPyObject(hunicode);
	return CreateString(object);
}

/*
https://docs.python.org/3/c-api/tuple.html
*/
int _PyTuple_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyTuple_Check(object);
}

int _PyTuple_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyTuple_CheckExact(object);
}

int _PyTuple_New(int size)
{
	return GetHandle(PyTuple_New(size));
}

//int _PyTuple_Pack(int size, ...)

int _PyTuple_Size(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyTuple_Size(object);
}

int _PyTuple_GetItemHandle(int hobject, int pos)
{
	PyObject *object = GetPyObject(hobject);
	return GetHandle(PyTuple_GetItem(object, pos));
}

float _PyTuple_GetItemFloat(int hobject, int pos)
{
	PyObject *object = GetPyObject(hobject);
	return (float) PyFloat_AsDouble(PyTuple_GetItem(object, pos));
}

int _PyTuple_GetItemInt(int hobject, int pos)
{
	PyObject *object = GetPyObject(hobject);
	return PyLong_AsLong(PyTuple_GetItem(object, pos));
}

const char *_PyTuple_GetItemString(int hobject, int pos)
{
	PyObject *object = GetPyObject(hobject);
	return CreateString(PyTuple_GetItem(object, pos));
}

int _PyTuple_GetSlice(int hobject, int low, int high)
{
	PyObject *object = GetPyObject(hobject);
	return GetHandle(PyTuple_GetSlice(object, low, high));
}

int _PyTuple_SetItemHandle(int hobject, int pos, int hvalue)
{
	PyObject *object = GetPyObject(hobject);
	PyObject *value = GetPyObject(hvalue);
	return PyTuple_SetItem(object, pos, value);
}

int _PyTuple_SetItemFloat(int hobject, int pos, float value)
{
	PyObject *object = GetPyObject(hobject);
	// PyTuple_SetItem steals the value reference!
	return PyTuple_SetItem(object, pos, PyFloat_FromDouble(value));
}

int _PyTuple_SetItemInt(int hobject, int pos, int value)
{
	PyObject *object = GetPyObject(hobject);
	// PyTuple_SetItem steals the value reference!
	return PyTuple_SetItem(object, pos, PyLong_FromLong(value));
}

int _PyTuple_SetItemString(int hobject, int pos, const char *value)
{
	PyObject *object = GetPyObject(hobject);
	// PyTuple_SetItem steals the value reference!
	return PyTuple_SetItem(object, pos, PyUnicode_FromString(value));
}

/*
https://docs.python.org/3/c-api/list.html
*/
int _PyList_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyList_Check(object);
}

int _PyList_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyList_CheckExact(object);
}

int _PyList_New(int size)
{
	return GetHandle(PyList_New(size));
}

int _PyList_Size(int hlist)
{
	PyObject *list = GetPyObject(hlist);
	return PyList_Size(list);
}

int _PyList_GetItemHandle(int hlist, int index)
{
	PyObject *list = GetPyObject(hlist);
	return GetHandle(PyList_GetItem(list, index));
}

float _PyList_GetItemFloat(int hlist, int index)
{
	PyObject *list = GetPyObject(hlist);
	return (float)PyFloat_AsDouble(PyList_GetItem(list, index));
}

int _PyList_GetItemInt(int hlist, int index)
{
	PyObject *list = GetPyObject(hlist);
	return PyLong_AsLong(PyList_GetItem(list, index));
}

const char *_PyList_GetItemString(int hlist, int index)
{
	PyObject *list = GetPyObject(hlist);
	return CreateString(PyList_GetItem(list, index));
}

int _PyList_SetItemHandle(int hlist, int index, int hitem)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *item = GetPyObject(hitem);
	return PyList_SetItem(list, index, item);
}

int _PyList_SetItemFloat(int hlist, int index, float value)
{
	PyObject *list = GetPyObject(hlist);
	// PyList_SetItem steals the value reference!
	return PyList_SetItem(list, index, PyFloat_FromDouble(value));
}

int _PyList_SetItemInt(int hlist, int index, int value)
{
	PyObject *list = GetPyObject(hlist);
	// PyList_SetItem steals the value reference!
	return PyList_SetItem(list, index, PyLong_FromLong(value));
}

int _PyList_SetItemString(int hlist, int index, const char *value)
{
	PyObject *list = GetPyObject(hlist);
	// PyList_SetItem steals the value reference!
	return PyList_SetItem(list, index, PyUnicode_FromString(value));
}

int _PyList_InsertHandle(int hlist, int index, int hitem)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *item = GetPyObject(hitem);
	return PyList_Insert(list, index, item);
}

int _PyList_InsertFloat(int hlist, int index, float value)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *v = PyFloat_FromDouble(value);
	int result = PyList_Insert(list, index, v);
	Py_DecRef(v);
	return result;
}

int _PyList_InsertInt(int hlist, int index, int value)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *v = PyLong_FromLong(value);
	int result = PyList_Insert(list, index, v);
	Py_DecRef(v);
	return result;
}

int _PyList_InsertString(int hlist, int index, const char *value)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *v = PyUnicode_FromString(value);
	int result = PyList_Insert(list, index, v);
	Py_DecRef(v);
	return result;
}

int _PyList_AppendHandle(int hlist, int hitem)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *item = GetPyObject(hitem);
	return PyList_Append(list, item);
}

int _PyList_AppendFloat(int hlist, float value)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *v = PyFloat_FromDouble(value);
	int result = PyList_Append(list, v);
	Py_DecRef(v);
	return result;
}

int _PyList_AppendInt(int hlist, int value)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *v = PyLong_FromLong(value);
	int result = PyList_Append(list, v);
	Py_DecRef(v);
	return result;
}

int _PyList_AppendString(int hlist, const char *value)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *v = PyUnicode_FromString(value);
	int result = PyList_Append(list, v);
	Py_DecRef(v);
	return result;
}

int _PyList_GetSlice(int hlist, int low, int high)
{
	PyObject *list = GetPyObject(hlist);
	return GetHandle(PyList_GetSlice(list, low, high));
}

int _PyList_SetSlice(int hlist, int low, int high, int hitemlist)
{
	PyObject *list = GetPyObject(hlist);
	PyObject *itemlist = GetPyObject(hitemlist);
	return PyList_SetSlice(list, low, high, itemlist);
}

int _PyList_Sort(int hlist)
{
	PyObject *list = GetPyObject(hlist);
	return PyList_Sort(list);
}

int _PyList_Reverse(int hlist)
{
	PyObject *list = GetPyObject(hlist);
	return PyList_Reverse(list);
}

int _PyList_AsTuple(int hlist)
{
	PyObject *list = GetPyObject(hlist);
	return GetHandle(PyList_AsTuple(list));
}

/*
https://docs.python.org/3/c-api/dict.html
*/
int _PyDict_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyDict_Check(object);
}

int _PyDict_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyDict_CheckExact(object);
}

int _PyDict_New()
{
	return GetHandle(PyDict_New());
}

void _PyDict_Clear(int hdict)
{
	PyObject *dict = GetPyObject(hdict);
	PyDict_Clear(dict);
}

// Note: Can't name this _PyDict_Contains because that's a macro that exists in the Python header.
int _PyDict_ContainsKey(int hdict, int hkey)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *key = GetPyObject(hkey);
	return PyDict_Contains(dict, key);
}

int _PyDict_ContainsKeyS(int hdict, const char *key)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *k = PyUnicode_FromString(key);
	int result = PyDict_Contains(dict, k);
	Py_DecRef(k);
	return result;
}

int _PyDict_Copy(int hdict)
{
	PyObject *object = GetPyObject(hdict);
	return GetHandle(PyDict_Copy(object));
}

int _PyDict_SetItemHandle(int hdict, int hkey, int hvalue)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *key = GetPyObject(hkey);
	PyObject *value = GetPyObject(hvalue);
	return PyDict_SetItem(dict, key, value);
}

int _PyDict_SetItemHandleS(int hdict, const char *key, int hvalue)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *value = GetPyObject(hvalue);
	return PyDict_SetItemString(dict, key, value);
}

int _PyDict_SetItemFloat(int hdict, const char *key, float value)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *v = PyFloat_FromDouble(value);
	int result = PyDict_SetItemString(dict, key, v);
	Py_DecRef(v);
	return result;
}

int _PyDict_SetItemInt(int hdict, const char *key, int value)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *v = PyLong_FromLong(value);
	int result = PyDict_SetItemString(dict, key, v);
	Py_DecRef(v);
	return result;
}

int _PyDict_SetItemString(int hdict, const char *key, const char *value)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *v = PyUnicode_FromString(value);
	int result = PyDict_SetItemString(dict, key, v);
	Py_DecRef(v);
	return result;
}

int _PyDict_DelItem(int hdict, int hkey)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *key = GetPyObject(hkey);
	return PyDict_DelItem(dict, key);
}

int _PyDict_DelItemString(int hdict, const char *key)
{
	PyObject *dict = GetPyObject(hdict);
	return PyDict_DelItemString(dict, key);
}

// Borrowed ref
int _PyDict_GetItemHandle(int hdict, int hkey)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *key = GetPyObject(hkey);
	return GetHandle(PyDict_GetItem(dict, key));
}

// Borrowed ref
int _PyDict_GetItemHandleS(int hdict, const char *key)
{
	PyObject *dict = GetPyObject(hdict);
	return GetHandle(PyDict_GetItemString(dict, key));
}

float _PyDict_GetItemFloat(int hdict, const char *key)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *item = PyDict_GetItemString(dict, key);
	if (item == NULL)
	{
		return 0.0;
	}
	return (float)PyFloat_AsDouble(item);
}

int _PyDict_GetItemInt(int hdict, const char *key)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *item = PyDict_GetItemString(dict, key);
	if (item == NULL)
	{
		return 0;
	}
	return PyLong_AsLong(item);
}

const char *_PyDict_GetItemString(int hdict, const char *key)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *item = PyDict_GetItemString(dict, key);
	if (item == NULL)
	{
		return CreateString((const char *)NULL);
	}
	return CreateString(item);
}

//int _PyDict_GetItemWithError(int hdict, int hkey);

int _PyDict_SetDefault(int hdict, int hkey, int hdefault)
{
	PyObject *dict = GetPyObject(hdict);
	PyObject *key = GetPyObject(hkey);
	PyObject *default = GetPyObject(hdefault);
	return GetHandle(PyDict_SetDefault(dict, key, default));
}

int _PyDict_Items(int hdict)
{
	PyObject *dict = GetPyObject(hdict);
	return GetHandle(PyDict_Items(dict));
}

int _PyDict_Keys(int hdict)
{
	PyObject *dict = GetPyObject(hdict);
	return GetHandle(PyDict_Keys(dict));
}

int _PyDict_Values(int hdict)
{
	PyObject *dict = GetPyObject(hdict);
	return GetHandle(PyDict_Values(dict));
}

int _PyDict_Size(int hdict)
{
	PyObject *dict = GetPyObject(hdict);
	return PyDict_Size(dict);
}

int _PyDict_Merge(int hdicta, int hdictb, int override)
{
	PyObject *dicta = GetPyObject(hdicta);
	PyObject *dictb = GetPyObject(hdictb);
	return PyDict_Merge(dicta, dictb, override);
}

int _PyDict_Update(int hdicta, int hdictb)
{
	PyObject *dicta = GetPyObject(hdicta);
	PyObject *dictb = GetPyObject(hdictb);
	return PyDict_Update(dicta, dictb);
}

/*
https://docs.python.org/3/c-api/set.html
*/
int _PySet_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PySet_Check(object);
}

int _PyFrozenSet_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyFrozenSet_Check(object);
}

int _PyAnySet_Check(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyAnySet_Check(object);
}

int _PyAnySet_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyAnySet_CheckExact(object);
}

int _PyFrozenSet_CheckExact(int hobject)
{
	PyObject *object = GetPyObject(hobject);
	return PyFrozenSet_CheckExact(object);
}

int _PySet_New(int hiterable)
{
	PyObject *iterable = GetPyObject(hiterable);
	return GetHandle(PySet_New(iterable));
}

int _PyFrozenSet_New(int hiterable)
{
	PyObject *iterable = GetPyObject(hiterable);
	return GetHandle(PyFrozenSet_New(iterable));
}

int _PySet_Size(int hset)
{
	PyObject *set = GetPyObject(hset);
	return PySet_Size(set);
}

int _PySet_Contains(int hset, int hkey)
{
	PyObject *set = GetPyObject(hset);
	PyObject *key = GetPyObject(hkey);
	return PySet_Contains(set, key);
}

int _PySet_Add(int hset, int hkey)
{
	PyObject *set = GetPyObject(hset);
	PyObject *key = GetPyObject(hkey);
	return PySet_Add(set, key);
}

int _PySet_Discard(int hset, int hkey)
{
	PyObject *set = GetPyObject(hset);
	PyObject *key = GetPyObject(hkey);
	return PySet_Discard(set, key);
}

int _PySet_Pop(int hset)
{
	PyObject *set = GetPyObject(hset);
	return GetHandle(PySet_Pop(set));
}

int _PySet_Clear(int hset)
{
	PyObject *set = GetPyObject(hset);
	return PySet_Clear(set);
}
