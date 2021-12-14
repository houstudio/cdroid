!***********************************************************************
!  plplot_binding.f90
!
!  Copyright (C) 2005-2016  Arjen Markus
!  Copyright (C) 2006-2018 Alan W. Irwin
!
!  This file is part of PLplot.
!
!  PLplot is free software; you can redistribute it and/or modify
!  it under the terms of the GNU Library General Public License as published
!  by the Free Software Foundation; either version 2 of the License, or
!  (at your option) any later version.
!
!  PLplot is distributed in the hope that it will be useful,
!  but WITHOUT ANY WARRANTY; without even the implied warranty of
!  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!  GNU Library General Public License for more details.
!
!  You should have received a copy of the GNU Library General Public License
!  along with PLplot; if not, write to the Free Software
!  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
!
!
!***********************************************************************

module plplot
    use plplot_single
    use plplot_double
    use plplot_types, only: private_plflt, private_plint, private_plbool, private_plunicode, private_single, private_double
    use plplot_graphics
    use iso_c_binding, only: c_ptr, c_char, c_loc, c_funloc, c_funptr, c_null_char, c_null_ptr, c_null_funptr
    use iso_fortran_env, only: error_unit
    use plplot_private_utilities, only: character_array_to_c, c_to_character_array, copystring2f, max_cstring_length
    implicit none
    ! We will continue to define plflt for those of our users who are
    ! content to simply follow in their own Fortran code the
    ! floating-point precision of our C libraries, but our users no
    ! longer need to use of this parameter.  Instead, kind(1.0) or
    ! kind(1.0d0) floating-point precision should work for users so
    ! long as the precision of the floating-point arguments of a given
    ! call to a PLplot routine are identical.
    integer, parameter :: plflt = private_plflt
    ! The following parameter is provided as a convenience to PLplot
    ! developers to allow them to test any combination of
    ! floating-point precision of our C libraries and the
    ! floating-point precision (pl_test_flt) of our Fortran examples.
    integer, parameter :: pl_test_flt = private_plflt
    integer(kind=private_plint), parameter :: maxlen = 320
    character(len=1), parameter :: PL_END_OF_STRING = achar(0)
    include 'included_plplot_parameters.f90'
    private :: private_plflt, private_plint, private_plbool, private_plunicode, private_single, private_double
    private :: c_ptr, c_char, c_loc, c_funloc, c_funptr, c_null_char, c_null_ptr, c_null_funptr
    private :: copystring2f, maxlen
    private :: max_cstring_length
    private :: error_unit
    private :: character_array_to_c
    private :: c_to_character_array

    ! Normally interface blocks describing the C routines that are
    ! called by this Fortran binding are embedded as part of module
    ! procedures, but when more than one module procedure uses such
    ! interface blocks there is a requirement (enforced at least by
    ! the nagfor compiler) that those interface blocks be consistent.
    ! We could comply with that requirement by embedding such multiply
    ! used interface blocks as part of module procedures using
    ! duplicated code, but that is inefficient (in terms of the number
    ! of lines of code to be compiled) and implies a maintenance issue
    ! (to keep that code duplicated whenever there are changes on the
    ! C side).  To deal with those two potential issues we collect
    ! here in alphabetical order all interface blocks describing C
    ! routines that are called directly by more than one module
    ! procedure below.

    interface
       function interface_plparseopts( argc, argv, mode ) bind(c,name='c_plparseopts')
         import :: c_ptr
         import :: private_plint
         implicit none
         integer(kind=private_plint) :: interface_plparseopts !function type
         integer(kind=private_plint), value, intent(in) :: mode
         ! The C routine changes both argc and argv
         integer(kind=private_plint), intent(inout) :: argc
         type(c_ptr), dimension(*), intent(inout) :: argv
       end function  interface_plparseopts
    end interface
    private :: interface_plparseopts

    !
    ! Interfaces that do not depend on the real kind.
    !
    interface pl_setcontlabelformat
        module procedure pl_setcontlabelformat_impl
    end interface pl_setcontlabelformat
    private :: pl_setcontlabelformat_impl

    interface pladv
        module procedure pladv_impl
    end interface pladv
    private :: pladv_impl

    interface plbop
        module procedure plbop_impl
    end interface plbop
    private :: plbop_impl

    interface plclear
        module procedure plclear_impl
    end interface plclear
    private :: plclear_impl

    interface plcol0
        module procedure plcol0_impl
    end interface plcol0
    private :: plcol0_impl

    interface plcpstrm
        module procedure plcpstrm_impl
    end interface plcpstrm
    private :: plcpstrm_impl

    interface plend1
        module procedure plend1_impl
    end interface plend1
    private :: plend1_impl

    interface plend
        module procedure plend_impl
    end interface plend
    private :: plend_impl

    interface pleop
        module procedure pleop_impl
    end interface pleop
    private :: pleop_impl

    interface plfamadv
        module procedure plfamadv_impl
    end interface plfamadv
    private :: plfamadv_impl

    interface plflush
        module procedure plflush_impl
    end interface plflush
    private :: plflush_impl

    interface plfont
        module procedure plfont_impl
    end interface plfont
    private :: plfont_impl

    interface plfontld
        module procedure plfontld_impl
    end interface plfontld
    private :: plfontld_impl

    interface plgcol0
        module procedure plgcol0_impl
    end interface plgcol0
    private :: plgcol0_impl

    interface plgcolbg
        module procedure plgcolbg_impl
    end interface plgcolbg
    private :: plgcolbg_impl

    interface plgcompression
        module procedure plgcompression_impl
    end interface plgcompression
    private :: plgcompression_impl

    interface plgdev
        module procedure plgdev_impl
    end interface plgdev
    private :: plgdev_impl

    interface plgdrawmode
        module procedure plgdrawmode_impl
    end interface plgdrawmode
    private :: plgdrawmode_impl

    interface plget_arguments
        module procedure plget_arguments_dynamic
        module procedure plget_arguments_static_length
        module procedure plget_arguments_static
    end interface plget_arguments
    private :: plget_arguments_dynamic, plget_arguments_static_length, plget_arguments_static

    interface plgfam
        module procedure plgfam_impl
    end interface plgfam
    private :: plgfam_impl

    interface plgfci
        module procedure plgfci_impl
    end interface plgfci
    private :: plgfci_impl

    interface plgfnam
        module procedure plgfnam_impl
    end interface plgfnam
    private :: plgfnam_impl

    interface plgfont
        module procedure plgfont_impl
    end interface plgfont
    private :: plgfont_impl

    interface plglevel
        module procedure plglevel_impl
    end interface plglevel
    private :: plglevel_impl

    interface plgra
        module procedure plgra_impl
    end interface plgra
    private :: plgra_impl

    interface plgstrm
        module procedure plgstrm_impl
    end interface plgstrm
    private :: plgstrm_impl

    interface plgver
        module procedure plgver_impl
    end interface plgver
    private :: plgver_impl

    interface plgxax
        module procedure plgxax_impl
    end interface plgxax
    private :: plgxax_impl

    interface plgyax
        module procedure plgyax_impl
    end interface plgyax
    private :: plgyax_impl

    interface plgzax
        module procedure plgzax_impl
    end interface plgzax
    private :: plgzax_impl

    interface plinit
        module procedure plinit_impl
    end interface plinit
    private :: plinit_impl

    interface pllab
        module procedure pllab_impl
    end interface pllab
    private :: pllab_impl

    interface pllsty
        module procedure pllsty_impl
    end interface pllsty
    private :: pllsty_impl

    interface plmkstrm
        module procedure plmkstrm_impl
    end interface plmkstrm
    private :: plmkstrm_impl

    interface plparseopts
        module procedure plparseopts_dynamic
        module procedure plparseopts_static_length
        module procedure plparseopts_static
        module procedure plparseopts_brief
    end interface plparseopts
    private :: plparseopts_dynamic, plparseopts_static_length, plparseopts_static, plparseopts_brief

    interface plpat
        module procedure plpat_impl
    end interface plpat
    private :: plpat_impl

    interface plprec
        module procedure plprec_impl
    end interface plprec
    private :: plprec_impl

    interface plpsty
        module procedure plpsty_impl
    end interface plpsty
    private :: plpsty_impl

    interface plreplot
        module procedure plreplot_impl
    end interface plreplot
    private :: plreplot_impl

    interface plscmap0
        module procedure plscmap0_impl
    end interface plscmap0
    private :: plscmap0_impl

    interface plscmap0n
        module procedure plscmap0n_impl
    end interface plscmap0n
    private :: plscmap0n_impl

    interface plscmap1
        module procedure plscmap1_impl
    end interface plscmap1
    private :: plscmap1_impl

    interface plscmap1n
        module procedure plscmap1n_impl
    end interface plscmap1n
    private :: plscmap1n_impl

    interface plscol0
        module procedure plscol0_impl
    end interface plscol0
    private :: plscol0_impl

    interface plscolbg
        module procedure plscolbg_impl
    end interface plscolbg
    private :: plscolbg_impl

    interface plscolor
        module procedure plscolor_impl
    end interface plscolor
    private :: plscolor_impl

    interface plscompression
        module procedure plscompression_impl
    end interface plscompression
    private :: plscompression_impl

    interface plsdev
        module procedure plsdev_impl
    end interface plsdev
    private :: plsdev_impl

    interface plsdrawmode
        module procedure plsdrawmode_impl
    end interface plsdrawmode
    private :: plsdrawmode_impl

    interface plseed
        module procedure plseed_impl
    end interface plseed
    private :: plseed_impl

    interface plsesc
        module procedure plsesc_impl
    end interface plsesc
    private :: plsesc_impl

    interface plsetopt
        module procedure plsetopt_impl
    end interface plsetopt
    private :: plsetopt_impl

    interface plsfam
        module procedure plsfam_impl
    end interface plsfam
    private :: plsfam_impl

    interface plsfci
        module procedure plsfci_impl
    end interface plsfci
    private :: plsfci_impl

    interface plsfnam
        module procedure plsfnam_impl
    end interface plsfnam
    private :: plsfnam_impl

    interface plsfont
        module procedure plsfont_impl
    end interface plsfont
    private :: plsfont_impl

    interface plsmem
        module procedure plsmem_impl
    end interface plsmem
    private :: plsmem_impl

    interface plsmema
        module procedure plsmema_impl
    end interface plsmema
    private :: plsmema_impl

    interface plsori
        module procedure plsori_impl
    end interface plsori
    private :: plsori_impl

    interface plspal0
        module procedure plspal0_impl
    end interface plspal0
    private :: plspal0_impl

    interface plspal1
        module procedure plspal1_impl
    end interface plspal1
    private :: plspal1_impl

    interface plspause
        module procedure plspause_impl
    end interface plspause
    private :: plspause_impl

    interface plsstrm
        module procedure plsstrm_impl
    end interface plsstrm
    private :: plsstrm_impl

    interface plssub
        module procedure plssub_impl
    end interface plssub
    private :: plssub_impl

    interface plstar
        module procedure plstar_impl
    end interface plstar
    private :: plstar_impl

    interface plstart
        module procedure plstart_impl
    end interface plstart
    private :: plstart_impl

    interface plstripd
        module procedure plstripd_impl
    end interface plstripd
    private :: plstripd_impl

    interface plstyl
        module procedure plstyl_impl
    end interface plstyl
    private :: plstyl_impl

    ! double- and single-precision variants defined in the plplot_double and plplot_single modules.
    interface plsvect
        module procedure plsvect_none
    end interface plsvect
    private :: plsvect_none

    interface plsxax
        module procedure plsxax_impl
    end interface plsxax
    private :: plsxax_impl

    interface plsyax
        module procedure plsyax_impl
    end interface plsyax
    private :: plsyax_impl

    interface plszax
        module procedure plszax_impl
    end interface plszax
    private :: plszax_impl

    interface pltext
        module procedure pltext_impl
    end interface pltext
    private :: pltext_impl

    interface pltimefmt
        module procedure pltimefmt_impl
    end interface pltimefmt
    private :: pltimefmt_impl

    interface plvsta
        module procedure plvsta_impl
    end interface plvsta
    private :: plvsta_impl

    interface plxormod
        module procedure plxormod_impl
    end interface plxormod
    private :: plxormod_impl

