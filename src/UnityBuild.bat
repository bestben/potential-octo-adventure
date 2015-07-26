@Echo off

for /R %%f in (*.cpp) do @echo #include "%%f"
for /R %%f in (*.c) do @echo #include "%%f"