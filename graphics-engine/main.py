import sys

from script import run

if len(sys.argv) == 2:
    run(sys.argv[1])
elif len(sys.argv) == 1:
    run(input("Please enter the filename of an mdl script file: \n"))
else:
    print("Too many arguments.")
