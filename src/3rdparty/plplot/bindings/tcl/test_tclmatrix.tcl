puts "Do not edit this generated file which gives the stdout results of"
puts "pltcl running the command \"source bindings/tcl/test_tclmatrix.tcl\""
puts "to test the matrix capabilities of libtclmatrix.  Instead, check its"
puts "consistency with the latest pltcl, libtclmatrix, and"
puts "bindings/tcl/test_tclmatrix.tcl by building the (Unix) target"
puts "\"check_libtclmatrix_capabilities\" which creates the latest version"
puts "of this file in bindings/tcl/test_tclmatrix.out_compare in the build"
puts "tree and compares that version with bindings/tcl/test_tclmatrix.out"
puts "in the source tree.  If that target fails because the two versions"
puts "of that file are different, diff the build-tree version with the"
puts "source tree version to help evaluate the differences, and if those"
puts "differences seem reasonable (say due to a known change in"
puts "bindings/tcl/test_tclmatrix.tcl in the source tree), then copy the"
puts "build-tree version of this file back to the source tree, verify the"
puts "result by running the \"check_libtclmatrix_capabilities\" target,"
puts "and then commit the new version of the source-tree version."
puts ""
puts ""

set execute 1
if {$execute == 1} {
puts "Create one-dimensional x matrix using \"matrix x f 4 = {0., 1., 2., 3.}\""
matrix x f 4 = {0., 1., 2., 3.}
puts "Various start:stop:step slice examples for this matrix"
puts "Examples where start, stop, and step are default"
puts "\[x :\] yields [x :]"
puts "\"*\" (implemented for backwards compatibility) has exactly the same effect as \":\""
puts "\[x *\] yields [x *]"
puts "\"::\" has exactly the same effect as \":\""
puts "\[x ::\] yields [x ::]"
puts "Examples where start and stop are default"
puts "\[x ::1\] yields [x ::1]"
puts "\[x ::2\] yields [x ::2]"
puts "\[x ::3\] yields [x ::3]"
puts "\[x ::4\] yields [x ::4]"
puts "\[x ::-1\] yields [x ::-1]"
puts "\[x ::-2\] yields [x ::-2]"
puts "\[x ::-3\] yields [x ::-3]"
puts "\[x ::-4\] yields [x ::-4]"
puts "Examples where start and step are default"
puts "\[x :2:\] yields [x :2:]"
puts "\[x :2\] yields [x :2]"
puts "Examples where stop and step are default"
puts "\[x 2::\] yields [x 2::]"
puts "\[x 2:\] yields [x 2:]"
puts "Examples where start is default"
puts "\[x :3:2\] yields [x :3:2]"
puts "\[x :-4:-2\] yields [x :-4:-2]"
puts "Examples where stop is default"
puts "\[x 1::2\] yields [x 1::2]"
puts "\[x -2::-2\] yields [x -2::-2]"
puts "Examples where step is default"
puts "\[x 1:3:\] yields [x 1:3:]"
puts "\[x 1:3\] yields [x 1:3]"
puts "Examples where start, stop, and step are all explicitly specified"
puts "\[x 1:0:2\] yields [x 1:0:2]"
puts "\[x 1:1:2\] yields [x 1:1:2]"
puts "\[x 1:2:2\] yields [x 1:2:2]"
puts "\[x 1:3:2\] yields [x 1:3:2]"
puts "\[x 1:4:2\] yields [x 1:4:2]"
puts "\[x 1:5:2\] yields [x 1:5:2]"
puts "\[x -2:-1:-2\] yields [x -2:-1:-2]"
puts "\[x -2:-2:-2\] yields [x -2:-2:-2]"
puts "\[x -2:-3:-2\] yields [x -2:-3:-2]"
puts "\[x -2:-4:-2\] yields [x -2:-4:-2]"
puts "\[x -2:-5:-2\] yields [x -2:-5:-2]"
puts "\[x -2:-6:-2\] yields [x -2:-6:-2]"
puts "Single index (i.e., not a slice) examples"
puts "\[x 0\] yields [x 0]"
puts "\[x 1\] yields [x 1]"
puts "\[x 2\] yields [x 2]"
puts "\[x 3\] yields [x 3]"
puts "\[x -1] yields [x -1]"
puts "\[x -2] yields [x -2]"
puts "\[x -3] yields [x -3]"
puts "\[x -4] yields [x -4]"
puts "Results from foreach loop over all elements are:"
foreach v [x :] { puts $v }
puts "Results from for loop with output only for \[x \$i\] < 3 are:"
for {set i 0} {$i < 4} {incr i} {
    if {[x $i] < 3} {puts [x $i]} }
puts "Change 3rd element using \"x 2 = 7\""
x 2 = 7
puts "\[x :\] yields [x :]"
puts "Change all elements using \"x : = 3\""
x : = 3
puts "\[x :\] = [x :]"
x delete
puts "Various matrix initializations and assignments"
puts "Using a collection of space-separated numbers"
matrix x f 4 = 1 2 3 4
puts "\[x :\] = [x :]"
matrix y f 2 4
y : : = 1 2 3 4 5 6 7 8
puts "\[y : :\] = [y : :]"
x delete
y delete
puts "Using a list of lists of numbers"
matrix x f 4 = {{{1 2}} {3 4}}
puts "\[x :\] = [x :]"
matrix y f 2 4
y : : = {{1 2 3 4 5} {{6}} {7 8}}
puts "\[y : :\] = [y : :]"
puts "Using slices of a previously defined matrix"
matrix z f 2 2 2 = [x ::] [x ::-1]
puts "\[z : : :\] = [z : : :]"
y : : = [x ::] [x ::-1]
puts "\[y : :\] = [y : :]"
puts "Combination of previously defined matrices, deep lists, and space-separated numbers"
matrix a f 2 2 3 = [x ::] [x ::-1] {{{1.E-13} {2}}} 3 4 5 6 7 8 9 10 11 12 13 14
puts "\[a : : :\] = [a : : :]"
matrix b f 2 2 3
b : : : = [x ::] [x ::-1] {{{1.E-13} {2}}} 3 4 5 6 7 8 9 10 11 12 13 14
puts "\[b : : :\] = [b : : :]"
x delete
y delete
z delete
a delete
b delete
}
