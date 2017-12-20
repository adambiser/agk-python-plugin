// Project: AGKPythonPluginTest
// Created: 2017-11-14
// Copyright 2017 Adam Biser
#option_explicit

#import_plugin PythonPlugin As Py

#constant KEY_ESCAPE	27
#constant NEWLINE		CHR(10)
#constant WINDOW_WIDTH	1024
#constant WINDOW_HEIGHT	768
#constant STATUS_X		0
#constant STATUS_Y		200
#constant STATUS_WIDTH	1024
#constant STATUS_HEIGHT	568

// Set up window and display.
SetWindowTitle("Python 3 Example")
SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT, 0)
SetVirtualResolution(WINDOW_WIDTH, WINDOW_HEIGHT)
SetSyncRate(30, 0)
UseNewDefaultFonts(1)
SetPrintSize(20)
SetErrorMode(2) // show all errors

CreateStatusArea()

// Virtual buttons
#constant CREATE_INTEGER_BUTTON	1
#constant CHANGE_NAME_BUTTON	2
#constant RUN_FILE_BUTTON		3
#constant BUILD_VALUE_BUTTON	4
#constant SIMPLE_STRING_BUTTON	5
#constant CALL_FUNCTION_BUTTON	6
#constant CAUSE_ERROR_BUTTON	7

global buttonText as string[6] = ["Create Int", "Change_Name", "Run File", "Build_Value", "Simple_String", "Call_Function", "Cause_Error"]
x as integer
for x = 0 to buttonText.length
	CreateButton(x + 1, 50 + x * 100, 50, ReplaceString(buttonText[x], "_", NEWLINE, -1))
next


Py.Py_SetProgramName("UsePythonFromAGK")
Py.Py_SetPythonHome("media") // Does not affect file paths, just imports.
Py.Py_Initialize()

// Show general information
AddStatus("Py_GetProgramName: " + Py.Py_GetProgramName())
AddStatus("Py_IsInitialized: " + TF(Py.Py_IsInitialized()))
AddStatus("Py_GetVersion: " + Py.Py_GetVersion())
AddStatus("Py_GetProgramFullPath: " + Py.Py_GetProgramFullPath())
AddStatus("Py_GetPythonHome: " + Py.Py_GetPythonHome())
//~ AddStatus("Py_GetCopyright: " + Py.Py_GetCopyright())

//
// The main loop
//
do
	Sync()
	CheckMouseWheel()
	CheckInput()
	if GetRawKeyPressed(KEY_ESCAPE) 
		exit
	endif
loop

Py.Py_Finalize()
Quit()

Function CheckInput()
	if GetVirtualButtonPressed(CREATE_INTEGER_BUTTON)
		AddStatus("---------------------------")
		CreateInteger()
	endif
	if GetVirtualButtonPressed(CHANGE_NAME_BUTTON)
		AddStatus("---------------------------")
		ChangeName()
	endif
	if GetVirtualButtonPressed(RUN_FILE_BUTTON)
		AddStatus("---------------------------")
		RunFile()
	endif
	if GetVirtualButtonPressed(BUILD_VALUE_BUTTON)
		AddStatus("---------------------------")
		BuildValue()
	endif
	if GetVirtualButtonPressed(SIMPLE_STRING_BUTTON)
		AddStatus("---------------------------")
		RunSimpleString()
	endif
	if GetVirtualButtonPressed(CALL_FUNCTION_BUTTON)
		AddStatus("---------------------------")
		CallPythonFunction()
	endif
	if GetVirtualButtonPressed(CAUSE_ERROR_BUTTON)
		AddStatus("---------------------------")
		CausePythonError()
	endif
EndFunction

//
// Creates a PyLong object that is initialized to 500, prints its value, then destroys the PyObject.
//
Function CreateInteger()
	handle as integer
	handle = Py.PyLong_FromLong(500)
	AddStatus("PyLong_AsLong: Handle " + str(handle) + " = " + str(Py.PyLong_AsLong(handle)))
	// Remember to DECREF PyObjects that you create!
	Py.Py_DECREF(handle)
	AddStatus("Py_REFCNT: " + str(Py.Py_REFCNT(handle)))
