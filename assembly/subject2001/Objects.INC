; OBJECTS.INC                 VERSION:  2.27                                  SEPT 19, 2001
; --=====================================================================================--
; This is the MACRO definitions needed to properly sustain the MASM32 OBJECT model
; developed by NaN (Jaymeson@Hotmail.com) and Thomas (_thomas_@runbox.com).
;
; Copyright � 9/19/01  Jaymeson Trudgen and Thomas Bleeker.  If objects become a major
;                      portion of any project, please make an attempt to acknowledge
;                      the use of our work by any means, thank you.
;
; For educational use only. Any commercial re-use only by written license.
;
;
; NOTE:  MASM OBJECTS RESERVED LABELS ARE:  "Class", "Inher", "AbvNa" 

O_WARNINGS equ 1   ; COMPILED WARNING OF MISMATCHED INIT DATA TO CLASS SIZE.
                   ; COMMENT THIS OUT IF NOT WANTED.

; --=====================================================================================--
; MACRO LIST INDEX:
; --=====================================================================================--
;   NEWOBJECT       To create new Object Instances
;   METHOD          To call Instance methods
;   DESTROY         To destroy existing object instances (MUST)
;   SetObject       To set Class to register
;   ReleaseObject   To release Class from register
;   OVERRIDE        To assign a new method offset in any instance
;   SET_CLASS       To assign class inits, and do inheritance if needed. (MUST)
;   SUPER           To provide super classing support of polymorphic methods.
;
;   $EAX()          Accelerated METHOD, returns in eax
;   $EBX()          Accelerated METHOD, returns in ebx
;   $ESI()          Accelerated METHOD, returns in esi
;   $EDI()          Accelerated METHOD, returns in edi
;   $NEW()          Accelerated NEWOBJECT, returns in eax
;   $SUPER()        Accelerated SUPER, returns in eax
;   $DESTROY()      Accelerated DESTROY, returns in eax
;   $invoke()       Accelerated invoke, returns in eax
;
;   BEGIN_INIT      To label Class initialization info in data segment (MUST)
;   END_INIT        To Declair the end of class initialization data (MUST)
;
;   CLASS           To Declair Abbreviated Class Name used in class definitions (MUST)
;   SET_INTERFACE   To Declair Abbreviated Interface and Abv Name (MUST)
;   CMETHOD         TO Declair a Class Method in a Class or Interface structure
;
; --=====================================================================================--
; Set Non case sensitive macros
; --=====================================================================================--
    method     TEXTEQU <METHOD>
    destroy    TEXTEQU <DESTROY>
    newobject  TEXTEQU <NEWOBJECT>
    override   TEXTEQU <OVERRIDE>
    set_class  TEXTEQU <SET_CLASS>
    super      TEXTEQU <SUPER>
    begin_init TEXTEQU <BEGIN_INIT>
    end_init   TEXTEQU <END_INIT>

; --=====================================================================================--
; This macro is used to create a new instance of a class definition.  The instances
; are taken from the local heap.  As well this macro will call the constructor of the
; class to do any needed initializations.
; 
; SYNTAX:  NEWOBJECT ClassName, Constructor Arguments (if any)
; RETURN:  eax == Handle of Object Instance
;
; GENERATED CODE:
; 
;    invoke GetProcessHeap
;    invoke HeapAlloc, eax, NULL, SIZEOF ObjType
;    push   eax
;    invoke ObjType&_Init, eax (, &args)
;    pop    eax
;   
; --=====================================================================================--
NEWOBJECT MACRO ObjType:REQ, args:VARARG
     invoke GetProcessHeap
     invoke HeapAlloc, eax, NULL, SIZEOF ObjType

     push eax
     IFNB <args>
        invoke ObjType&_Init, eax, &args
     ELSE
        invoke ObjType&_Init, eax
     ENDIF
     pop eax
ENDM