contains

    !
    ! Interface routines
    !
    subroutine pl_setcontlabelformat_impl( lexp, sigdig )
        integer, intent(in) :: lexp, sigdig

        interface
            subroutine interface_pl_setcontlabelformat( lexp, sigdig ) bind(c,name='c_pl_setcontlabelformat')
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: lexp, sigdig
            end subroutine interface_pl_setcontlabelformat
        end interface

        call interface_pl_setcontlabelformat( int(lexp,kind=private_plint), int(sigdig,kind=private_plint) )
    end subroutine pl_setcontlabelformat_impl

    subroutine pladv_impl( sub )
        integer, intent(in) :: sub
        interface
            subroutine interface_pladv( sub ) bind( c, name = 'c_pladv' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: sub
            end subroutine interface_pladv
        end interface

        call interface_pladv( int(sub,kind=private_plint) )
    end subroutine pladv_impl

    subroutine plbop_impl()
        interface
            subroutine interface_plbop() bind(c,name='c_plbop')
            end subroutine interface_plbop
        end interface
        call interface_plbop()
    end subroutine plbop_impl

    subroutine plclear_impl()
        interface
            subroutine interface_plclear() bind(c,name='c_plclear')
            end subroutine interface_plclear
        end interface
        call interface_plclear()
    end subroutine plclear_impl

    subroutine plcol0_impl( icol )
        integer, intent(in) :: icol
        interface
            subroutine interface_plcol0( icol ) bind(c, name = 'c_plcol0' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: icol
            end subroutine interface_plcol0
        end interface

        call interface_plcol0( int(icol,kind=private_plint) )
    end subroutine plcol0_impl

    subroutine plcpstrm_impl( iplsr, flags )
        integer, intent(in) :: iplsr
        logical, intent(in) :: flags
        interface
            subroutine interface_plcpstrm( iplsr, flags ) bind(c, name = 'c_plcpstrm' )
                import :: private_plint, private_plbool
                implicit none
                integer(kind=private_plint), value, intent(in) :: iplsr
                integer(kind=private_plbool), value, intent(in) :: flags
            end subroutine interface_plcpstrm
        end interface

        call interface_plcpstrm( int(iplsr,kind=private_plint), int(merge(1,0,flags),kind=private_plbool) )
    end subroutine plcpstrm_impl

    subroutine plend1_impl()
        interface
            subroutine interface_plend1() bind( c, name = 'c_plend1' )
            end subroutine interface_plend1
        end interface
        call interface_plend1()
    end subroutine plend1_impl

    subroutine plend_impl()
        interface
            subroutine interface_plend() bind( c, name = 'c_plend' )
            end subroutine interface_plend
        end interface
        call interface_plend()
    end subroutine plend_impl

    subroutine pleop_impl()
        interface
            subroutine interface_pleop() bind( c, name = 'c_pleop' )
            end subroutine interface_pleop
        end interface
        call interface_pleop()
    end subroutine pleop_impl

    subroutine plfamadv_impl()
        interface
            subroutine interface_plfamadv() bind( c, name = 'c_plfamadv' )
            end subroutine interface_plfamadv
        end interface
        call interface_plfamadv()
    end subroutine plfamadv_impl

    subroutine plflush_impl()
        interface
            subroutine interface_plflush() bind( c, name = 'c_plflush' )
            end subroutine interface_plflush
        end interface
        call interface_plflush()
    end subroutine plflush_impl

    subroutine plfont_impl( font )
        integer, intent(in) :: font
        interface
            subroutine interface_plfont( font ) bind( c, name = 'c_plfont' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: font
            end subroutine interface_plfont
        end interface

        call interface_plfont( int(font,kind=private_plint) )
    end subroutine plfont_impl

    subroutine plfontld_impl( charset )
        integer, intent(in) :: charset
        interface
            subroutine interface_plfontld( charset ) bind( c, name = 'c_plfontld' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: charset
            end subroutine interface_plfontld
        end interface

        call interface_plfontld( int(charset,kind=private_plint) )
    end subroutine plfontld_impl

    subroutine plgcol0_impl( icol, r, g, b )
        integer, intent(in) :: icol
        integer, intent(out) :: r, g, b

        integer(kind=private_plint) :: r_out, g_out, b_out

        interface
            subroutine interface_plgcol0( icol, r, g, b ) bind( c, name = 'c_plgcol0' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: icol
                integer(kind=private_plint), intent(out) :: r, g, b
            end subroutine interface_plgcol0
        end interface

        call interface_plgcol0( int(icol,kind=private_plint), r_out, g_out, b_out )
        r = int(r_out)
        g = int(g_out)
        b = int(b_out)
    end subroutine plgcol0_impl

    subroutine plgcolbg_impl( r, g, b )
        integer, intent(out) :: r, g, b

        integer(kind=private_plint) :: r_out, g_out, b_out

        interface
            subroutine interface_plgcolbg( r, g, b ) bind( c, name = 'c_plgcolbg' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: r, g, b
            end subroutine interface_plgcolbg
        end interface

        call interface_plgcolbg( r_out, g_out, b_out )
        r = int(r_out)
        g = int(g_out)
        b = int(b_out)
    end subroutine plgcolbg_impl

    subroutine plgcompression_impl( compression )
        integer, intent(out) :: compression

        integer(kind=private_plint) :: compression_out

        interface
            subroutine interface_plgcompression( compression ) bind( c, name = 'c_plgcompression' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: compression
            end subroutine interface_plgcompression
        end interface

        call interface_plgcompression( compression_out )
        compression = int(compression_out)
    end subroutine plgcompression_impl

    subroutine plgdev_impl(dev)
        character*(*), intent(out) :: dev

        character(len=1), dimension(100) :: dev_out

        interface
            subroutine interface_plgdev( dev ) bind(c,name='c_plgdev')
                implicit none
                character(len=1), dimension(*), intent(out) :: dev
            end subroutine interface_plgdev
        end interface

        call interface_plgdev( dev_out )
        call copystring2f( dev, dev_out )
    end subroutine plgdev_impl

    function plgdrawmode_impl()

        integer :: plgdrawmode_impl !function type

        interface
            function interface_plgdrawmode() bind(c,name='c_plgdrawmode')
                import :: private_plint
                implicit none
                integer(kind=private_plint) :: interface_plgdrawmode !function type
            end function interface_plgdrawmode
        end interface

        plgdrawmode_impl = int(interface_plgdrawmode())
    end function plgdrawmode_impl

    function plget_arguments_dynamic( argv )
        integer :: plget_arguments_dynamic !function type
        character(len=:), dimension(:), allocatable, intent(out) :: argv

        integer :: nargv_local
        character(len=1) :: test_argv_local
        integer :: length_argv_local, length_local, iargs_local

        nargv_local = command_argument_count()
        if (nargv_local < 0) then
!           This actually happened historically on a buggy Cygwin platform.
            write(error_unit,'(a)') 'Plplot Fortran Severe Warning: plget_arguments: negative number of arguments'
            plget_arguments_dynamic = 1
            return
        endif

!       Determine maximum length of arguments
        length_argv_local = 0
        do iargs_local = 0, nargv_local
            call get_command_argument(iargs_local, test_argv_local, length_local)
            length_argv_local = max(length_argv_local, length_local)
        enddo

        ! Allocate argv of correct size (but static length) to hold the command-line arguments.
        allocate(character(len=length_argv_local) :: argv(0:nargv_local))
        do iargs_local = 0, nargv_local
            call get_command_argument(iargs_local, argv(iargs_local))
        enddo
        plget_arguments_dynamic = 0
    end function plget_arguments_dynamic

    function plget_arguments_static_length( argv, disambiguate )
        integer :: plget_arguments_static_length !function type
        character(len=*), dimension(:), allocatable, intent(out) :: argv
        integer, intent(in) :: disambiguate

        integer :: nargv_local
        character(len=1) :: test_argv_local
        integer :: length_argv_local, length_local, iargs_local

        nargv_local = command_argument_count()
        if (nargv_local < 0) then
!           This actually happened historically on a buggy Cygwin platform.
            write(error_unit,'(a)') 'Plplot Fortran Severe Warning: plget_arguments: negative number of arguments'
            plget_arguments_static_length = 1
            return
        endif

!       Determine maximum length of arguments
        length_argv_local = 0
        do iargs_local = 0, nargv_local
            call get_command_argument(iargs_local, test_argv_local, length_local)
            length_argv_local = max(length_argv_local, length_local)
        enddo

        if(length_argv_local > len(argv) ) then
            write(error_unit,*) 'Plplot Fortran Severe Warning: plget_arguments: at least one argument is too long to process'
            plget_arguments_static_length = 1
            return
        endif

        ! Allocate argv of correct size (but static length) to hold the command-line arguments.
        allocate(argv(0:nargv_local))
        do iargs_local = 0, nargv_local
            call get_command_argument(iargs_local, argv(iargs_local))
        enddo
        plget_arguments_static_length = 0
    end function plget_arguments_static_length

    function plget_arguments_static( nargv, argv )
        integer :: plget_arguments_static !function type
        integer, intent(out) :: nargv
        character(len=*), dimension(0:), intent(out) :: argv

        character(len=1) :: test_argv_local
        integer :: length_argv_local, length_local, iargs_local

        nargv = command_argument_count()
        if (nargv < 0) then
!           This actually happened historically on a buggy Cygwin platform.
            write(error_unit,'(a)') 'Plplot Fortran Severe Warning: plget_arguments: negative number of arguments'
            plget_arguments_static = 1
            return
        endif

        if(nargv + 1 > size(argv)) then
            write(error_unit,'(a)') 'Plplot Fortran Severe Warning: plget_arguments: too many arguments to process'
            plget_arguments_static = 1
            return
        endif

!       Determine maximum length of arguments
        length_argv_local = 0
        do iargs_local = 0, nargv
            call get_command_argument(iargs_local, test_argv_local, length_local)
            length_argv_local = max(length_argv_local, length_local)
        enddo
        if(length_argv_local > len(argv) ) then
            write(error_unit,*) 'Plplot Fortran Severe Warning: plget_arguments: at least one argument is too long to process'
            plget_arguments_static = 1
            return
        endif

        do iargs_local = 0, nargv
            call get_command_argument(iargs_local, argv(iargs_local))
        enddo
        plget_arguments_static = 0
    end function plget_arguments_static

    subroutine plgfam_impl( fam, num, bmax )
        integer, intent(out) :: fam, num, bmax

        integer(kind=private_plint) :: fam_out, num_out, bmax_out

        interface
            subroutine interface_plgfam( fam, num, bmax ) bind( c, name = 'c_plgfam' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: fam, num, bmax
            end subroutine interface_plgfam
        end interface

        call interface_plgfam( fam_out, num_out, bmax_out )
        fam  = int(fam_out)
        num  = int(num_out)
        bmax = int(bmax_out)
    end subroutine plgfam_impl

    subroutine plgfci_impl( fci )
        integer, intent(out) :: fci

        integer(kind=private_plunicode) :: fci_out

        interface
            subroutine interface_plgfci( fci ) bind( c, name = 'c_plgfci' )
                import :: private_plunicode
                implicit none
                integer(kind=private_plunicode), intent(out) :: fci
            end subroutine interface_plgfci
        end interface

        call interface_plgfci( fci_out )
        fci  = int(fci_out)
    end subroutine plgfci_impl

    subroutine plgfnam_impl( fnam )
        character*(*), intent(out) :: fnam

        character(len=1), dimension(100) :: fnam_out

        interface
            subroutine interface_plgfnam( fnam ) bind(c,name='c_plgfnam')
                implicit none
                character(len=1), dimension(*), intent(out) :: fnam
            end subroutine interface_plgfnam
        end interface

        call interface_plgfnam( fnam_out )
        call copystring2f( fnam, fnam_out )
    end subroutine plgfnam_impl

    subroutine plgfont_impl( family, style, weight )
        integer, intent(out) :: family, style, weight

        integer(kind=private_plint) :: family_out, style_out, weight_out

        interface
            subroutine interface_plgfont( family, style, weight ) bind( c, name = 'c_plgfont' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: family, style, weight
            end subroutine interface_plgfont
        end interface

        call interface_plgfont( family_out, style_out, weight_out )
        family = int(family_out)
        style  = int(style_out)
        weight = int(weight_out)
    end subroutine plgfont_impl

    subroutine plglevel_impl( level )
        integer, intent(out) :: level

        integer(kind=private_plint) :: level_out

        interface
            subroutine interface_plglevel( level ) bind( c, name = 'c_plglevel' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: level
            end subroutine interface_plglevel
        end interface

        call interface_plglevel( level_out )
        level = int(level_out)
    end subroutine plglevel_impl

    subroutine plgra_impl()
        interface
            subroutine interface_plgra() bind( c, name = 'c_plgra' )
            end subroutine interface_plgra
        end interface
        call interface_plgra()
    end subroutine plgra_impl

    subroutine plgstrm_impl( strm )
        integer, intent(out) :: strm

        integer(kind=private_plint) :: strm_out

        interface
            subroutine interface_plgstrm( strm ) bind( c, name = 'c_plgstrm' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: strm
            end subroutine interface_plgstrm
        end interface

        call interface_plgstrm( strm_out )
        strm = int(strm_out)
    end subroutine plgstrm_impl

    subroutine plgver_impl(ver)
        character*(*), intent(out) :: ver

        character(len=1), dimension(100) :: ver_out

        interface
            subroutine interface_plgver( ver ) bind(c,name='c_plgver')
                implicit none
                character(len=1), dimension(*), intent(out) :: ver
            end subroutine interface_plgver
        end interface

        call interface_plgver( ver_out )
        call copystring2f( ver, ver_out )
    end subroutine plgver_impl

    subroutine plgxax_impl( digmax, digits )
        integer, intent(out) :: digmax, digits

        integer(kind=private_plint) :: digmax_out, digits_out

        interface
            subroutine interface_plgxax( digmax, digits ) bind( c, name = 'c_plgxax' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: digmax, digits
            end subroutine interface_plgxax
        end interface

        call interface_plgxax( digmax_out, digits_out )
        digmax = int(digmax_out)
        digits = int(digits_out)
    end subroutine plgxax_impl

    subroutine plgyax_impl( digmax, digits )
        integer, intent(out) :: digmax, digits

        integer(kind=private_plint) :: digmax_out, digits_out

        interface
            subroutine interface_plgyax( digmax, digits ) bind( c, name = 'c_plgyax' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: digmax, digits
            end subroutine interface_plgyax
        end interface

        call interface_plgyax( digmax_out, digits_out )
        digmax = int(digmax_out)
        digits = int(digits_out)
    end subroutine plgyax_impl

    subroutine plgzax_impl( digmax, digits )
        integer, intent(out) :: digmax, digits

        integer(kind=private_plint) :: digmax_out, digits_out

        interface
            subroutine interface_plgzax( digmax, digits ) bind( c, name = 'c_plgzax' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), intent(out) :: digmax, digits
            end subroutine interface_plgzax
        end interface

        call interface_plgzax( digmax_out, digits_out )
        digmax = int(digmax_out)
        digits = int(digits_out)
    end subroutine plgzax_impl

    subroutine plinit_impl()
        interface
            subroutine interface_plinit() bind( c, name = 'c_plinit' )
            end subroutine interface_plinit
        end interface
        call interface_plinit()
    end subroutine plinit_impl

    subroutine pllab_impl( xlab, ylab, title )
        character(len=*), intent(in) :: xlab, ylab, title

        interface
            subroutine interface_pllab( xlab, ylab, title ) bind(c,name='c_pllab')
                implicit none
                character(len=1), dimension(*), intent(in) :: xlab, ylab, title
            end subroutine interface_pllab
        end interface

        call interface_pllab( trim(xlab)//c_null_char, trim(ylab)//c_null_char, trim(title)//c_null_char )

    end subroutine pllab_impl

    subroutine pllsty_impl( lin )
        integer, intent(in) :: lin
        interface
            subroutine interface_pllsty( lin ) bind( c, name = 'c_pllsty' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: lin
            end subroutine interface_pllsty
        end interface

        call interface_pllsty( int(lin,kind=private_plint) )
    end subroutine pllsty_impl

    subroutine plmkstrm_impl( strm )
        integer, intent(in) :: strm
        interface
            subroutine interface_plmkstrm( strm ) bind( c, name = 'c_plmkstrm' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: strm
            end subroutine interface_plmkstrm
        end interface

        call interface_plmkstrm( int(strm,kind=private_plint) )
    end subroutine plmkstrm_impl

    function plparseopts_dynamic(argv, mode)
        integer :: plparseopts_dynamic !function type
        character(len=:), intent(inout), dimension(:), allocatable :: argv
        integer, intent(in) :: mode

        integer(kind=private_plint) :: size_local
        integer :: max_length_local

        character(len=1), dimension(:,:), allocatable :: cstring_arg_local
        type(c_ptr), dimension(:), allocatable :: cstring_address_arg_inout

        call character_array_to_c( cstring_arg_local, cstring_address_arg_inout, argv )

        ! Note that the C routine interface_plparseopts changes this value.
        size_local = size(argv, kind=private_plint)
        plparseopts_dynamic = int(interface_plparseopts( size_local, &
             cstring_address_arg_inout, int(mode, kind=private_plint)))
        if(plparseopts_dynamic /= 0) return
        deallocate(argv)
        max_length_local = max_cstring_length(cstring_address_arg_inout(1:size_local))
        allocate(character(len=max_length_local) :: argv(0:size_local - 1))
        plparseopts_dynamic = c_to_character_array(argv, cstring_address_arg_inout(1:size_local))
    end function plparseopts_dynamic

    function plparseopts_static_length(argv, mode, disambiguate)
        integer :: plparseopts_static_length !function type
        character(len=*), intent(inout), dimension(:), allocatable :: argv
        integer, intent(in) :: mode, disambiguate

        integer(kind=private_plint) :: size_local

        character(len=1), dimension(:,:), allocatable :: cstring_arg_local
        type(c_ptr), dimension(:), allocatable :: cstring_address_arg_inout

        call character_array_to_c( cstring_arg_local, cstring_address_arg_inout, argv )

        ! Note that the C routine interface_plparseopts changes this value.
        size_local = size(argv, kind=private_plint)
        plparseopts_static_length = int(interface_plparseopts( size_local, &
             cstring_address_arg_inout, int(mode, kind=private_plint)))
        if(plparseopts_static_length /= 0) return

        if(len(argv) < max_cstring_length(cstring_address_arg_inout(1:size_local))) then
            write(error_unit,*) 'Plplot Fortran Severe Warning: plparseopts: at least one argument is too long to process'
            plparseopts_static_length = 1
            return
        endif
        deallocate(argv)
        allocate(argv(0:size_local - 1))
        plparseopts_static_length = c_to_character_array(argv, cstring_address_arg_inout(1:size_local))
    end function plparseopts_static_length

    function plparseopts_static(nargv, argv, mode)
        integer :: plparseopts_static !function type
        integer, intent(out) :: nargv
        character(len=*), intent(inout), dimension(0:) :: argv
        integer, intent(in) :: mode

        integer(kind=private_plint) :: size_local

        character(len=1), dimension(:,:), allocatable :: cstring_arg_local
        type(c_ptr), dimension(:), allocatable :: cstring_address_arg_inout

        call character_array_to_c( cstring_arg_local, cstring_address_arg_inout, argv )

        ! Note that the C routine interface_plparseopts changes this value.
        size_local = size(argv, kind=private_plint)
        plparseopts_static = int(interface_plparseopts( size_local, &
             cstring_address_arg_inout, int(mode, kind=private_plint)))
        if(plparseopts_static /= 0) return
        if(len(argv) < max_cstring_length(cstring_address_arg_inout(1:size_local))) then
            write(error_unit,*) 'Plplot Fortran Severe Warning: plparseopts: at least one argument is too long to process'
            plparseopts_static = 1
            return
        endif
        plparseopts_static = c_to_character_array(argv, cstring_address_arg_inout(1:size_local))
        nargv = int(size_local - 1)
    end function plparseopts_static

    function plparseopts_brief(mode)
        integer :: plparseopts_brief !function type
        integer, intent(in) :: mode

        integer :: numargs_local, plget_arguments_rc
        integer(kind=private_plint) :: size_local

        integer, parameter :: maxargs_local = 1000
        character (len=maxlen), dimension(0:maxargs_local) :: arg_local
        character(len=1), dimension(:,:), allocatable :: cstring_arg_local
        type(c_ptr), dimension(:), allocatable :: cstring_address_arg_inout

        plget_arguments_rc = plget_arguments(numargs_local, arg_local)
        if(plget_arguments_rc /= 0) then
           plparseopts_brief = 1
           return
        endif
        call character_array_to_c( cstring_arg_local, cstring_address_arg_inout, arg_local(0:numargs_local) )

        ! Note that the C routine interface_plparseopts changes this value.
        size_local = int(numargs_local + 1, kind=private_plint)
        plparseopts_brief = int(interface_plparseopts( size_local, &
               cstring_address_arg_inout, int(mode, kind=private_plint)))
    end function plparseopts_brief

    subroutine plpat_impl( inc, del )
        integer, dimension(:), intent(in) :: inc, del

        integer(kind=private_plint) :: nlin_local

        interface
            subroutine interface_plpat( nlin, inc, del ) bind( c, name = 'c_plpat' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: nlin
                integer(kind=private_plint), dimension(*), intent(in) :: inc, del
            end subroutine interface_plpat
        end interface

        nlin_local = size(inc, kind=private_plint)
        if(nlin_local /= size(del, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plpat: sizes of inc and del are not consistent"
        endif

        call interface_plpat( nlin_local, int(inc,kind=private_plint), int(del,kind=private_plint) )

    end subroutine plpat_impl

    subroutine plprec_impl( setp, prec )
        integer, intent(in) :: setp, prec
        interface
            subroutine interface_plprec( setp, prec ) bind( c, name = 'c_plprec' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: setp, prec
            end subroutine interface_plprec
        end interface

        call interface_plprec( int(setp,kind=private_plint), int(prec,kind=private_plint) )
    end subroutine plprec_impl

    subroutine plpsty_impl( patt )
        integer, intent(in) :: patt
        interface
            subroutine interface_plpsty( patt ) bind( c, name = 'c_plpsty' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: patt
            end subroutine interface_plpsty
        end interface

        call interface_plpsty( int(patt,kind=private_plint) )
    end subroutine plpsty_impl

    subroutine plreplot_impl()
        interface
            subroutine interface_plreplot() bind(c,name='c_plreplot')
            end subroutine interface_plreplot
        end interface
        call interface_plreplot()
    end subroutine plreplot_impl

    subroutine plscmap0_impl( r, g, b )
        integer, dimension(:), intent(in) :: r, g, b

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plscmap0( r, g, b, n ) bind(c,name='c_plscmap0')
                import :: private_plint
                implicit none
                integer(kind=private_plint), dimension(*), intent(in) :: r, g, b
                integer(kind=private_plint), value, intent(in) :: n
            end subroutine interface_plscmap0
        end interface

        n_local = size(r,kind=private_plint)
        if( n_local /= size(g, kind=private_plint) .or. n_local /= size(b, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plscmap0: inconsistent sizes for r, g, and/or b"
        end if

        call interface_plscmap0( int(r,kind=private_plint), int(g,kind=private_plint), int(b,kind=private_plint), &
               n_local )
    end subroutine plscmap0_impl

    subroutine plscmap0n_impl( n )
        integer, intent(in) :: n
        interface
            subroutine interface_plscmap0n( n ) bind( c, name = 'c_plscmap0n' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
            end subroutine interface_plscmap0n
        end interface

        call interface_plscmap0n( int(n,kind=private_plint) )
    end subroutine plscmap0n_impl

    subroutine plscmap1_impl( r, g, b )
        integer, dimension(:), intent(in) :: r, g, b

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plscmap1( r, g, b,  n ) bind(c,name='c_plscmap1')
                import :: private_plint
                implicit none
                integer(kind=private_plint), dimension(*), intent(in) :: r, g, b
                integer(kind=private_plint), value, intent(in) :: n
            end subroutine interface_plscmap1
        end interface

        n_local = size(r,kind=private_plint)
        if( n_local /= size(g, kind=private_plint) .or. n_local /= size(b, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plscmap1: inconsistent sizes for r, g, and/or b"
        end if

        call interface_plscmap1( int(r,kind=private_plint), int(g,kind=private_plint), int(b,kind=private_plint), &
               n_local )
    end subroutine plscmap1_impl

    subroutine plscmap1n_impl( n )
        integer, intent(in) :: n
        interface
            subroutine interface_plscmap1n( n ) bind( c, name = 'c_plscmap1n' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
            end subroutine interface_plscmap1n
        end interface

        call interface_plscmap1n( int(n,kind=private_plint) )
    end subroutine plscmap1n_impl

    subroutine plscol0_impl( icol, r, g, b )
        integer, intent(in) :: icol, r, g, b
        interface
            subroutine interface_plscol0( icol, r, g, b ) bind( c, name = 'c_plscol0' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: icol, r, g, b
            end subroutine interface_plscol0
        end interface

        call interface_plscol0( int(icol,kind=private_plint), &
               int(r,kind=private_plint), int(g,kind=private_plint), int(b,kind=private_plint) )
    end subroutine plscol0_impl

    subroutine plscolbg_impl( r, g, b )
        integer, intent(in) :: r, g, b
        interface
            subroutine interface_plscolbg( r, g, b ) bind( c, name = 'c_plscolbg' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: r, g, b
            end subroutine interface_plscolbg
        end interface

        call interface_plscolbg( int(r,kind=private_plint), int(g,kind=private_plint), int(b,kind=private_plint) )
    end subroutine plscolbg_impl


    subroutine plscolor_impl( color )
        integer, intent(in) :: color
        interface
            subroutine interface_plscolor( color ) bind( c, name = 'c_plscolor' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: color
            end subroutine interface_plscolor
        end interface

        call interface_plscolor( int(color,kind=private_plint) )
    end subroutine plscolor_impl

    subroutine plscompression_impl( compression )
        integer, intent(in) :: compression
        interface
            subroutine interface_plscompression( compression ) bind( c, name = 'c_plscompression' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: compression
            end subroutine interface_plscompression
        end interface

        call interface_plscompression( int(compression,kind=private_plint) )
    end subroutine plscompression_impl

    subroutine plsdev_impl( devname )
        character(len=*), intent(in) :: devname

        interface
            subroutine interface_plsdev( devname ) bind(c,name='c_plsdev')
                implicit none
                character(len=1), dimension(*), intent(in) :: devname
            end subroutine interface_plsdev
        end interface

        call interface_plsdev( trim(devname)//c_null_char )

    end subroutine plsdev_impl

    subroutine plsdrawmode_impl( mode )
        integer, intent(in) :: mode
        interface
            subroutine interface_plsdrawmode( mode ) bind( c, name = 'c_plsdrawmode' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: mode
            end subroutine interface_plsdrawmode
        end interface

        call interface_plsdrawmode( int(mode,kind=private_plint) )
    end subroutine plsdrawmode_impl

    subroutine plseed_impl( s )
        integer, intent(in) :: s
        interface
            subroutine interface_plseed( s ) bind( c, name = 'c_plseed' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: s
            end subroutine interface_plseed
        end interface

        call interface_plseed( int(s,kind=private_plint) )
    end subroutine plseed_impl

    ! TODO: character-version
    subroutine plsesc_impl( esc )
        integer, intent(in) :: esc
        interface
            subroutine interface_plsesc( esc ) bind( c, name = 'c_plsesc' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: esc
            end subroutine interface_plsesc
        end interface

        call interface_plsesc( int(esc,kind=private_plint) )
    end subroutine plsesc_impl

    function plsetopt_impl( opt, optarg )
        integer :: plsetopt_impl !function type
        character(len=*), intent(in) :: opt, optarg


        interface
            function interface_plsetopt( opt, optarg ) bind(c,name='c_plsetopt')
                import :: private_plint
                implicit none
                integer(kind=private_plint) :: interface_plsetopt !function type
                character(len=1), dimension(*), intent(in) :: opt, optarg
            end function interface_plsetopt
        end interface

        plsetopt_impl = int(interface_plsetopt( trim(opt)//c_null_char, trim(optarg)//c_null_char ))

    end function plsetopt_impl

    subroutine plsfam_impl( fam, num, bmax )
        integer, intent(in) :: fam, num, bmax
        interface
            subroutine interface_plsfam( fam, num, bmax ) bind( c, name = 'c_plsfam' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: fam, num, bmax
            end subroutine interface_plsfam
        end interface

        call interface_plsfam( int(fam,kind=private_plint), int(num,kind=private_plint), int(bmax,kind=private_plint) )
    end subroutine plsfam_impl

    subroutine plsfci_impl( fci )
        integer, intent(in) :: fci

        interface
            subroutine interface_plsfci( fci ) bind( c, name = 'c_plsfci' )
                import :: private_plunicode
                implicit none
                integer(kind=private_plunicode), value, intent(in) :: fci
            end subroutine interface_plsfci
        end interface

        call interface_plsfci( int(fci,kind=private_plunicode) )

    end subroutine plsfci_impl

    subroutine plsfnam_impl( fnam )
        character(len=*), intent(in) :: fnam

        interface
            subroutine interface_plsfnam( fnam ) bind(c,name='c_plsfnam')
                implicit none
                character(len=1), dimension(*), intent(in) :: fnam
            end subroutine interface_plsfnam
        end interface

        call interface_plsfnam( trim(fnam)//c_null_char )

    end subroutine plsfnam_impl

    subroutine plsfont_impl( family, style, weight )
        integer, intent(in) :: family, style, weight
        interface
            subroutine interface_plsfont( family, style, weight ) bind( c, name = 'c_plsfont' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: family, style, weight
            end subroutine interface_plsfont
        end interface

        call interface_plsfont( int(family,kind=private_plint), int(style,kind=private_plint), int(weight,kind=private_plint) )
    end subroutine plsfont_impl

    ! Probably would be better to define this in redacted form, but it is not documented that
    ! way, and the python interface also does not use redacted form.  So leave it for now.
    ! I (AWI) followed advice in <http://stackoverflow.com/questions/10755896/fortran-how-to-store-value-255-into-one-byte>
    ! for the type statement for plotmem
    subroutine plsmem_impl( maxx, maxy, plotmem )
        integer, intent(in) :: maxx, maxy
        character(kind=c_char), dimension(:, :, :), target, intent(in) :: plotmem
        interface
            subroutine interface_plsmem( maxx, maxy, plotmem ) bind( c, name = 'c_plsmem' )
                import :: c_ptr
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: maxx, maxy
                type(c_ptr), value, intent(in) :: plotmem
            end subroutine interface_plsmem
        end interface

        ! We need a first dimension of 3 to have space for RGB
        if( 3 /= size(plotmem,1) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsmem: first dimension of plotmem is not 3"
        endif

        ! Since not defined in redacted form, we at least check that
        ! maxx, and maxy are consistent with the second and third dimensions of plotmem.
        if( maxx /= size(plotmem,2) .or. maxy /= size(plotmem,3) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsmem: maxx and/or maxy not &
                   &consistent with second and third plotmem dimensions"
        endif
        call interface_plsmem( int(maxx,kind=private_plint), int(maxy,kind=private_plint),  c_loc(plotmem))
    end subroutine plsmem_impl

    ! Probably would be better to define this in redacted form, but it is not documented that
    ! way, and the python interface also does not use redacted form.  So leave it for now.
    ! I (AWI) followed advice in <http://stackoverflow.com/questions/10755896/fortran-how-to-store-value-255-into-one-byte>
    ! for the type statement for plotmem
    subroutine plsmema_impl( maxx, maxy, plotmem )
        integer, intent(in) :: maxx, maxy
        character(kind=c_char), dimension(:, :, :), target, intent(in) :: plotmem
        interface
            subroutine interface_plsmema( maxx, maxy, plotmem ) bind( c, name = 'c_plsmema' )
                import :: c_ptr
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: maxx, maxy
                type(c_ptr), value, intent(in) :: plotmem
            end subroutine interface_plsmema
        end interface

        ! We need a first dimension of 4 to have space for RGBa
        if( 4 /= size(plotmem,1) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsmema: first dimension of plotmem is not 4"
        endif

        ! Since not defined in redacted form, we at least check that
        ! maxx, and maxy are consistent with the second and third dimensions of plotmem.
        if( maxx /= size(plotmem,2) .or. maxy /= size(plotmem,3) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plsmema: maxx and/or maxy not &
                   &consistent with second and third plotmem dimensions"
        endif
        call interface_plsmema( int(maxx,kind=private_plint), int(maxy,kind=private_plint),  c_loc(plotmem))
    end subroutine plsmema_impl

    subroutine plsori_impl( rot )
        integer, intent(in) :: rot
        interface
            subroutine interface_plsori( rot ) bind( c, name = 'c_plsori' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: rot
            end subroutine interface_plsori
        end interface

        call interface_plsori( int(rot,kind=private_plint) )
    end subroutine plsori_impl

    subroutine plspal0_impl( filename )
        character(len=*), intent(in) :: filename

        interface
            subroutine interface_plspal0( filename ) bind(c,name='c_plspal0')
                implicit none
                character(len=1), dimension(*), intent(in) :: filename
            end subroutine interface_plspal0
        end interface

        call interface_plspal0( trim(filename)//c_null_char )

    end subroutine plspal0_impl

    subroutine plspal1_impl( filename, interpolate )
        character(len=*), intent(in) :: filename
        logical, intent(in) :: interpolate

        interface
            subroutine interface_plspal1( filename, interpolate ) bind(c,name='c_plspal1')
                import :: private_plbool
                implicit none
                integer(kind=private_plbool), value, intent(in) :: interpolate
                character(len=1), dimension(*), intent(in) :: filename
            end subroutine interface_plspal1
        end interface

        call interface_plspal1( trim(filename)//c_null_char, int( merge(1,0,interpolate),kind=private_plbool) )

    end subroutine plspal1_impl

    subroutine plspause_impl( pause )
        logical, intent(in) :: pause

        interface
            subroutine interface_plspause( pause ) bind(c,name='c_plspause')
                import :: private_plbool
                implicit none
                integer(kind=private_plbool), value, intent(in) :: pause
            end subroutine interface_plspause
        end interface

        call interface_plspause( int( merge(1,0,pause),kind=private_plbool) )
    end subroutine plspause_impl

    subroutine plsstrm_impl( strm )
        integer, intent(in) :: strm
        interface
            subroutine interface_plsstrm( strm ) bind( c, name = 'c_plsstrm' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: strm
            end subroutine interface_plsstrm
        end interface

        call interface_plsstrm( int(strm,kind=private_plint) )
    end subroutine plsstrm_impl

    subroutine plssub_impl( nx, ny )
        integer, intent(in) :: nx, ny
        interface
            subroutine interface_plssub( nx, ny ) bind( c, name = 'c_plssub' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny
            end subroutine interface_plssub
        end interface

        call interface_plssub( int(nx,kind=private_plint), int(ny,kind=private_plint) )
    end subroutine plssub_impl

    subroutine plstar_impl( nx, ny )
        integer, intent(in) :: nx, ny
        interface
            subroutine interface_plstar( nx, ny ) bind( c, name = 'c_plstar' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny
            end subroutine interface_plstar
        end interface

        call interface_plstar( int(nx,kind=private_plint), int(ny,kind=private_plint) )
    end subroutine plstar_impl

    subroutine plstart_impl( devname, nx, ny )
        integer, intent(in) :: nx, ny
        character(len=*), intent(in) :: devname
        interface
            subroutine interface_plstart( devname, nx, ny ) bind( c, name = 'c_plstart' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: nx, ny
                character(len=1), dimension(*), intent(in) :: devname
            end subroutine interface_plstart
        end interface

        call interface_plstart( trim(devname)//c_null_char, int(nx,kind=private_plint), int(ny,kind=private_plint) )
    end subroutine plstart_impl

    subroutine plstripd_impl( id )
        integer, intent(in) :: id
        interface
            subroutine interface_plstripd( id ) bind( c, name = 'c_plstripd' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: id
            end subroutine interface_plstripd
        end interface

        call interface_plstripd( int(id,kind=private_plint) )
    end subroutine plstripd_impl

    subroutine plstyl_impl( mark, space )
        integer, dimension(:), intent(in) :: mark, space

        integer(kind=private_plint) :: n_local

        interface
            subroutine interface_plstyl( n, mark, space ) bind( c, name = 'c_plstyl' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: n
                integer(kind=private_plint), dimension(*), intent(in) :: mark, space
            end subroutine interface_plstyl
        end interface

        n_local = size(mark,kind=private_plint)
        if( n_local /= size(space, kind=private_plint) ) then
            write(error_unit, "(a)") "Plplot Fortran Warning: plstyl: inconsistent sizes for mark and space"
        end if

        call interface_plstyl( n_local, int(mark,kind=private_plint), int(space,kind=private_plint) )
    end subroutine plstyl_impl

    ! double- and single-precision variants defined in the plplot_double and plplot_single modules.
    subroutine plsvect_none( fill )
        logical, optional, intent(in) :: fill

        interface
            subroutine interface_plsvect_null() bind(c,name='plsvect_null')
            end subroutine interface_plsvect_null
        end interface

        ! Note: simple construct to avoid compiler message about unused argument
        if ( present(fill) ) then
            call interface_plsvect_null
        else
            call interface_plsvect_null
        endif
    end subroutine plsvect_none

    subroutine plsxax_impl( digmax, digits )
        integer, intent(in) :: digmax, digits
        interface
            subroutine interface_plsxax( digmax, digits ) bind( c, name = 'c_plsxax' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: digmax, digits
            end subroutine interface_plsxax
        end interface

        call interface_plsxax( int(digmax,kind=private_plint), int(digits,kind=private_plint) )
    end subroutine plsxax_impl

    subroutine plsyax_impl( digmax, digits )
        integer, intent(in) :: digmax, digits
        interface
            subroutine interface_plsyax( digmax, digits ) bind( c, name = 'c_plsyax' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: digmax, digits
            end subroutine interface_plsyax
        end interface

        call interface_plsyax( int(digmax,kind=private_plint), int(digits,kind=private_plint) )
    end subroutine plsyax_impl

    subroutine plszax_impl( digmax, digits )
        integer, intent(in) :: digmax, digits
        interface
            subroutine interface_plszax( digmax, digits ) bind( c, name = 'c_plszax' )
                import :: private_plint
                implicit none
                integer(kind=private_plint), value, intent(in) :: digmax, digits
            end subroutine interface_plszax
        end interface

        call interface_plszax( int(digmax,kind=private_plint), int(digits,kind=private_plint) )
    end subroutine plszax_impl

    subroutine pltext_impl()
        interface
            subroutine interface_pltext() bind(c,name='c_pltext')
            end subroutine interface_pltext
        end interface
        call interface_pltext()
    end subroutine pltext_impl

    subroutine pltimefmt_impl( fmt )
        character(len=*), intent(in) :: fmt

        interface
            subroutine interface_pltimefmt( fmt ) bind(c,name='c_pltimefmt')
                implicit none
                character(len=1), dimension(*), intent(in) :: fmt
            end subroutine interface_pltimefmt
        end interface

        call interface_pltimefmt( trim(fmt)//c_null_char )

    end subroutine pltimefmt_impl

    subroutine plvsta_impl()
        interface
            subroutine interface_plvsta() bind(c,name='c_plvsta')
            end subroutine interface_plvsta
        end interface
        call interface_plvsta()
    end subroutine plvsta_impl

    subroutine plxormod_impl( mode, status )
        logical, intent(in) :: mode
        logical, intent(out) :: status

        integer(kind=private_plbool) :: status_out

        interface
            subroutine interface_plxormod( mode, status ) bind(c,name='c_plxormod')
                import :: private_plbool
                implicit none
                integer(kind=private_plbool), value, intent(in) :: mode
                integer(kind=private_plbool), intent(out) :: status
            end subroutine interface_plxormod
        end interface

        call interface_plxormod( int( merge(1,0,mode),kind=private_plbool), status_out )
        status = status_out /= 0_private_plbool

    end subroutine plxormod_impl

end module plplot