EndFunction

//
// Creates a PyUnicode "name" object that starts as one value and is then changed by running a script string.
//
Function ChangeName()
	// Set up a dictionary of local values.
	hLocals as integer
	hLocals = Py.PyDict_New()
	hNameValue as integer
	hNameValue = Py.PyUnicode_FromString("Alex")
	Py.PyDict_SetItemHandle(hLocals, "name", hNameValue)
	AddStatus("hLocals: " + str(hLocals) + ", type: " + Py.Py_TYPE_NAME(hLocals))
	AddStatus("hNameValue: " + str(hNameValue) + ", type: " + Py.Py_TYPE_NAME(hNameValue))
	AddStatus("Name Before: " + Py.PyDict_GetItemString(hLocals, "name"))
	// Run a simple script from a string to set the name.
	script as string
	script = "name = 'Bob'"
	hResult as integer
	hResult = Py.PyRun_String(script, 0, hLocals)
	AddStatus("hResult: " + str(hResult) + ", type: " + Py.Py_TYPE_NAME(hResult))
	AddStatus("Name After: " + Py.PyDict_GetItemString(hLocals, "name"))
	// Remember to DECREF PyObjects that you create!
	Py.Py_DECREF(hResult)
	Py.Py_DECREF(hLocals)
	Py.Py_DECREF(hNameValue)
	AddStatus("Py_REFCNT hLocals: " + str(Py.Py_REFCNT(hLocals)))
	AddStatus("Py_REFCNT hNameValue: " + str(Py.Py_REFCNT(hNameValue)))
	AddStatus("Py_REFCNT hResult: " + str(Py.Py_REFCNT(hResult)))
EndFunction

//
// Runs a file passing in a "name" and reading the "result" set by the script.
//
Function RunFile()
	hLocals as integer
	hLocals = Py.PyDict_New()
	hNameValue as integer
	hNameValue = Py.PyUnicode_FromString("Alex")
	Py.PyDict_SetItemHandle(hLocals, "name", hNameValue)
	AddStatus("hLocals: " + str(hLocals) + ", type: " + Py.Py_TYPE_NAME(hLocals))
	AddStatus("hNameValue: " + str(hNameValue) + ", type: " + Py.Py_TYPE_NAME(hNameValue))
	// Run a simple script file to get a result.
	// Since the file uses import, this needs to use GetMainModuleDict to get the __main__ dict and use it as globals.
	hResult as integer
	hResult = Py.PyRun_File("media/run.py", Py.GetMainModuleDict(), hLocals)
	AddStatus("hResult: " + str(hResult) + ", type: " + Py.Py_TYPE_NAME(hResult))
	AddStatus("Result text: " + Py.PyDict_GetItemString(hLocals, "result"))
	// Remember to DECREF PyObjects that you create!
	Py.Py_DECREF(hResult)
	Py.Py_DECREF(hLocals)
	Py.Py_DECREF(hNameValue)
	AddStatus("Py_REFCNT hLocals: " + str(Py.Py_REFCNT(hLocals)))
	AddStatus("Py_REFCNT hNameValue: " + str(Py.Py_REFCNT(hNameValue)))
	AddStatus("Py_REFCNT hResult: " + str(Py.Py_REFCNT(hResult)))
EndFunction

//
// Build a PyObject value from a format string and csv string.
// NOTE: This is limited to strings, integers, and floats.
//
Function BuildValue()
	AddStatus("Creating a tuple using Py_BuildValue")
	hValue as integer
	hValue = Py.Py_BuildValue("ssiif", "hello,'test,this',2,365,2.5886")
	AddStatus("Py_BuildValue handle: " + str(hValue))
	AddStatus("Py_BuildValue repr: " + Py.PyObject_Repr(hValue))
	x as integer
	for x = 0 to Py.PyTuple_Size(hValue) - 1
		hItem as integer
		hItem = Py.PyTuple_GetItemHandle(hValue, x) // This returns a BORROWED ref.
		AddStatus("Item " + str(x) + " handle: " + str(hItem) + ", Py_REFCNT: " + str(Py.Py_REFCNT(hItem)) + ", type : " +  Py.Py_TYPE_NAME(hItem) + ", repr: " + Py.PyObject_Repr(hItem))
	next
	// Remember to DECREF PyObjects that you create!
	Py.Py_DECREF(hValue)
	AddStatus("Py_REFCNT hValue: " + str(Py.Py_REFCNT(hValue)))
