module hello_module
contains
    subroutine hello()
        write(*,"(a)", advance="no") "hello, world"
    end subroutine hello
end module hello_module
