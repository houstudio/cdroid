!   Demonstration program for PLplot illustrating absolute positioning
!   of graphs on a page
!
!   Copyright (C) 2004-2016 Alan W. Irwin
!
!   This file is part of PLplot.
!
!   PLplot is free software; you can redistribute it and/or modify
!   it under the terms of the GNU Library General Public License as
!   published by the Free Software Foundation; either version 2 of the
!   License, or (at your option) any later version.
!
!   PLplot is distributed in the hope that it will be useful,
!   but WITHOUT ANY WARRANTY; without even the implied warranty of
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!   GNU Library General Public License for more details.
!
!   You should have received a copy of the GNU Library General Public
!   License along with PLplot; if not, write to the Free Software
!   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

!     N.B. the pl_test_flt parameter used in this code is only
!     provided by the plplot module to allow convenient developer
!     testing of either kind(1.0) or kind(1.0d0) floating-point
!     precision regardless of the floating-point precision of the
!     PLplot C libraries.  We do not guarantee the value of this test
!     parameter so it should not be used by users, and instead user
!     code should replace the pl_test_flt parameter by whatever
!     kind(1.0) or kind(1.0d0) precision is most convenient for them.
!     For further details on floating-point precision issues please
!     consult README_precision in this directory.
!
program x10f
    use plplot
    implicit none
    integer :: plparseopts_rc

    !   Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    call plinit()

    call pladv(0)
    call plvpor(0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plwind(0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plbox('bc', 0.0_pl_test_flt, 0, 'bc', 0.0_pl_test_flt, 0 )

    call plsvpa(50.0_pl_test_flt, 150.0_pl_test_flt, 50.0_pl_test_flt, 100.0_pl_test_flt )
    call plwind(0.0_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plbox('bc', 0.0_pl_test_flt, 0, 'bc', 0.0_pl_test_flt, 0 )
    call plptex(0.5_pl_test_flt, 0.5_pl_test_flt, 1.0_pl_test_flt, 0.0_pl_test_flt, &
           0.5_pl_test_flt, 'BOX at (50,150,50,100)' )

    call plend

end program x10f