EndFunction

//
// Another example of running a script from a string, this time using PyRun_SimpleString.
// Can't do any interaction when using PyRun_SimpleString.  Use PyRun_String.
//
Function RunSimpleString()
	script as string
	script = script + "from time import time,ctime" + NEWLINE
	script = script + "result = 'The time is {}, and here\'s a number: {}'.format(ctime(time()), 2.5)"
	AddStatus("Script:" + NEWLINE + script)
	// Return: 0 = Success, -1 = Failure
	AddStatus("PyRun_SimpleString: " + str(Py.PyRun_SimpleString(script)))
EndFunction

//
// How to call a function in a module, pass it some parameters, and get its return value.
//
Function CallPythonFunction()
	// Don't need "media." because of Py_SetPythonHome call above.
	hModule as integer
	hModule = Py.PyImport_Import("module")
	AddStatus("PyImport_Import hModule: " + str(hModule) + ", type: " + Py.Py_TYPE_NAME(hModule))
	if hModule
		hFunc as integer
		hFunc = Py.PyObject_GetAttrHandle(hModule, "test")
		AddStatus("PyObject_GetAttrHandle hFunc: " + str(hFunc) + ", type: " + Py.Py_TYPE_NAME(hFunc))
		if hFunc
			hArgs as integer
			hArgs = Py.Py_BuildValue("si", "Alex,100")
			AddStatus("Py_BuildValue hArgs: " + str(hArgs) + ", type: " + Py.Py_TYPE_NAME(hArgs))
			if hArgs
				AddStatus("Arguments: " + Py.PyObject_Repr(hArgs))
				hResult as integer
				hResult = Py.PyObject_Call(hFunc, hArgs, 0)
				AddStatus("PyObject_Call hResult: " + str(hResult) + ", type: " + Py.Py_TYPE_NAME(hResult))
				if hResult
					AddStatus("RESULT: " + Py.PyUnicode_AsString(hResult))
					Py.Py_DECREF(hResult)
				endif
				Py.Py_DECREF(hArgs)
			endif
			Py.Py_DECREF(hFunc)
		endif
		Py.Py_DECREF(hModule)
	endif
	AddStatus("Py_REFCNT hModule: " + str(Py.Py_REFCNT(hModule)))
	AddStatus("Py_REFCNT hFunc: " + str(Py.Py_REFCNT(hFunc)))
	AddStatus("Py_REFCNT hArgs: " + str(Py.Py_REFCNT(hArgs)))
	AddStatus("Py_REFCNT hResult: " + str(Py.Py_REFCNT(hResult)))
EndFunction

//
// Shows how Python errors are reported.
//
Function CausePythonError()
	// This causes an error because "name" is not defined.
	script as string
	script = "response = 'This is going to cause a problem, {}.'.format(name)"
	hResult as integer
	hResult = Py.PyRun_String(script, 0, 0)
	if GetErrorOccurred()
		AddStatus("ERROR:" + NEWLINE + GetLastError())
	endif
	AddStatus("hResult: " + str(hResult) + ", type: " + Py.Py_TYPE_NAME(hResult))
	// Remember to DECREF PyObjects that you create!
	if hResult
		Py.Py_DECREF(hResult)
	endif
	AddStatus("Py_REFCNT hResult: " + str(Py.Py_REFCNT(hResult)))
EndFunction

//---------------------------------------------------------------------
//
// Keep the UI stuff separate to highligh the plugin code.
//
// The type holds IDs for all text controls.
global text as TextControls
Type TextControls
	status as integer
EndType

