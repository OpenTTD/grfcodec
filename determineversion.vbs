Option Explicit

' Customized version of a file, which was originally part of OpenTTD.
' OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
' OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
' See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.

Dim FSO
Set FSO = CreateObject("Scripting.FileSystemObject")

Function LoadVersionNumber(filename)
	Dim file, data, exp, m
	Set file = FSO.OpenTextFile(filename, 1, 0, 0)
	data = file.ReadLine ' skip
	data = file.ReadLine
	file.Close
	LoadVersionNumber = Mid(data, InStr(data, "=") + 2)
End Function

Sub FindReplaceInFile(filename, to_find, replacement)
	Dim file, data
	Set file = FSO.OpenTextFile(filename, 1, 0, 0)
	data = file.ReadAll
	file.Close
	data = Replace(data, to_find, replacement)
	Set file = FSO.CreateTextFile(filename, -1, 0)
	file.Write data
	file.Close
End Sub

Sub UpdateFile(modified, revision, version, cur_date, version_number, filename)
	FSO.CopyFile filename & ".in", filename
	FindReplaceInFile filename, "!!MODIFIED!!", modified
	FindReplaceInFile filename, "!!REVISION!!", revision
	FindReplaceInFile filename, "!!VERSION!!", version
	FindReplaceInFile filename, "!!DATE!!", cur_date
	FindReplaceInFile filename, "!!VERSION_NUMBER!!", version_number
End Sub

Sub UpdateFiles(version, version_number)
	Dim modified, revision, cur_date
	cur_date = DatePart("D", Date) & "." & DatePart("M", Date) & "." & DatePart("YYYY", Date)

	If InStr(version, Chr(9)) Then
		revision = Mid(version, InStr(version, Chr(9)) + 1)
		revision = Mid(revision, 1, InStr(revision, Chr(9)) - 1)
		modified = Mid(version, InStrRev(version, Chr(9)) + 1)
		version  = Mid(version, 1, InStr(version, Chr(9)) - 1)
	Else
		revision = 0
		modified = 1
	End If

	UpdateFile modified, revision, version, cur_date, version_number, "./version.h"
End Sub

Function ReadRegistryKey(shive, subkey, valuename, architecture)
	Dim hiveKey, objCtx, objLocator, objServices, objReg, Inparams, Outparams

	' First, get the Registry Provider for the requested architecture
	Set objCtx = CreateObject("WbemScripting.SWbemNamedValueSet")
	objCtx.Add "__ProviderArchitecture", architecture ' Must be 64 of 32
	Set objLocator = CreateObject("Wbemscripting.SWbemLocator")
	Set objServices = objLocator.ConnectServer("","root\default","","",,,,objCtx)
	Set objReg = objServices.Get("StdRegProv")

	' Check the hive and give it the right value
	Select Case shive
		Case "HKCR", "HKEY_CLASSES_ROOT"
			hiveKey = &h80000000
		Case "HKCU", "HKEY_CURRENT_USER"
			hiveKey = &H80000001
		Case "HKLM", "HKEY_LOCAL_MACHINE"
			hiveKey = &h80000002
		Case "HKU", "HKEY_USERS"
			hiveKey = &h80000003
		Case "HKCC", "HKEY_CURRENT_CONFIG"
			hiveKey = &h80000005
		Case "HKDD", "HKEY_DYN_DATA" ' Only valid for Windows 95/98
			hiveKey = &h80000006
		Case Else
			MsgBox "Hive not valid (ReadRegistryKey)"
	End Select

	Set Inparams = objReg.Methods_("GetStringValue").Inparameters
	Inparams.Hdefkey = hiveKey
	Inparams.Ssubkeyname = subkey
	Inparams.Svaluename = valuename
	Set Outparams = objReg.ExecMethod_("GetStringValue", Inparams,,objCtx)

	ReadRegistryKey = Outparams.SValue
End Function