; --=====================================================================================--
; This Macro is used to call instance methods of a given class.  This macro is allows for
; user flexibility of having either and indirect object label or direct registers for the
; object handle.  Some classes are designed to act differently depending on the initialization
; parameters passed to their constructors, so this macro also supports optional arguments
; if the constructor requires it.
;
;    SYNTAX: (INDIRECT)
;
;       METHOD hMyObject, ClassName, MethodName (, Method arguments)
;
;    SYNTAX: (DIRECT)
;     
;       METHOD ecx, ClassName, MethodName (, Method arguments)
;
;    RETURN: eax == the return value from the method called (if provided)
;
;    CODE GENERATED: (INDIRECT)
;       mov edx, hMyObject
;       invoke (ClassName PTR [edx]).Function, edx (, args)
;
;    CODE GENERATED: (DIRECT)
;       invoke (ClassName PTR [reg]).Function, reg (, args)
;
; --=====================================================================================--


METHOD MACRO pInterface:REQ, Interface:REQ, Function:REQ, args:VARARG
   
   IF (OPATTR (pInterface)) AND 00010000b
        IFNB <args>   
           invoke (Interface PTR[&pInterface]).&Function, pInterface, &args
        ELSE
           invoke (Interface PTR[&pInterface]).&Function, pInterface
        ENDIF 
    ELSE
        mov edx, pInterface
        IFNB <args>
           FOR arg, <args>
              IFIDNI <&arg>, <edx>
                .ERR <edx is not allowed as a Method parameter with indirect object label>
              ENDIF
           ENDM
           invoke (Interface PTR[edx]).&Function, pInterface, &args
        ELSE
           invoke (Interface PTR[edx]).&Function, pInterface
        ENDIF 
    ENDIF
ENDM

; --=====================================================================================--
; This MACRO is used to destroy any class instance.  This macro makes use of the special
; fact that EVERY destructor of ANY class is ALWAYS the first method entry in the instance
; object.  For this reason this macro doesnt need to know what object type the instance
; is to properly destroy it.  This design allows for insurance that clean up will always
; get done for any kind of object nesting.
;
; NOTE: The instance object's destructor is called before the object is returned to the
;       heap.
;
; SYNTAX:  DESTROY hMyInstance  (or)  DESTROY edx (etc.)
; RETURN:  eax == Any return value from the Instance object's destructor.
;
; GENERATED CODE: (Straight forward)
;
; --=====================================================================================--
DESTROY MACRO pName
     mov eax, pName
     push eax
     call dword ptr [eax]
          
     push eax
     invoke GetProcessHeap
     invoke HeapFree, eax, NULL, pName
     pop eax
ENDM     

; --=====================================================================================--
; This macro is used to help method coding.  This macro will assign a class definition
; structure to the register passed.  This will shorten mov's etc. with any private variables
; in the object.
;
; NOTE: You MUST use the corresponding "ReleaseObject" at the end of your method or
;       unpredictable problems/errors/crashes could arise.  Also, only one register
;       should be Set at any point in your code.
;
; SYNTAX:  SetObject edi, MyClass
;          mov [edi].Variable1, NULL         ; NULL Private variable
;          lea eax, [edi].Variable1          ; Get address of private Variable in eax
;          ...
;          ReleaseObject edi                 ; MUST release before leaving method.
;          ret
;
; GENERATED CODE:
;
;          mov reg, lpTHIS
;
; --=====================================================================================--
SetObject MACRO reg:REQ, objtype:REQ
 mov  reg, lpTHIS
 assume reg:PTR objtype
ENDM

; --=====================================================================================--  
; This Macro is used to release a register from its class assignment (from SetObject).
; Please refer to the SetObject for more info on its use.
; 
; SYNTAX: ReleaseObject edi
;
; GENERATED CODE:  (NONE = Compiler level only)
;
; --=====================================================================================--  
ReleaseObject MACRO reg:REQ
 assume reg: nothing
ENDM

