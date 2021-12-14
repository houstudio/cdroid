!      Copyright (C) 2004-2016  Alan W. Irwin
!
!      This file is part of PLplot.
!
!      PLplot is free software; you can redistribute it and/or modify
!      it under the terms of the GNU Library General Public License as
!      published by the Free Software Foundation; either version 2 of the
!      License, or (at your option) any later version.
!
!      PLplot is distributed in the hope that it will be useful,
!      but WITHOUT ANY WARRANTY; without even the implied warranty of
!      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!      GNU Library General Public License for more details.
!
!      You should have received a copy of the GNU Library General Public
!      License along with PLplot; if not, write to the Free Software
!      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

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
!--------------------------------------------------------------------------
! main
!
! Shows two views of the world map.
!--------------------------------------------------------------------------

program x19f
    use plplot, double_PI => PL_PI
    use iso_c_binding, only: c_ptr, c_loc, c_f_pointer
    implicit none
    real(kind=pl_test_flt), parameter :: PI = double_PI

    integer, parameter :: double = kind(1.0d0)

    real(kind=pl_test_flt)    minx, maxx, miny, maxy
    real(kind=pl_test_flt), dimension(1:1) :: x, y

    !     Variables for the shapelib example
    integer, parameter                 :: nbeachareas    = 2
    integer, dimension(nbeachareas)    :: beachareas     = (/ 23, 24 /)
    integer, parameter                 :: nwoodlandareas = 94
    integer, dimension(nwoodlandareas) :: woodlandareas
    integer, parameter                 :: nshingleareas  = 22
    integer, dimension(nshingleareas)  :: shingleareas = (/ 0, 1, 24, 25, 26, 27, 28, 29, 30, 31, 32, &
           33, 34, 35, 217, 2424, 2425, 2426, 2427,  &
           2428, 2491, 2577 /)
    integer, parameter                 :: ncragareas     = 2024
    integer, dimension(ncragareas)     :: cragareas
    integer, dimension(9)              :: majorroads     = (/ 33, 48, 71, 83, 89, 90, 101, 102, 111 /)
    integer                            :: i
    integer :: plparseopts_rc
    type label_data_type
        character(len=2), dimension(-1:1,2) :: label_data
    end type label_data_type

    ! Whether to use no_label_data type of callback.
    logical, parameter :: ifno_label_data = .false.

    type(label_data_type), target :: label_data

    !      Process command-line arguments
    plparseopts_rc = plparseopts(PL_PARSE_FULL)
    if(plparseopts_rc .ne. 0) stop "plparseopts error"

    ! Longitude (x) and latitude (y)

    miny = -70._pl_test_flt
    maxy = 80._pl_test_flt

    call plinit()

    ! Cartesian plots
    ! Most of world

    minx = -170._pl_test_flt
    maxx = minx+360._pl_test_flt

    ! Setup a custom latitude and longitude-based scaling function.
    if(ifno_label_data) then
        call plslabelfunc(geolocation_labeler)
    else
        label_data%label_data(-1,1) = " W"
        label_data%label_data(0,1) = ""
        label_data%label_data(1,1) = " E"
        label_data%label_data(-1,2) = " S"
        label_data%label_data(0,2) = "Eq"
        label_data%label_data(1,2) = " N"
        call plslabelfunc(geolocation_labeler_data, c_loc(label_data))
    endif

    call plcol0(1)
    call plenv(minx, maxx, miny, maxy, 1, 70)

    call plmap('usaglobe', minx, maxx, miny, maxy)

    ! The Americas

    minx = 190._pl_test_flt
    maxx = 340._pl_test_flt

    call plcol0(1)
    call plenv(minx, maxx, miny, maxy, 1, 70)
    call plmap('usaglobe', minx, maxx, miny, maxy)

    ! Clear the labeling function
    call plslabelfunc

    ! Polar, Northern hemisphere

    minx = 0._pl_test_flt
    maxx = 360._pl_test_flt

    call plenv(-75._pl_test_flt, 75._pl_test_flt, -75._pl_test_flt, &
           75._pl_test_flt, 1, -1)
    call plmap(mapform19, 'globe', minx, maxx, miny, maxy)

    call pllsty(2)
    call plmeridians(mapform19,10.0_pl_test_flt, 10.0_pl_test_flt, &
           0.0_pl_test_flt, 360.0_pl_test_flt, -10.0_pl_test_flt, &
           80.0_pl_test_flt)

    ! Polar, Northern hemisphere, this time with a PLplot-wide transform

    minx = 0._pl_test_flt
    maxx = 360._pl_test_flt

    call plstransform( map_transform )

    call pllsty( 1 )
    call plenv( -75._pl_test_flt, 75._pl_test_flt, -75._pl_test_flt, &
           75._pl_test_flt, 1, -1 )
    ! No need to set the map transform here as the global
    ! transform will be used.
    call plmap('globe', minx, maxx, miny, maxy )

    call pllsty( 2 )
    call plmeridians(10.0_pl_test_flt, 10.0_pl_test_flt, &
           0.0_pl_test_flt, 360.0_pl_test_flt, -10.0_pl_test_flt, &
           80.0_pl_test_flt )

    ! Show Baltimore, MD on the map
    call plcol0( 2 )
    call plssym( 0.0_pl_test_flt, 2.0_pl_test_flt )
    x=-76.6125_pl_test_flt
    y=39.2902778_pl_test_flt
    call plpoin( x, y, 18 )
    call plssym( 0.0_pl_test_flt, 1.0_pl_test_flt )
    call plptex( -76.6125_pl_test_flt, 43.0_pl_test_flt, 0.0_pl_test_flt, &
           0.0_pl_test_flt, 0.0_pl_test_flt, 'Baltimore, MD' )

    ! For Fortran, this is how the global transform is cleared.
    call plstransform

    ! An example using shapefiles. The shapefiles used are from Ordnance Survey, UK.
    ! These were chosen because they provide shapefiles for small grid boxes which
    ! are easilly manageable for this demo.

    call pllsty( 1 )

    minx = 240570._pl_test_flt
    maxx = 621109._pl_test_flt
    miny = 87822._pl_test_flt
    maxy = 722770._pl_test_flt
    call plscol0( 0, 255, 255, 255 )
    call plscol0( 1, 0, 0, 0 )
    call plscol0( 2, 150, 150, 150 )
    call plscol0( 3, 0, 50, 200 )
    call plscol0( 4, 50, 50, 50 )
    call plscol0( 5, 150, 0, 0 )
    call plscol0( 6, 100, 100, 255 )

    minx = 265000._pl_test_flt
    maxx = 270000._pl_test_flt
    miny = 145000._pl_test_flt
    maxy = 150000._pl_test_flt
    call plscol0( 0, 255, 255, 255 )  !white
    call plscol0( 1, 0, 0, 0 )        !black
    call plscol0( 2, 255, 200, 0 )    !yelow for sand
    call plscol0( 3, 60, 230, 60 )    !green for woodland
    call plscol0( 4, 210, 120, 60 )   !brown for contours
    call plscol0( 5, 150, 0, 0 )      !red for major roads
    call plscol0( 6, 180, 180, 255 )  !pale blue for water
    call plscol0( 7, 100, 100, 100 )  !pale grey for shingle or boulders
    call plscol0( 8, 100, 100, 100 )  !dark grey for custom polygons - generally crags


    call plcol0( 1 )
    call plenv( minx, maxx, miny, maxy, 1, -1 )
    call pllab( "", "", "Martinhoe CP, Exmoor National Park, UK (shapelib only)" )


    ! Beach
    call plcol0( 2 )
    call plmapfill( "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, beachareas )

    ! Woodland
    call plcol0( 3 )
    do i = 1, nwoodlandareas
        woodlandareas(i) = i + 217
    enddo
    call plmapfill( "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, woodlandareas )

    ! Shingle or boulders
    call plcol0( 7 )
    call plmapfill( "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, shingleareas )

    ! Crags
    call plcol0( 8 )
    do i = 1, ncragareas
        cragareas(i) = i + 324
    enddo
    call plmapfill( "ss/ss64ne_Landform_Area", minx, maxx, miny, maxy, cragareas )

    ! draw contours, we need to separate contours from high/low coastline
    ! draw_contours(pls, "ss/SS64_line", 433, 20, 4, 3, minx, maxx, miny, maxy );
    call plcol0( 4 )
    call plmapline( "ss/ss64ne_Height_Contours", minx, maxx, miny, maxy )

    ! draw the sea and surface water
    call plwidth( 0.0_pl_test_flt )
    call plcol0( 6 )
    call plmapfill( "ss/ss64ne_Water_Area", minx, maxx, miny, maxy )
    call plwidth( 2.0_pl_test_flt )
    call plmapline( "ss/ss64ne_Water_Line", minx, maxx, miny, maxy )

    ! draw the roads, first with black and then thinner with colour to give an
    ! an outlined appearance
    call plwidth( 5.0_pl_test_flt )
    call plcol0( 1 )
    call plmapline( "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy )
    call plwidth( 3.0_pl_test_flt )
    call plcol0( 0 )
    call plmapline( "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy )
    call plcol0( 5 )
    call plmapline( "ss/ss64ne_Road_Centreline", minx, maxx, miny, maxy, majorroads )

    ! draw buildings
    call plwidth( 1.0_pl_test_flt )
    call plcol0( 1 )
    call plmapfill( "ss/ss64ne_Building_Area", minx, maxx, miny, maxy )

    ! labels
    call plsfci( int(z'00000100') )
    call plschr( 0._pl_test_flt, 0.8_pl_test_flt )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "MARTINHOE CP", minx, maxx, miny, maxy, 202 )
    call plschr( 0._pl_test_flt, 0.7_pl_test_flt );
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "Heale"//achar(10)//"Down", minx, maxx, miny, maxy, 13 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "South"//achar(10)//"Down", minx, maxx, miny, maxy, 34 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "Martinhoe"//achar(10)//"Common", minx, maxx, miny, maxy, 42 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "Woody Bay", minx, maxx, miny, maxy, 211 )
    call plschr( 0._pl_test_flt, 0.6_pl_test_flt );
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "Mill Wood", minx, maxx, miny, maxy, 16 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "Heale Wood", minx, maxx, miny, maxy, 17 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 1.0_pl_test_flt, &
           "Bodley", minx, maxx, miny, maxy, 31 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.0_pl_test_flt, &
           "Martinhoe", minx, maxx, miny, maxy, 37 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "Woolhanger"//achar(10)//"Common", minx, maxx, miny, maxy, 60 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "West Ilkerton"//achar(10)//"Common", minx, maxx, miny, maxy, 61 )
    call plmaptex( "ss/ss64ne_General_Text", 1.0_pl_test_flt, 0.0_pl_test_flt, 0.5_pl_test_flt, &
           "Caffyns"//achar(10)//"Heanton"//achar(10)//"Down", minx, maxx, miny, maxy, 62 )

    call plend()

contains

    subroutine map_transform(x, y, xt, yt)

        ! These callback arguments must have exactly these attributes
        ! because of disambiguation issues.
        real(kind=double), intent(in) :: x, y
        real(kind=double), intent(out) :: xt, yt

        real(kind=pl_test_flt) :: radius

        radius = 90.0_pl_test_flt - y
        xt = radius * cos(x * PI / 180.0_pl_test_flt)
        yt = radius * sin(x * PI / 180.0_pl_test_flt)
    end subroutine map_transform
    !--------------------------------------------------------------------------
    ! mapform19
    !
    ! Defines specific coordinate transformation for example 19.
    ! Not to be confused with mapform in src/plmap.c.
    ! x(), y() are the coordinates to be plotted.
    !--------------------------------------------------------------------------

    subroutine mapform19(x, y)

        real(kind=pl_test_flt), dimension(:), intent(inout) :: x, y

        real(kind=pl_test_flt), dimension(:), allocatable :: radius

        ! evaluate x last so RHS's do not get overwritten too soon
        allocate( radius(size(x)) )

        radius = 90.0_pl_test_flt - y
        y = radius*sin(x*PI/180.0_pl_test_flt)
        x = radius*cos(x*PI/180.0_pl_test_flt)
    end subroutine mapform19

    ! A custom axis labeling callback function for longitudes and latitudes.
    subroutine geolocation_labeler(axis, value, label)
        integer, intent(in) :: axis
        ! These callback arguments must have exactly these attributes
        ! because of disambiguation issues.
        real(kind=double), intent(in) :: value
        character(len=*), intent(out) :: label

        type(label_data_type), target :: data

        data%label_data(-1,1) = " W"
        data%label_data(0,1) = ""
        data%label_data(1,1) = " E"
        data%label_data(-1,2) = " S"
        data%label_data(0,2) = "Eq"
        data%label_data(1,2) = " N"
        call custom_labeler(axis, real(value,kind=pl_test_flt), label, c_loc(data))
    end subroutine geolocation_labeler

    ! Another custom axis labeling callback function for longitudes and latitudes.
    subroutine geolocation_labeler_data(axis, value, label, data)
        integer, intent(in) :: axis
        ! These callback arguments must have exactly these attributes
        ! because of disambiguation issues.
        real(kind=double), intent(in) :: value
        character(len=*), intent(out) :: label

        type(c_ptr), intent(in) :: data

        call custom_labeler(axis, real(value,kind=pl_test_flt), label, data)
    end subroutine geolocation_labeler_data

    ! This routine called by two different callbacks.
    subroutine custom_labeler(axis, value, label, data)
        integer, intent(in) :: axis
        real(kind=pl_test_flt), intent(in) :: value
        character(len=*), intent(out) :: label
        type(c_ptr), intent(in) :: data

        type(label_data_type), pointer :: d
        character(len=5) direction_label
        real(kind=pl_test_flt) :: label_val

        call c_f_pointer(data, d)
        label_val = 0.0_pl_test_flt

        if (axis .eq. 2) then
            label_val = value
            if (label_val .gt. 0.0_pl_test_flt) then
                direction_label = d%label_data(1,axis)
            else if (label_val .lt. 0.0_pl_test_flt) then
                direction_label = d%label_data(-1,axis)
            else
                direction_label = d%label_data(0,axis)
            endif
        else if (axis .eq. 1) then
            label_val = normalize_longitude(value)
            if (label_val .gt. 0.0_pl_test_flt) then
                direction_label = d%label_data(1,axis)
            else if (label_val .lt. 0.0_pl_test_flt) then
                direction_label = d%label_data(-1,axis)
            else
                direction_label = d%label_data(0,axis)
            endif
        endif
        if (axis .eq. 2 .and. value .eq. 0.0_pl_test_flt) then
            !     A special case for the equator
            label = direction_label
        else if (abs(label_val) .lt. 10.0_pl_test_flt) then
            write(label,'(I1.1,A2)') iabs(int(label_val)),direction_label
        else if (abs(label_val) .lt. 100.0_pl_test_flt) then
            write(label,'(I2.1,A2)') iabs(int(label_val)),direction_label
        else
            write(label,'(I3.1,A2)') iabs(int(label_val)),direction_label
        endif
    end subroutine custom_labeler

    !     "Normalize" longitude values so that they always fall between
    !      -180.0 and 180.0
    function normalize_longitude(lon)
        real(kind=pl_test_flt) :: normalize_longitude
        real(kind=pl_test_flt) :: lon, times

        if ((lon .ge. -180.0_pl_test_flt) .and. (lon .le. 180.0_pl_test_flt)) then
            normalize_longitude = lon
        else
            times = floor ((abs(lon) + 180.0_pl_test_flt) / 360.0_pl_test_flt)
            if (lon .lt. 0.0_pl_test_flt) then
                normalize_longitude = lon + 360.0_pl_test_flt * times
            else
                normalize_longitude = lon - 360.0_pl_test_flt * times
            endif
        endif
    end function normalize_longitude
end program x19f
