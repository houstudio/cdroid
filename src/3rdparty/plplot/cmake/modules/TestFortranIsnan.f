      program testisnan

c      external isnan

      if (isnan(0.0/0.0)) then
        print *,"NaN"
      endif

      end program testisnan