; --=====================================================================================-- 
; This macro is used mainly for POLYMORPHIC assignments.  It is used to OVERRIDE a
; set method's function pointer to a new one, thus having a new unique method code for
; the same method name.
;
; NOTE: This MACRO is generic, and can alter any METHOD from an instanciated object.  For
;       this reason use with some caution.  Also, function PROTO's must agree with the 
;       previous function for this to work.
;
; NOTE: This MACRO must be used in conjunction with the SetObject/ReleaseObject MACROS.
;       and the Register used must be EDI. (This is the default register used in the 
;       MASM32 Object Model ~ its best use is in the constructors of class definitions).
;
; SYNTAX:  
;       OVERRIDE MethodName, NewFunctionProc
;
; CODE GENERATED:
;       mov [edi].MethodName, offset NewFunctionProc
;
; --=====================================================================================-- 
OVERRIDE MACRO AName:REQ, MCode:REQ
     mov [edi].&AName, offset &MCode
ENDM

; --=====================================================================================-- 
; This MACRO is used for initializing instances with correct function pointing.  It is
; required in the constructor of all object classes definitions.  This macro is complicated
; but the code generated is relatively short.  Its main purpose is to fill a new instance
; with the proper method pointers and initialize variables in the instance structure.
; Previous version of Objects.inc, simply did this by "mov [edi].StuctEntry, Stuff", but
; this version replaces this by using Memory Copy from the .Data segment.  This implies that
; in order for this to work, the "ClassName_initdata" found in the data segment must have
; the proper offsets, and variable initializations in the same order as the Class definition.
;
; Secondly, this also does Inheritance if the key word "INHERITS" is found following the
; class name.  Inheritance recursively digresses to the inherited classes where this macro
; should also exist, thus it becomes recursive, and essentially the same action is take to
; properly initialize the instance structure (with a Memory Copy loop).
;
; NOTE: No polymorphic action is take at this level, you must manual override any methods
;       in the constructor *AFTER* this macro call.  Otherwise the overridden method will
;       be corrected by this macro.
;
; NOTE: For proper inheritance the class to be inherited from must be included in the 
;       the class definition previous to this macro.
;
; SYNTAX: (No Inheritance)
;       SET_CLASS Shape                      ; this class is a shape class
;
; SYNTAX: (With Inheritance, no constructor arguments)
;       SET_CLASS Circle INHERITS Shape (, constructor arguments)
;
;   NOTE: 1) There is no comma between the class and the inherited class, only the
;            word "INHERITS"
;         2) The constructor arguments are for the inherited class' constructor, if
;            it was designed to require such.
;
;
; NOTE: 'Class' and 'Inher' are now reserved TEXTEQU's for proper object operations,
;       No other source must alter theses two keywords, or unpredictable results will
;       happen.
;
; RETURN: (Nothing), but ECX is altered.
;
; GENERATED CODE: (Class with no inheritance)
;   	  push    esi
;	  push    edi
;	  cld   
;	  mov 	esi, CONSTANT
;	  mov 	edi, lpTHIS
;	  mov 	ecx, CONSTANT
;	  shr 	ecx, 2
;	  rep 	movsd
;	  mov 	ecx, CONSTANT
;	  and 	ecx, 3
;	  rep 	movsb
;	  pop 	edi
;	  pop 	esi
;
; GENERATED CODE: (Class with inheritance)
;   	  push    esi
;	  push    edi
;      cld
;	  mov 	edi, lpTHIS
;	  mov 	esi, CONSTANT
;	  mov     eax, [esi]
;	  mov     [edi], eax
;      add     esi, 4     
;      add     edi, Inher
; 	  mov 	ecx, CONSTANT
;	  shr 	ecx, 2
;	  rep 	movsd
;	  mov 	ecx, CONSTANT
;	  and 	ecx, 3
;	  rep 	movsb
;	  pop 	edi
;	  pop 	esi
;
; --=====================================================================================-- 
SET_CLASS MACRO AName:REQ, args:VARARG
     LOCAL Where, sz1, sz2, sz3, @InitValLabel, @InitValSizeLabel
     LOCAL bk1, bk2, ck1, ck2
     
     Inher TEXTEQU <>

     ; See if there is the word 'INHERITS'
     Where equ @InStr(1,<AName>,<INHERITS> )
     IFE Where eq 0
      Inher SUBSTR <AName>, Where + 9        ; Get Inheriting Class
     ENDIF

     ; Do size equate:  "THIS_CLASS_initsize equ sizeof THIS_CLASS"
     sz1 CATSTR Class,<_initsize equ sizeof >,<Class>
     
     ; If there was no Inheriting class present
     IF Where eq 0
     sz1                                     ; Do it Init Size!!!!!

     ; Do Initializations and Inheriting
     @InitValLabel     CATSTR Class,<_initdata>   ; LABEL: "THIS_CLASS_initdata"
     @InitValSizeLabel CATSTR Class,<_initend>    ; LABEL: "THIS_CLASS_initend"

     ck2 equ &@InitValSizeLabel
     
       ; No inheriting
   	  push    esi
	  push    edi
	  cld   
