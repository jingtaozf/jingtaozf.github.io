;////////// Windows Version Build Option //////////

;#define _WIN32_WINNT 0x0500
;#define WINVER       0x0500

;//////////////////// Unicode Build Option ////////////////

; If we are not compiling for an x86 CPU, we always compile using Unicode.
;IFNDEF _M_IX86
;DEFINE UNICODE
;ENDIF
;To compile using Unicode on the x86 CPU, uncomment the line below.
;#define UNICODE
;When using Unicode Windows functions, use Unicode C-Runtime functions too.
;IFDEF UNICODE
;DEFINE _UNICODE
;ENDIF

;////////////// Quick MessageBox Macro /////////////////
	.data?
		szTMP_chMB db 128 dup(?)
	.code

chMB MACRO s
local szTMP
	invoke GetModuleFileNameA,NULL,addr szTMP, sizeof szTMP
	invoke GetActiveWindow
	invoke MessageBoxA,eax,s,addr szTMP,MB_OK
 ENDM

;///////// Dialog Box Icon Setting Macro /////////////////

chSETDLGICONS MACRO hWnd,id
	invoke GetWindowLong,hWnd, GWL_HINSTANCE
	invoke LoadIcon,eax,id
	push eax
	invoke SendMessage,hWnd,WM_SETICON,ICON_SMALL,eax		
	pop eax
	invoke SendMessage,hWnd,WM_SETICON,ICON_BIG,eax
 ENDM

;//////////// OS Version Check Macros //////////////////

chWindows9xNotAllowed MACRO 
LOCAL vi
	.data
		vi OSVERSIONINFO <>
	.code
	invoke GetVersionEx,addr vi
	.if vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
		chMB SADD("This application requires features not present in Windows 9x.")
      invoke ExitProcess,0	
	.endif
 ENDM

chWindows2000Required macro
LOCAL vi
	.data
		vi OSVERSIONINFO <>
	.code
	invoke GetVersionEx,addr vi
	.if (vi.dwPlatformId != VER_PLATFORM_WIN32_NT) && (vi.dwMajorVersion < 5)
		chMB SADD("This application requires features present in Windows 2000.")
		invoke ExitProcess,0
	.endif
 endm

;///////////////// UNICODE Check Macro /////////////////////

;Since Windows 98 does not support Unicode, issue an error and terminate
; the process if this is a native Unicode build running on Windows 98

; This is accomplished by creating a global C++ object. Its constructor
;is executed before WinMain.

;ifdef UNICODE
;	invoke GetWindowsDirectoryW,NULL, 0
;	chMB("This application requires an OS that supports Unicode.")
;	invoke ExitProcess,0
;ENDIF

