! test_plfortrandemolib.f90 --
!     Test program for the auxiliary functions
!
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
program test_plfortrandemolib
    use plfortrandemolib

    real(kind=pl_test_flt), dimension(10)   :: value
    integer               :: i
    integer, dimension(4) :: start = (/  0,  0,  1,  2 /), &
           stop  = (/ 10, 10, 11, 22 /), &
           step  = (/  1,  2,  1,  2 /), &
           expected_size = &
           (/ 10,  5, 10, 10 /)
    integer               :: sz

    ! Function arange:
    ! - Check the length of the returned array
    ! - Check the values
    !
    do i = 1,size(start)
        sz = size( arange( start(i), stop(i), step(i) ) )
        if ( sz /= expected_size(i) ) then
            write(*,*) 'Expected:', expected_size(i), ' - got: ', sz
        else
            value(1:sz) = arange( start(i), stop(i), step(i) )
            write(*,*) value(1:sz)
            if ( any( value(1:sz) < start(i) ) ) then
                write(*,*) 'Minimum value too low: ', minval(value(1:sz)), ' - expected: ', start(i)
            endif
            if ( any( value(1:sz) >= stop(i) ) ) then
                write(*,*) 'Maximum value too high: ', maxval(value(1:sz)), ' - should be below: ', stop(i)
            endif
            if ( any( value(2:sz) - value(1:sz-1) /= step(i) ) ) then
                write(*,*) 'Difference in subsequent values incorrect: ', value(1:sz), ' - expected difference:', step(i)
            endif
        endif
    enddo
end program test_plfortrandemolib
