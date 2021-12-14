!   Sample plots using date / time formatting for axes
!
!   Copyright (C) 2008  Andrew Ross
!   Copyright (C) 2008-2016 Alan W. Irwin
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

program x29f
    use plplot
    implicit none

    integer, parameter :: double = kind(1.0d0)
    integer :: plparseopts_rc

    real(kind=double), parameter :: PI = PL_PI

    real(kind=pl_test_flt), dimension(365) :: x, y, xerr1, xerr2, yerr1, yerr2

    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    call plinit()

    !  This is the ASCII value for character @
    call plsesc(64)

    call plot1()
    call plot2()
    call plot3()
    call plot4()

    call plend()

contains


    !====================================================================
    !
    !     Plot a model diurnal cycle of temperature

    subroutine plot1()


        integer :: i, npts
        real(kind=pl_test_flt) :: xmin, xmax, ymin, ymax

        parameter(npts = 73)
        parameter(xmin = 0.0_pl_test_flt)
        parameter(xmax = 60.0_pl_test_flt*60.0_pl_test_flt*24.0_pl_test_flt)
        parameter(ymin = 10.0_pl_test_flt)
        parameter(ymax = 20.0_pl_test_flt)

        do i = 1,npts
            x(i) = xmax*(real(i-1,kind=pl_test_flt)/real(npts,kind=pl_test_flt))
            y(i) = 15.0_pl_test_flt - 5.0_pl_test_flt* &
                   cos(2.0_pl_test_flt*PI*real(i-1,kind=pl_test_flt)/ &
                   real(npts,kind=pl_test_flt))
            !     Set x error bars to +/- 5 minute
            xerr1(i) = x(i)-60.0_pl_test_flt*5.0_pl_test_flt
            xerr2(i) = x(i)+60.0_pl_test_flt*5.0_pl_test_flt
            !     Set y error bars to +/- 0.1 deg C
            yerr1(i) = y(i)-0.1_pl_test_flt
            yerr2(i) = y(i)+0.1_pl_test_flt
        enddo

        call pladv(0)

        !     Rescale major ticks marks by 0.5
        call plsmaj(0.0_pl_test_flt,0.5_pl_test_flt)
        !     Rescale minor ticks and error bar marks by 0.5
        call plsmin(0.0_pl_test_flt,0.5_pl_test_flt)

        call plvsta()
        call plwind(xmin, xmax, ymin, ymax)

        !     Draw a box with ticks spaced every 3 hour in X and 1 degree C in Y.
        call plcol0(1)
        !     Set time format to be hours:minutes
        call pltimefmt("%H:%M")
        call plbox("bcnstd", 3.0_pl_test_flt*60.0_pl_test_flt*60.0_pl_test_flt, 3, "bcnstv", &
               1.0_pl_test_flt, 5)

        call plcol0(3)
        call pllab("Time (hours:mins)", "Temperature (degC)", &
               "@frPLplot Example 29 - Daily temperature")

        call plcol0(4)

        call plline(x(1:npts), y(1:npts))
        call plcol0(2)
        call plerrx(xerr1(1:npts), xerr2(1:npts), y(1:npts))
        call plcol0(3)
        call plerry(x(1:npts), yerr1(1:npts), yerr2(1:npts))

        !     Rescale major / minor tick marks back to default
        call plsmin(0.0_pl_test_flt,1.0_pl_test_flt)
        call plsmaj(0.0_pl_test_flt,1.0_pl_test_flt)

    end subroutine plot1

    !
    !     Plot the number of hours of daylight as a function of day for a year
    !
    subroutine plot2()

        integer ::  j, npts
        real(kind=pl_test_flt) :: xmin, xmax, ymin, ymax
        real(kind=pl_test_flt) :: lat, p, d


        ! Latitude for London
        parameter (lat = 51.5_pl_test_flt)

        parameter (npts = 365)

        parameter(xmin = 0.0_pl_test_flt)
        parameter(xmax = npts*60.0_pl_test_flt*60.0_pl_test_flt*24.0_pl_test_flt)
        parameter(ymin = 0)
        parameter(ymax = 24)

        !     Formula for hours of daylight from
        !     "A Model Comparison for Daylength as a Function of Latitude and
        !     Day of the Year", 1995, Ecological Modelling, 80, pp 87-95.
        do j=1,npts
            x(j) = (j-1)*60.0_pl_test_flt*60.0_pl_test_flt*24.0_pl_test_flt
            p = asin(0.39795_pl_test_flt*cos(0.2163108_pl_test_flt + 2.0_pl_test_flt* &
                   atan(0.9671396_pl_test_flt*tan(0.00860_pl_test_flt*(j-187)))))
            d = 24.0_pl_test_flt - (24.0_pl_test_flt/PI)* &
                   acos( (sin(0.8333_pl_test_flt*PI/180.0_pl_test_flt) + &
                   sin(lat*PI/180.0_pl_test_flt)*sin(p)) / (cos(lat*PI/180.0_pl_test_flt)* &
                   cos(p)) )
            y(j) = d
        enddo

        call plcol0(1)
        !     Set time format to be abbreviated month name followed by day of month
        call pltimefmt("%b %d")
        call plprec(1,1)
        call plenv(xmin, xmax, ymin, ymax, 0, 40)

        call plcol0(3)
        call pllab("Date", "Hours of daylight", &
               "@frPLplot Example 29 - Hours of daylight at 51.5N")

        call plcol0(4)

        call plline(x, y)

        call plprec(0,0)

    end subroutine plot2

    !
    !
    !
    subroutine plot3()

        integer :: i, npts
        real(kind=pl_test_flt) :: xmin, xmax, ymin, ymax
        integer :: tstart

        parameter (npts = 62)

        !     number of seconds elapsed since the Unix epoch (1970-01-01, UTC) for
        !     2005-12-01, UTC.  This is the same result as the Python
        !     calendar.timegm((2005,12,1,0,0,0)) result or the Linux C timegm
        !     result corresponding to 2005-12-01.
        tstart = 1133395200

        xmin = real(tstart,kind=pl_test_flt)
        xmax = xmin + npts*60.0_pl_test_flt*60.0_pl_test_flt*24.0_pl_test_flt
        ymin = 0.0_pl_test_flt
        ymax = 5.0_pl_test_flt

        do i=1,npts
            x(i) = xmin + real(i-1,kind=pl_test_flt)*60.0_pl_test_flt*60.0_pl_test_flt*24.0_pl_test_flt
            y(i) = 1.0_pl_test_flt + &
                   sin( 2.0_pl_test_flt*PI*real(i-1,kind=pl_test_flt)/7.0_pl_test_flt) + &
                   exp( real(min(i-1,npts+1-i),kind=pl_test_flt) / 31.0_pl_test_flt)
        enddo
        call pladv(0)

        call plvsta()
        call plwind(xmin, xmax, ymin, ymax)

        call plcol0(1)
        !     Set time format to be ISO 8601 standard YYYY-MM-DD. Note that this is
        !     equivalent to %f for C99 compliant implementations of strftime.
        call pltimefmt("%Y-%m-%d")
        !     Draw a box with ticks spaced every 14 days in X and 1 hour in Y.
        call plbox("bcnstd", 14.0_pl_test_flt*24.0_pl_test_flt*60.0_pl_test_flt*60.0_pl_test_flt,14, &
               "bcnstv", 1.0_pl_test_flt, 4)

        call plcol0(3)
        call pllab("Date", "Hours of television watched", &
               "@frPLplot Example 29 - Hours of television watched in " // &
               "Dec 2005 / Jan 2006")

        call plcol0(4)

        call plssym(0.0_pl_test_flt, 0.5_pl_test_flt)
        call plpoin(x(1:npts), y(1:npts), 2)
        call plline(x(1:npts), y(1:npts))

    end subroutine plot3

    !
    !
    !
    subroutine plot4()
        !     TAI-UTC (seconds) as a function of time.

        real(kind=pl_test_flt) :: scale
        real(kind=pl_test_flt) :: xmin, xmax, ymin, ymax, xlabel_step
        integer :: k, npts = 0, i
        logical :: if_TAI_time_format = .false.
        character(len=10) :: time_format
        character(len=100) :: title_suffix
        character(len=100) :: xtitle
        character(len=100) :: title
        real(kind=pl_test_flt) :: x(1001), y(1001)
        integer :: epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min
        real(kind=pl_test_flt) :: epoch_sec
        integer :: tai_year, tai_month, tai_day, tai_hour, tai_min
        real(kind=pl_test_flt) :: tai_sec, tai
        integer :: utc_year, utc_month, utc_day, utc_hour, utc_min
        real(kind=pl_test_flt) :: utc_sec, utc

        ! Continuous time unit is Besselian years from whatever epoch is
        ! chosen below.  Could change to seconds (or days) from the
        ! epoch, but then would have to adjust xlabel_step below.
        scale = 365.242198781_pl_test_flt
        ! MJD epoch (see <https://en.wikipedia.org/wiki/Julian_day>).
        ! This is only set for illustrative purposes, and is overwritten
        ! below for the time-representation reasons given in the
        ! discussion below.
        epoch_year  = 1858
        epoch_month = 11
        epoch_day   = 17
        epoch_hour  = 0
        epoch_min   = 0
        epoch_sec   = 0._pl_test_flt
        ! To illustrate the time-representation issues of using the
        ! MJD epoch, in 1985, MJD was roughly 46000 days which
        ! corresponds to 4e9 seconds.  Thus, for the case where
        ! pl_test_flt corresponds to double precision which can
        ! represent continuous time to roughly 16 decimal digits of
        ! precision, the time-representation error is roughly ~400
        ! nanoseconds.  Therefore the MJD epoch would be acceptable
        ! for the plots below when pl_test_flt corresponds to double
        ! precision.  However, that epoch is obviously not acceptable
        ! for the case where pl_test_flt corresponds to single
        ! precision which can represent continuous time to only ~7
        ! decimal digits of precision corresponding to a time
        ! representation error of 400 seconds (!)  in 1985.  For this
        ! reason, we do not use the MJD epoch below and instead choose
        ! the best epoch for each case to minimize time-representation
        ! issues.

        do k = 0,6
            if (k .eq. 0) then
                ! Choose midpoint to maximize time-representation precision.
                epoch_year  = 1985
                epoch_month = 0
                epoch_day   = 2
                epoch_hour  = 0
                epoch_min   = 0
                epoch_sec   = 0._pl_test_flt
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 0, .true., &
                       epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec )
                call plctime(1950,0,2,0,0,0._pl_test_flt,xmin)
                call plctime(2020,0,2,0,0,0._pl_test_flt,xmax)
                npts = 70*12 + 1
                ymin = 0.0_pl_test_flt
                ymax = 36.0_pl_test_flt
                time_format="%Y%"
                if_TAI_time_format = .true.
                title_suffix = "from 1950 to 2020"
                xtitle =  "Year"
                xlabel_step = 10.0_pl_test_flt
            elseif ((k .eq. 1) .or. (k .eq. 2)) then
                ! Choose midpoint to maximize time-representation precision.
                epoch_year  = 1961
                epoch_month = 7
                epoch_day   = 1
                epoch_hour  = 0
                epoch_min   = 0
                epoch_sec   = 1.64757_pl_test_flt
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 0, .true., &
                       epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec )
                call plctime(1961,7,1,0,0,1.64757_pl_test_flt-.20_pl_test_flt, xmin)
                call plctime(1961,7,1,0,0,1.64757_pl_test_flt+.20_pl_test_flt, xmax)
                npts = 1001
                ymin = 1.625_pl_test_flt
                ymax = 1.725_pl_test_flt
                time_format = "%S%2%"
                title_suffix = "near 1961-08-01 (TAI)"
                xlabel_step = 0.05_pl_test_flt/(scale*86400.0_pl_test_flt)
                if (k .eq. 1) then
                    if_TAI_time_format = .true.
                    xtitle = "Seconds (TAI)"
                else
                    if_TAI_time_format = .false.
                    xtitle = "Seconds (TAI) labelled with corresponding UTC"
                endif
            elseif ((k .eq. 3) .or. (k .eq. 4)) then
                ! Choose midpoint to maximize time-representation precision.
                epoch_year  = 1963
                epoch_month = 10
                epoch_day   = 1
                epoch_hour  = 0
                epoch_min   = 0
                epoch_sec   = 2.6972788_pl_test_flt
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 0, .true., &
                       epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec )
                call plctime(1963,10,1,0,0,2.6972788_pl_test_flt-.20_pl_test_flt, xmin)
                call plctime(1963,10,1,0,0,2.6972788_pl_test_flt+.20_pl_test_flt, xmax)
                npts = 1001
                ymin = 2.55_pl_test_flt
                ymax = 2.75_pl_test_flt
                time_format = "%S%2%"
                title_suffix = "near 1963-11-01 (TAI)"
                xlabel_step = 0.05_pl_test_flt/(scale*86400.0_pl_test_flt)
                if (k .eq. 3) then
                    if_TAI_time_format = .true.
                    xtitle = "Seconds (TAI)"
                else
                    if_TAI_time_format = .false.
                    xtitle = "Seconds (TAI) labelled with corresponding UTC"
                endif
            elseif ((k .eq. 5) .or. (k .eq. 6)) then
                ! Choose midpoint to maximize time-representation precision.
                epoch_year  = 2009
                epoch_month = 0
                epoch_day   = 1
                epoch_hour  = 0
                epoch_min   = 0
                epoch_sec   = 34._pl_test_flt
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 0, .true., &
                       epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec )
                call plctime(2009,0,1,0,0,34._pl_test_flt-5._pl_test_flt,xmin)
                call plctime(2009,0,1,0,0,34._pl_test_flt+5._pl_test_flt,xmax)
                npts = 1001
                ymin = 32.5_pl_test_flt
                ymax = 34.5_pl_test_flt
                time_format = "%S%2%"
                title_suffix = "near 2009-01-01 (TAI)"
                xlabel_step = 1._pl_test_flt/(scale*86400._pl_test_flt)
                if (k .eq. 5) then
                    if_TAI_time_format = .true.
                    xtitle = "Seconds (TAI)"
                else
                    if_TAI_time_format = .false.
                    xtitle = "Seconds (TAI) labelled with corresponding UTC"
                endif
            endif

            do i=0,npts-1
                x(i+1) = xmin + i*(xmax-xmin)/(real(npts-1,kind=pl_test_flt))
                tai = x(i+1)
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 0, .true., &
                       epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec )
                call plbtime(tai_year, tai_month, tai_day, tai_hour, &
                       tai_min, tai_sec, tai)
                ! Calculate residual using tai as the epoch to nearly maximize time-representation precision.
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 0, .true., &
                       tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec)
                ! Calculate continuous tai with new epoch.
                call plctime( tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec, tai)
                ! Calculate broken-down utc (with leap seconds inserted) from continuous tai with new epoch.
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 2, .true., &
                       tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec)
                call plbtime(utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec, tai)
                ! Calculate continuous utc from broken-down utc using same epoch as for the continuous tai.
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 0, .true., &
                       tai_year, tai_month, tai_day, tai_hour, tai_min, tai_sec)
                call plctime(utc_year, utc_month, utc_day, utc_hour, utc_min, utc_sec, utc)
                ! Convert residuals to seconds.
                y(i+1)=(tai-utc)*scale*86400._pl_test_flt
            enddo

            call pladv(0)
            call plvsta()
            call plwind(xmin, xmax, ymin, ymax)
            call plcol0(1)
            if (if_TAI_time_format) then
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 0, .true., &
                       epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec )
            else
                call plconfigtime(scale, 0._pl_test_flt, 0._pl_test_flt, 2, .true., &
                       epoch_year, epoch_month, epoch_day, epoch_hour, epoch_min, epoch_sec )
            endif
            call pltimefmt(time_format)
            call plbox("bcnstd", xlabel_step, 0, "bcnstv", 0._pl_test_flt, 0)
            call plcol0(3)
            title = "@frPLplot Example 29 - TAI-UTC "// &
                   trim(title_suffix)
            call pllab(xtitle, "TAI-UTC (sec)", title)

            call plcol0(4)

            call plline(x(1:npts), y(1:npts))
        enddo
    end subroutine plot4
end program x29f
