@echo off
echo ============================================================
echo   ULUSAN SIGORTA - Mail Sunucusu GUI Build
echo ============================================================

set PATH=C:\msys64\mingw64\bin;%PATH%
set GCC=g++
set FLAGS=-std=c++17 -O2 -DUNICODE -D_UNICODE
set INC=-Iimgui -Iimgui/backends -Isrc
set OUT=ulusan_mail_gui.exe

if not exist obj mkdir obj

echo Derleniyor...

%GCC% %FLAGS% %INC% -c src/main_gui.cpp -o obj/main_gui.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% %INC% -c src/gui_app.cpp -o obj/gui_app.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% %INC% -c src/gui_screens_nav.cpp -o obj/gui_nav.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% %INC% -c src/gui_screens_content.cpp -o obj/gui_content.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Isrc -c src/auth.cpp -o obj/auth.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Isrc -c src/mail_store.cpp -o obj/mail_store.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Isrc -c src/smtp_server.cpp -o obj/smtp.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Isrc -c src/imap_server.cpp -o obj/imap.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Iimgui -c imgui/imgui.cpp -o obj/imgui.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Iimgui -c imgui/imgui_draw.cpp -o obj/imgui_draw.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Iimgui -c imgui/imgui_tables.cpp -o obj/imgui_tables.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Iimgui -c imgui/imgui_widgets.cpp -o obj/imgui_widgets.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -DUNICODE -D_UNICODE -Iimgui -Iimgui/backends -c imgui/backends/imgui_impl_win32.cpp -o obj/imgui_win32.o
if %ERRORLEVEL% NEQ 0 goto fail
%GCC% %FLAGS% -Iimgui -Iimgui/backends -c imgui/backends/imgui_impl_opengl3.cpp -o obj/imgui_gl3.o
if %ERRORLEVEL% NEQ 0 goto fail

echo Linkleniyor...
%GCC% obj/*.o -o %OUT% -lws2_32 -lopengl32 -lgdi32 -ldwmapi -limm32 -lole32 -mwindows -static-libgcc -static-libstdc++
if %ERRORLEVEL% NEQ 0 goto fail

echo.
echo [OK] Basarili! %OUT% olusturuldu.
echo Calistirmak icin: %OUT%
pause
exit /b 0

:fail
echo.
echo [HATA] Derleme basarisiz!
pause
exit /b 1