%	  mov 	esi, offset @InitValLabel
	  mov 	edi, lpTHIS
%	  mov 	ecx, @InitValSizeLabel
	  shr 	ecx, 2
	  rep 	movsd
%	  mov 	ecx, @InitValSizeLabel
	  and 	ecx, 3
	  rep 	movsb
	  pop 	edi
	  pop 	esi
  ELSE           ; it inherits!...

      bk2 INSTR Inher, <INTERFACE>
     IFE bk2 eq 0
        ; Interface, so only set size...
        sz1                                     ; Do it Init Size!!!!!
        
        Inher SUBSTR Inher, 10
        
     ; Do Initializations and Inheriting
     @InitValLabel     CATSTR Class,<_initdata>   ; LABEL: "THIS_CLASS_initdata"
     @InitValSizeLabel CATSTR Class,<_initend>    ; LABEL: "THIS_CLASS_initend"
     
     ck2 equ &@InitValSizeLabel + sizeof Inher - 4     
        
     ELSE
        ; not an interface, do normal inheritance..
        
         sz1 CATSTR sz1,< - sizeof >,Inher
         sz1                                     ; Do it Init Size!!!!!

     ; Do Initializations and Inheriting
     @InitValLabel     CATSTR Class,<_initdata>   ; LABEL: "THIS_CLASS_initdata"
     @InitValSizeLabel CATSTR Class,<_initend>    ; LABEL: "THIS_CLASS_initend"
     
     ck2 equ &@InitValSizeLabel - 4
     
       ; Inheriting Class 'Inher' is present!
       
       ; Build the invoke string: "invoke INHER_CLASS_Init, lpTHIS"
       sz2 CATSTR <invoke >,Inher,<_Init, lpTHIS>  
       IFNB <args>
          sz2 CATSTR sz2, <, >, <&args>
       ENDIF
       sz2                         ; invoke the inherited constructor
     ENDIF      
     
       ; Modify the destructor, and do THIS_CLASS function pointing
   	  push    esi
	  push    edi
       cld
       
       ; replace the destructor...
	  mov 	edi, lpTHIS
%	  mov 	esi, offset @InitValLabel
	  mov     eax, [esi]
	  mov     [edi], eax

       ; now do rest of Class's inits for THIS_CLASS..
       add     esi, 4                        ; mov to next DWORD
%      add     edi, Inher
%	  mov 	ecx, (@InitValSizeLabel - 4)      
	  shr 	ecx, 2
	  rep 	movsd
%	  mov 	ecx, (@InitValSizeLabel - 4)
	  and 	ecx, 3
	  rep 	movsb
	  
	  pop 	edi
	  pop 	esi
     
     ENDIF   

  IFDEF O_WARNINGS
 
     ; Do basic checking and provide warning if needed
     ; ----------------------------------------------------
     sz2 CATSTR Class,<_initsize>
     ck1 equ &sz2
     
     IFE ck1 eq ck2
       sz2 CATSTR <# Warning # >,Class
       %echo 
       %echo ###########
       %echo sz2 size differs from initialized size
       %echo ###########
       %echo
     endif
     
  ENDIF
     
ENDM     

