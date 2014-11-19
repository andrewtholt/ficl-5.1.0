

load lib.fth

: os
    s" /proc/cpuinfo" file-status nip 0=
    if
        OS_LINUX
    else
        OS_UNKNOWN
    then
;

: unknown-os?
    os OS_UNKNOWN =
;

: linux-os?
    os OS_LINUX =
;

: darwin-os?
    os OS_DARWIN =
;
\ 
: freebsd-os?
    os OS_FREEBSD =
;
 
: solaris-os?
    os OS_SOLARIS =
;
 
: uclinux-os?
    os OS_UCLINUX =
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

: .cpu
    unknown-cpu? if ." Unknown CPU" then
    x86?      if ." Intel x86" then
    ppc?      if ." PowerPC" then
    8xx?      if ." Embedded PowerPC 8xx" then
    sparc?    if ." SPARC" then
    coldfire? if ." Coldfire" then
    cr
;

: .os
    unknown-os? if ." Unknown OS" then
    linux-os?   if ." Linux" then
    darwin-os?  if ." Darwin/MacOSX" then
    freebsd-os? if ." FreeBSD" then
    solaris-os? if ." Solaris" then
    uclinux-os? if ." uClinux" then
    cr
;

 .cpu
 .os
