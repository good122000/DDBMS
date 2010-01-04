SET /A i = 0
:LOOP
SET /A i = i+1
COPY .\DDBMS.exe ddbms%i%\DDBMS.exe
COPY .\libmysql.dll ddbms%i%\libmysql.dll
COPY .\mysqlpp.dll ddbms%i%\mysqlpp.dll
CD ddbms%i%
START DDBMS
CD ..
IF %i%==3 GOTO END
GOTO LOOP
:END