; --=====================================================================================-- 
; This macro is used to provide support for super classing of inherited methods.  This can
; be called from within ANY method code.  Its purpose is to call the inherited class method
; that has be overridden in a polymorphic action, such that in some cases code re-use can
; be achieved, since OVERRIDE simply ignores the inherited method code.
;
; SYNTAX:
;     SUPER MethodName (, args )
;
; RETURN: eax == any return from the super classed method.
; 
; NOTE:
;   This works by use of the XXX_initdata tables in the data segment that
;   is used to instanciated classes.  It basically checks to see if the method
;   trying to be super'd is within one level of inheritance, and if so, it will
;   calculate the and mov into esi, the function pointer of the super class for this
;   method. 
;
;   IE)  If Donut inherits Circle
;           and Circle inherits Shape
;
;       Then Donut can SUPER call any Methods first introduced in Circle.
;       Then Circle can SUPER call any Methods first introduced in Shape.
;
;       If Donut SUPER call any methods first introduced from Shape,
;         The compile will stop and error a message.
;
; GENERATED CODE:
;
;       invoke funcptr PTR [ (base+offset) ], lpTHIS [, args ]
;
;       NOTE: base+offset is pre-compiler LOCAL variables, and not in code.  
;  
; --=====================================================================================-- 
SUPER MACRO MName:REQ, args:VARARG 
  LOCAL sz1, sz2, sz3, sz4, sz5, count, check, bases
  LOCAL bk1, bk2, funcptr
  
  FOR arg, <args>
    IFIDNI <&arg>, <edx>
        .ERR <edx is not allowed as a Method parameter>
  	ENDIF
  ENDM
    
  sz1 CATSTR Inher,<_initdata>    			 ; LABEL "INHER_initdata"
  sz2 CATSTR Inher,<.>,<&MName>   			 ; LABEL "INHER.MethodName"
  sz5 CATSTR Inher,<_initsize>               ; LABEL "INHER_initsize"
  
  check = sizeof Inher - sz5                 ; check = Sizeof (INHER - INHER_INITSIZE)
  bases = sz2                                ; bases = byte offset of method in struct
  
  ; If check = 0 then it implies that the inherited class doesnt inherit a previous
  ;              class, thus INHER is a normal class.
  ; If check != 0 then it does imply the inherited class also inherits further back.
  ;
  ; Thus if BASES < Check, then the method were trying to superclass is defined
  ; further back in there inheritance chain!  The only exception to this is for 
  ; the destructor as its always the first entry in a structure and will be less than.
  
  IF bases lt check                          ; Check for more than 1 level call
   IFE bases eq 0                            ; double check to see if its not a destructor
     sz3 CATSTR <Can't Superclass methods beyond the first level of inheritance: &MName is not defined in >,Inher
    .err sz3                                 ; Cancle compile, and error
   ENDIF
  ENDIF
  
  ; Here, implies that the method > check, so within 1 level of inheritance, OR
  ; that the super method is the destructor.  If it is the destructor the offset
  ; is already correct (0).  If not, we need to do some more calculations.
  ; 
  ; Since each Class has its own init segment specific to the methods it provides,
  ; and this segment doesnt take in any ordering of anything it inherits, we need
  ; to reduce the offset by the size if the class inherited. (bases - check)
  ;
  ; However, if the inherited class is a NORMAL class this is perfect, the destructors
  ; will be properly aligned. BUT, if the inherited class inherits as well, then we
  ; need to adjust the offset 4 bytes ahead. (This is because in the inheriting class
  ; definitions, there is only 1 destructor present in any class, yet every data segment
  ; init has a destructor defined).
  IFE bases eq 0                                  ; If not...
    bk1 = bases - check                           ; Move the offset back 
    IFE check eq 0
      bk1 = bk1 + 4                               ; if this inherits as well, adjust
    ENDIF
    sz2 TEXTEQU %bk1                              ; replace sz2 with new offset
  ENDIF
    
 	sz4 CATSTR AbvNa, <_&MName>, <Pto>		     ; Inhe_MethodPto (was sz4)
    
	funcptr TYPEDEF PTR &sz4				     ; funcptr TYPEDEF PTR Inhe_MethodPto

    sz4 CATSTR <invoke funcptr PTR [>,sz1,<+>,&sz2, <], lpTHIS>   
   
    IFNB <args>   
        sz4 CATSTR sz4, <, >, <&args> 
    ENDIF 

	sz4                                          ; ONLY LINE OF CODE!
ENDM  

