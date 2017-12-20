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

// Force use of the release build of python36.dll.
#include <string>

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#ifdef PLUGIN
#include "..\AGKLibraryCommands.h"
#else
#include "agk.h"
#endif

void CheckError()
{
	PyObject *err = PyErr_Occurred();
	if (err)
	{
		std::string msg = "";
		PyObject *type, *value, *traceback;
		PyErr_Fetch(&type, &value, &traceback);
		PyErr_NormalizeException(&type, &value, &traceback);
		// Get the exception type string.
		PyObject *temp;
		temp = PyObject_Str(type);
		if (temp != NULL)
		{
			msg += _PyUnicode_AsString(temp);
			msg += "\n";
			Py_DECREF(temp);
		}
		// Get the exception value string.
		temp = PyObject_Str(value);
		if (temp != NULL) {
			msg += _PyUnicode_AsString(temp);
			Py_DECREF(temp);
		}
		//msg += "\n";
		// Import the traceback.
		if (traceback != NULL)
		{
			temp = PyUnicode_FromString("traceback");
			PyObject *module = PyImport_Import(temp);
			Py_DECREF(temp);
			if (module != NULL)
			{
				PyObject *dict = PyModule_GetDict(module); // borrowed ref
				PyObject *func = PyDict_GetItemString(dict, "format_tb"); // borrowed ref
				if (func && PyCallable_Check(func))
				{
					PyObject *args = PyTuple_New(1);
					PyTuple_SetItem(args, 0, traceback);
					PyObject *result = PyObject_CallObject(func, args);
					if (result != NULL)
					{
						int len = PyList_Size(result);
						char *buffer;
						for (int i = 0; i < len; i++)
						{
							PyObject *trace = Py_BuildValue("(O)", PyList_GetItem(result, i));
							if (PyArg_ParseTuple(trace, "s", &buffer))
							{
								msg += "\n";
								msg += buffer;
								//msg += "\n";
							}
							Py_DECREF(trace);
						}
						Py_DECREF(result);
					}
					Py_DECREF(args);
				}
				Py_DECREF(module);
			}
			Py_DECREF(traceback);
		}
		// PyErr_Restore causes the AGK host to crash.
		//PyErr_Restore(type, value, traceback);
		// DECREF instead.
		if (type != NULL)
		{
			Py_DECREF(type);

		}
		if (value != NULL)
		{
			Py_DECREF(value);

		}
		//MessageBoxA(NULL, msg.c_str(), "Error", MB_OK);
		agk::PluginError(msg.c_str());
	}
}