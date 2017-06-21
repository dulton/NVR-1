@ECHO OFF
@echo ===================================================================
@echo    清除svn,vss,cvs标记文件   author:Yusheng date:2009.4.24
@echo    filename : clearSvnCvsVss.bat
@echo ===================================================================
@ECHO 按Ctrl + C取消。
@pause
@echo 执行批处理：%0 %1 %2 %3
@echo 转到：%1 
cd /d %1

@echo (1)开始清除“.SVN”文件夹
@rem for /r %%d in (.) do if exist "%%d\.svn" echo /s /q "%%d\.svn"
@for /r %%d in (.) do if exist "%%d\.svn" rd /s /q "%%d\.svn"