; Accelerator Macro's
; --=====================================================================================--
; The Method Accelerator macros are $EAX(), $EBX(), $EDI(), $ESI().  They are used for
; inline coding (to shorted your code, as well as make some 'code' more readable).  There
; is a small price to pay to use $EBX, $EDI, and $ESI in you code, one extra
; "mov" statement more than the METHOD macro itself.  As well, proper understanding of
; their use is needed or very hard to find bugs may arise.  Namely, each of these
; accelerator macros does NOT give any attention to register preservation.  Method itself,
; will only use EDX (if needed), and will always alter EAX! 
;
; These accelerator make use of the "EXITM" macro command.  Essentially, the macro is placed
; into code on the line BEFORE, and the MACRO itself is replaces with what is given in 
; EXITM.
;
; Example1:   mov hData, $EAX( hObject, Class, Method, Param1)
; 
;    This will be replaced as:
;
;            METHOD hObject, Class, Method, Param1
;            mov hData, eax
;
; Example2:   invoke MessageBox, $ESI(hObj1, Class, getWindow), \
;                                $EDI(hObj1, Class, getMessage), \
;                                $EBX(hObj2, Class2, getTitle), \
;                                $EAX(hObj1, Class, getMsgType)
;
;    This will be replaced as:
;
;             METHOD hObj1, Class, getWindow
;             mov esi, eax
;             METHOD hObj1, Class, getMessage
;             mov edi, eax
;             METHOD hObj2, Class2, getTitle
;             mov ebx, eax
;             METHOD hObj1, Class, getMsgType
;             invoke MessageBox, esi, edi, ebx, eax
;
; It is important to realize that $EAX will most often be used as in example 1, as
; most cases in MASM will only demand one operation parameter.  But, if the situation
; arises for multiple parameters as in example 2, $EAX should **ALWAYS** be last or
; eax will be erased by the next $() accelerator method macro's.
;
; Also note the choices are not by accident, as EBX, ESI, and EDI are typically window's
; reserved registers and "shouldn't" get changed vary often, but it is not to say it can't. 
; Furthermore, if you do use them, be sure to PUSH / POP them before the method/program exits.
;
; As a last thought, the $EAX() style is to help remind you that this register will hold
; the return value of the method being called.
;
; I find them a very practical use in coding practice if you understand their limitations.
; Hope you think so too, NaN.
;
; --=====================================================================================--
; $EAX(METHOD)
;
; SYNTAX:  mov hData, $EAX(hObject, Class, Method [, Params])
;
; RETURN:  eax == Method Return Data
; 
; CODE GENERATED: (no more than the METHOD macro does)
;
;          METHOD hObject, Class, Method [, Params]
;          mov Data, eax
;
; --=====================================================================================--
$EAX MACRO pInterface:REQ, Interface:REQ, Function:REQ, args:VARARG
   IFNB <args>
      METHOD pInterface, Interface, Function, args
   ELSE
      METHOD pInterface, Interface, Function
   ENDIF
   EXITM <eax>
ENDM

; --=====================================================================================--
; $EBX(METHOD)
;
; SYNTAX:  mov hData, $EBX(hObject, Class, Method [, Params])
;
; RETURN:  ebx == Method Return Data
; 
; CODE GENERATED: ( One extra mov than the METHOD Macro )
;
;          METHOD hObject, Class, Method [, Params]
;          mov ebx, eax
;          mov Data, ebx
;
; --=====================================================================================--
$EBX MACRO pInterface:REQ, Interface:REQ, Function:REQ, args:VARARG
   IFNB <args>
      METHOD pInterface, Interface, Function, args
   ELSE
      METHOD pInterface, Interface, Function
   ENDIF
   mov ebx, eax
   EXITM <ebx>
ENDM

; --=====================================================================================--
; $ESI(METHOD)
;
; SYNTAX:  mov hData, $ESI(hObject, Class, Method [, Params])
;
; RETURN:  esi == Method Return Data
; 
; CODE GENERATED: ( One extra mov than the METHOD Macro )
;
;          METHOD hObject, Class, Method [, Params]
;          mov esi, eax
;          mov Data, esi
;
; --=====================================================================================--
$ESI MACRO pInterface:REQ, Interface:REQ, Function:REQ, args:VARARG
   IFNB <args>
      METHOD pInterface, Interface, Function, args
   ELSE
      METHOD pInterface, Interface, Function
   ENDIF
   mov esi, eax
   EXITM <esi>
