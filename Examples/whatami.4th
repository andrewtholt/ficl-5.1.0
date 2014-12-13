\ 
\ 0 constant CPU_UNKNOWN
\ 1 constant CPU_X86
\ 2 constant CPU_PPC
\ 3 constant CPU_8XX
\ 4 constant CPU_AMD64
\ 5 constant CPU_SPARC
\ 6 constant CPU_COLDFIRE
\ 7 constant CPU_ARM
\ 
\ 0 constant OS_UNKNOWN
\ 1 constant OS_LINUX
\ 2 constant OS_DARWIN
\ 3 constant OS_FREEBSD
\ 4 constant OS_SOLARIS
\ 5 constant OS_UCLINUX
\ 6 constant OS_QNX
\ 
load lib.fth

: unknown-os?
	os OS_UNKNOWN =
;
	
: linux-os?
	os OS_LINUX =
;

: darwin-os?
	os OS_DARWIN =
;

: freebsd-os?
	os OS_FREEBSD =
;

: solaris-os?
	os OS_SOLARIS =
;

: uclinux-os?
	os OS_UCLINUX =
;

: qnx-os?
	os OS_QNX =
;

: unknown-cpu?
	cpu CPU_UNKNOWN = 
;

: x86?
	cpu CPU_X86 = 
;

: ppc?
	cpu CPU_PPC =
;

: 8xx?
	cpu CPU_8XX =
;

: amd64?
	cpu CPU_AMD64 =
;

: sparc?
	cpu CPU_SPARC =
;

: coldfire?
	cpu CPU_COLDFIRE =
;

: arm?
	cpu CPU_ARM =
;

: mips?
	cpu CPU_MIPS =
;

: .cpu
	unknown-cpu? if ." Unknown CPU" then
	x86?      if ." Intel x86" then
	ppc?      if ." PowerPC" then
	8xx?      if ." Embedded PowerPC 8xx" then
	sparc?    if ." SPARC" then
	coldfire? if ." Coldfire" then
	arm?      if ." ARM" then
	mips?     if ." MIPS" then
	cr
;

: .os
	unknown-os? if ." Unknown OS" then
	linux-os?   if ." Linux" then
	darwin-os?  if ." Darwin/MacOSX" then
	freebsd-os? if ." FreeBSD" then
	solaris-os? if ." Solaris" then
	uclinux-os? if ." uClinux" then
	qnx-os?     if ." QNX" then
	cr
;
	
.cpu
.os
cr

.features
cr
bye
