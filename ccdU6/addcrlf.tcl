

set all [glob ApnCamData_*.h]
foreach i $all { 
   set fap [open $i a]
   puts $fap " "
   close $fap 
   puts stdout "done $i"
}