ENDM

; --=====================================================================================--
; $EDI(METHOD)
;
; SYNTAX:  mov hData, $EDI(hObject, Class, Method [, Params])
;
; RETURN:  edi == Method Return Data
; 
; CODE GENERATED: ( One extra mov than the METHOD Macro )
;
;          METHOD hObject, Class, Method [, Params]
;          mov edi, eax
;          mov Data, edi
;
; --=====================================================================================--
$EDI MACRO pInterface:REQ, Interface:REQ, Function:REQ, args:VARARG
   IFNB <args>
      METHOD pInterface, Interface, Function, args
   ELSE
      METHOD pInterface, Interface, Function
   ENDIF
   mov edi, eax
   EXITM <edi>
ENDM

; --=====================================================================================--
; $NEW() 
;
; This macro accelerates the NEWOBJECT macro for inline coding.  It creates the exact
; same code as NEWOBJECT does.
;
; SYNTAX:  mov hNewObject, $NEW( ClassName, Param1 )
;
; RETURN:  eax = The handle to a newly created object
;
; CODE GENERATED:
;
;          NEWOBJECT ClassName, Param1
;          mov hNewObject, eax
; --=====================================================================================--
$NEW MACRO ClsName:REQ, args:VARARG
   IFNB <args>
      NEWOBJECT ClsName, args
   ELSE
      NEWOBJECT ClsName
   ENDIF
   EXITM <eax>
ENDM

; --=====================================================================================--
; $SUPER() 
;
; This macro accelerates the SUPER macro for inline coding.  It creates the exact
; same code as SUPER does.
;
; SYNTAX:  mov Data, $SUPER( MethodName, Param1 )
;
; RETURN:  eax = The return data by MethodName
;
; CODE GENERATED:
;
;          SUPER ClassName, Param1
;          mov hNewObject, eax
; --=====================================================================================--
$SUPER MACRO MName:REQ, args:VARARG
   IFNB <args>
      SUPER MName, args
   ELSE
      SUPER MName
   ENDIF
   EXITM <eax>
ENDM

; --=====================================================================================--
; $DESTROY() 
;
; This macro accelerates the DESTROY macro for inline coding.  It creates the exact
; same code as DESTROY does.
;
; SYNTAX:  mov hAnyReturn, $DESTROY( hInstance )
;
; RETURN:  eax = The any return from destructor
;
; CODE GENERATED:
;
;          DESTROY hInstance
;          mov hAnyReturn, eax
; --=====================================================================================--
$DESTROY MACRO OName:REQ
   DESTROY OName
   EXITM <eax>
ENDM

; --=====================================================================================--
; $invoke() 
;
; This macro accelerates the normal invoke macro for inline coding.  It creates the exact
; same code as invoke does.
;
; SYNTAX:  mov hAnyReturn, $invoke( function [,args] )
;
; RETURN:  eax = The any return from function
;
; CODE GENERATED:
;
;          invoke function [, args]
;          mov hAnyReturn, eax
; --=====================================================================================--
$invoke MACRO Fun:REQ, A:VARARG
  IFB <A>
    invoke Fun
  else
    invoke Fun, A
  endif
  exitm <eax>
ENDM

; --=====================================================================================--
; This macro has one purpose.  It is used to Provide a proper label
; for class initializations (based on the Class name given in CLASS macro). 
;
; The main purpose of providing proper data segment label is used as follows:
;
;           BEGIN_INIT
;               dd  OFFSET Circ_destroy_Funct
;               dd  OFFSET Circ_setRadius_Funct
;               dd  01
;           END_INIT
;
; The code generated by this is:
;
;           Circle_AbvName TEXTEQU <Circ>
;           Circle_initdata LABEL BYTE
;               dd  OFFSET Circ_destroy_Funct
;               dd  OFFSET Circ_setRadius_Funct
;               dd  01
;           Circle_initend equ $-Circle_initdata
;
; Its important to note that ONLY the data provided is translated to segment data
; and the macro's themselves only generate Compile level symbols and equates.
;
; --=====================================================================================--
BEGIN_INIT MACRO ;CName:REQ, Abiv:VARARG
    LOCAL sz1
  
     sz1 CATSTR Class,<_initdata LABEL BYTE>
     sz1
     