Function CreateStatusArea()
	CreateTextEx(STATUS_X, STATUS_Y - 25, "Status Messages (Scrollable)")
	spriteID as integer
	spriteID = CreateSprite(CreateImageColor(32, 32, 32, 255))
	SetSpritePosition(spriteID, STATUS_X, STATUS_Y)
	SetSpriteSize(spriteID, STATUS_WIDTH, STATUS_HEIGHT)
	// Set up scrolling status text
	text.status = CreateText("")
	SetTextPosition(text.status, STATUS_X, STATUS_Y)
	SetTextScissor(text.status, STATUS_X, STATUS_Y, STATUS_X + STATUS_WIDTH, STATUS_Y + STATUS_HEIGHT)
	SetTextSize(text.status, 18)
	SetTextVisible(text.status, 1)
EndFunction

Function CreateTextEx(x as float, y as float, text as string)
	id as integer
	id = CreateText(text)
	SetTextPosition(id, x, y)
	SetTextSize(id, 20)
EndFunction id

Function CreateButton(id as integer, x as integer, y as integer, txt as string)
	AddVirtualButton(id, x, y, 80)
	//~ SetVirtualButtonSize(id, 150, 80)
	SetVirtualButtonText(id, txt)
	//~ SetButtonEnabled(id, 0)
EndFunction

Function SetButtonEnabled(id as integer, enabled as integer)
	SetVirtualButtonActive(id, enabled)
	if enabled
		SetVirtualButtonColor(id, 192, 192, 192)
	else
		SetVirtualButtonColor(id, 64, 64, 64)
	endif
EndFunction	

Function GetDateFromUnix(unix as integer)
	text as string
	text = PadStr(GetYearFromUnix(unix), 4)
	text = text + "-" + PadStr(GetMonthFromUnix(unix), 2)
	text = text + "-" + PadStr(GetDaysFromUnix(unix), 2)
	text = text + " " + PadStr(GetHoursFromUnix(unix), 2)
	text = text + ":" + PadStr(GetMinutesFromUnix(unix), 2)
	text = text + ":" + PadStr(GetSecondsFromUnix(unix), 2)
EndFunction text

Function PadStr(number as integer, length as integer)
	text as string
	text = str(number)
	if len(text) < length
		text = ReplaceString(Spaces(length - len(text)), " ", "0", -1) + text
	endif
EndFunction text

Function AddStatus(status as string)
	SetTextString(text.status, GetTextString(text.status) + GetDateFromUnix(GetUnixTime()) + ": " + status + NEWLINE)
	// Scroll the text upward.
	height as float
	height = GetTextTotalHeight(text.status)
	if height < STATUS_HEIGHT
		height = STATUS_HEIGHT
	endif
	SetTextPosition(text.status, STATUS_X, STATUS_Y - (height - STATUS_HEIGHT))
EndFunction

// Scrollable status window.
Function CheckMouseWheel()
	delta as float
	delta = GetRawMouseWheelDelta()
	if delta <> 0
		mouseX as float
		mouseY as float
		mouseX = GetPointerX()
		mouseY = GetPointerY()
		if mouseX < STATUS_X or mouseY < STATUS_Y or mouseX >= STATUS_X + STATUS_WIDTH or mouseY >= STATUS_Y + STATUS_HEIGHT
			ExitFunction
		endif
		newY as float
		newY = GetTextY(text.status) + GetTextSize(text.status) * delta
		if newY >= STATUS_Y //WINDOW_HEIGHT
			newY = STATUS_Y
		else
			height as float
			height = GetTextTotalHeight(text.status)
			if height < STATUS_HEIGHT
				height = STATUS_HEIGHT
			endif
			//~ if newY - GetTextSize(text.status) + height < STATUS_Y
			if newY + height < WINDOW_HEIGHT
				newY = WINDOW_HEIGHT - height
			endif
		endif
		SetTextY(text.status, newY)
	endif
EndFunction


Function TF(value as integer)
	if value
		ExitFunction "TRUE"
	endif
EndFunction "FALSE"

Function Quit()
	// Cleanup
	x as integer
	for x = 1 to 100
		DeleteVirtualButton(x)
	next
	DeleteAllText()
	DeleteAllImages()
	DeleteAllSprites()
	end
EndFunction