Function DetermineSVNVersion()
	Dim WshShell, version, branch, modified, revision, url, oExec, line, hash
	Set WshShell = CreateObject("WScript.Shell")
	On Error Resume Next

	revision = 0

	' Try TortoiseSVN
	' Get the directory where TortoiseSVN (should) reside(s)
	Dim sTortoise
	' First, try with 32-bit architecture
	sTortoise = ReadRegistryKey("HKLM", "SOFTWARE\TortoiseSVN", "Directory", 32)
	If sTortoise = "" Then
		' No 32-bit version of TortoiseSVN installed, try 64-bit version (doesn't hurt on 32-bit machines, it returns nothing or is ignored)
		sTortoise = ReadRegistryKey("HKLM", "SOFTWARE\TortoiseSVN", "Directory", 64)
	End If

	' If TortoiseSVN is installed, try to get the revision number
	If sTortoise <> "" Then
		Dim SubWCRev
		Set SubWCRev = WScript.CreateObject("SubWCRev.object")
		SubWCRev.GetWCInfo FSO.GetAbsolutePathName("./"), 0, 0
		revision = SubWCRev.Revision
		version = "r" & revision
		modified = 0
		if SubWCRev.HasModifications then modified = 2
		url = SubWCRev.Url
	End If

	' Looks like there is no TortoiseSVN installed either. Then we don't know it.
	If revision = 0 Then
		' Reset error and version
		Err.Clear
		version = "norev000"
		modified = 0

		' Set the environment to english
		WshShell.Environment("PROCESS")("LANG") = "en"

		' Do we have subversion installed? Check immediatelly whether we've got a modified WC.
		Set oExec = WshShell.Exec("svnversion .")
		If Err.Number = 0 Then
			' Wait till the application is finished ...
			Do While oExec.Status = 0
			Loop

			line = OExec.StdOut.ReadLine()
			If line <> "exported" Then
				If InStr(line, "M") Then
					modified = 2
				End If

				' And use svn info to get the correct revision and branch information.
				Set oExec = WshShell.Exec("svn info .")
				If Err.Number = 0 Then
					Do
						line = OExec.StdOut.ReadLine()
						If InStr(line, "URL") Then
							url = line
						End If
						If InStr(line, "Last Changed Rev") Then
							revision = Mid(line, 19)
							version = "r" & revision
						End If
					Loop While Not OExec.StdOut.atEndOfStream
				End If ' Err.Number = 0
			End If ' line <> "exported"
		End If ' Err.Number = 0
	End If ' InStr(version, "$")

	If version <> "norev000" Then
		If InStr(url, "branches") Then
			url = Mid(url, InStr(url, "branches/") + 9)
			branch = Mid(url, 1, InStr(2, url, "/") - 1)
		End If
	End If ' version <> "norev000"

	If modified = 2 Then
		version = version & "M"
	End If

	If branch <> "" Then
		version = version & "-" & branch
	End If

	If version <> "norev000" Then
		DetermineSVNVersion = version & Chr(9) & revision & Chr(9) & modified
	Else
		DetermineSVNVersion = version
	End If
End Function

Function IsCachedVersion(ByVal version)
	Dim cache_file, cached_version
	cached_version = ""
	Set cache_file = FSO.OpenTextFile("./config.cache.version", 1, True, 0)
	If Not cache_file.atEndOfStream Then
		cached_version = cache_file.ReadLine()
	End If
	cache_file.Close

	If InStr(version, Chr(9)) Then
		version = Mid(version, 1, Instr(version, Chr(9)) - 1)
	End If

	If version <> cached_version Then
		Set cache_file = fso.CreateTextFile("./config.cache.version", True)
		cache_file.WriteLine(version)
		cache_file.Close
		IsCachedVersion = False
	Else
		IsCachedVersion = True
	End If
End Function

Function CheckFile(filename, source_filename)
	CheckFile = FSO.FileExists(filename)
	If CheckFile Then CheckFile = (FSO.GetFile(filename).DateLastModified >= FSO.GetFile(source_filename).DateLastModified)
End Function

Dim version, version_number
version = DetermineSVNVersion
version_number = LoadVersionNumber("./version.def")
If Not (IsCachedVersion(version) And CheckFile("./version.h", "./version.h.in") And CheckFile("./version.h", "./version.def")) Then
	UpdateFiles version, version_number
End If