ENDM     

END_INIT MACRO
       LOCAL sz1, sz2
       sz1 CATSTR Class,<_initend equ $->,Class,<_initdata> ;,< equ $->,@CatStr(Class,<_initdata>)
       sz1
ENDM

; --=====================================================================================--
; This method is used to build an INTERFACE.  It acts like CLASS except is doesn't
; begin the Interface structure.  As this also determins the structure size, it
; is not possible in one macro to start a structure label, and then get its final size!
; 
; For this reason, you must manualy start the interface structure.
;
; SYNTAX:
;
;    SET_INTERFACE Graphic, Grap
;        
; CODE GENERATED:
;
;    Class TEXTEQU <Graphic>
;    AbvNa TEXTEQU <Grap>
;    Graphic_initsize equ sizeof Graphic
;
; SYNTAX:
;
;    SET_INTERFACE Graphic
;        
; CODE GENERATED:
;
;    Class TEXTEQU <Graphic>
;    AbvNa TEXTEQU <Graphic>
;    Graphic_initsize equ sizeof Graphic
;
; ACTUAL EXAMPLE:
;
;  SET_INTERFACE Shape
;  Shape STRUC
;      CMETHOD destructor   <<< MUST! 
;      CMETHOD method1
;      CMETHOD method2
;  Shape ends
; --=====================================================================================--
SET_INTERFACE MACRO AName:REQ, Abiv:VARARG
     LOCAL sz1
     
     ; Split up Class and Inhering Classes
     Class TEXTEQU <AName>     

     ; Do size equate:  "THIS_CLASS_initsize equ sizeof THIS_CLASS"
     sz1 CATSTR Class,<_initsize equ sizeof >,Class
     
     sz1                                     ; Do it!

     ; Do Abv Nameing..
     IFNB <Abiv>
       AbvNa TEXTEQU <Abiv>
     ELSE
       AbvNa TEXTEQU <AName>
     ENDIF

ENDM     

; --=====================================================================================--
; This method, uses the already defined Class textequ to and the givin method name to
; build the needed proto pointers for masm to properly invoke function pointers.
; Its a techincal requirement of MASM32.  As well it also builds structure entries
; by the same name for the method.
;
; SYNTAX:
;
;    CMETHOD getCircleRadius
;        
; CODE GENERATED:
;
;    getCircleRadius PTR Circ_getCircleRadiusPto ?
;
; --=====================================================================================--
CMETHOD	MACRO MName:REQ
LOCAL sz1, sz2
     sz2 CATSTR AbvNa, <_&MName>,<Pto>
	sz1 typedef PTR &sz2 
	&MName sz1 ?
ENDM

; --=====================================================================================--
; This macro is used to set the reserved words for the compiling of class objects.
; By providing the Class name, and an optional short form name the reserved TEXTEQU's
; "Class" is set to the full class name, and "AbvNa" is set to the abreviation that
; follows.  If no abbreviation is given, it uses the full class name as well.
;
; SYNTAX:
;
;    Class Circle, Circ
;        
; CODE GENERATED:
;
;    Circle STRUC
;
;  NOTE: Class = "Circle", AbvNa = "Circ"
;
; SYNTAX:
;
;     Class Circle
;
; CODE GENERATED:
;
;     Circle STRUC
; 
;  NOTE: Class = "Circle", AbvNa = "Circle"
; --=====================================================================================--
CLASS MACRO CName:REQ, Abiv:VARARG
   IFNB <Abiv>
     AbvNa TEXTEQU <Abiv>
   ELSE
     AbvNa TEXTEQU <CName>
   ENDIF

     Class TEXTEQU <CName>

     @CatStr( CName, < STRUC >)
    
ENDM

; Is there ever an end to the madness............?   ( :   NaN & Thomas