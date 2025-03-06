@echo off
for /r "%~1" %%i in (*.cpp *.h *.hpp *.c *.cc *.cxx) do (
    echo Formatting: %%i
    clang-format -i "%%i"
)
echo Done!
